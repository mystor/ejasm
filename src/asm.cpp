#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>

struct PState {
  std::istream &is;
  std::stringstream os;
  int64_t lineno;
  int64_t col;
  int64_t offset;

  PState(std::istream &is) : is(is), lineno(1), col(0), offset(0) {}

  int peek() { return is.peek(); }
  int get() {
    char c = is.get();
    col++;
    if (c == '\n') { col = 0; lineno++; }

    return c;
  }

  void reportError(const char *err) {
    std::cerr << "ERROR (line " << lineno << ", col " << col << "): " << err << "\n";
    exit(-1);
  }

  void writeWord(int64_t word) {
    os.write((char *)&word, sizeof(word));
    offset += sizeof(word);
  }

  void writeChar(int8_t chr) {
    os.put(chr);
    offset++;
  }

  void alignToWord() {
    size_t size = sizeof(int64_t) - (offset % sizeof(int64_t));
    if (size == sizeof(int64_t)) return;
    for (size_t i = 0; i < size; i++) {
      os.put(0); offset++;
    }
  }

  int64_t getOffset() {
    return offset;
  }

  int64_t getWordAt(int64_t off) {
    os.seekg(off);
    int64_t word;
    os.read((char *)&word, sizeof(word));
    os.seekg(offset);

    return word;
  }

  void setWordAt(int64_t off, int64_t word) {
    os.seekp(off);
    os.write((char *)&word, sizeof(word));
    os.seekp(offset);
  }

  std::string data() {
    return os.str();
  }

  void flush() {
    os.flush();
  }
};

void skipWhitespace(PState &ps) {
  char c;
  for(;;) {
    c = ps.peek();

    switch (c) {
    case ' ':
    case '\n':
    case '\t':
    case '\r':
      ps.get();
      break;
    default:
      return;
    }
  }
}

std::string getIdentifier(PState &ps) {
  std::string s;
  char c;
  for (;;) {
    // TODO(michael): Don't allow null identifiers
    c = ps.peek();
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z')) {
      ps.get();
      s.push_back(c);
    } else {
      return s;
    }
  }
}

int64_t getInt(PState &ps) {
  int64_t i = 0;

  // Read a possible negative sign
  bool neg = false;
  if (ps.peek() == '-') {
    ps.get();
    neg = true;
  }

  // TODO(michael): Don't allow null numbers.
  // TODO(michael): allow hex and octal numbers
  // Read the number
  char c;
  for (;;) {
    c = ps.peek();
    if (c >= '0' && c <= '9') {
      ps.get();
      i *= 10;
      i += c - '0';
    } else {
      if (neg) {
        return -i;
      } else {
        return i;
      }
    }
  }
}

int main(int argc, char **argv) {
  bool ifileRead = false;
  std::string ifileName = "-";
  bool ofileRead = false;
  std::string ofileName = "-";
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-o") == 0) {
      // Reading an -o option!
      i++;
      if (i >= argc) {
        // We're out of options!
        std::cerr << "A filename is expected after the -o option" << std::endl;
        exit(-1);
      } else {
        if (ofileRead) {
          std::cerr << "You can only specify one output file" << std::endl;
          exit(-1);
        } else {
          ofileRead = true;
          ofileName = argv[i];
        }
      }
    } else {
      // Reading an input option
      if (ifileRead) {
        std::cerr << "You can only specify one input file" << std::endl;
        exit(-1);
      } else {
        ifileRead = true;
        ifileName = argv[i];
      }
    }
  }

  // std::cout << "ifileName = " << ifileName << std::endl;
  // std::cout << "ofileName = " << ofileName << std::endl;

  // Create the file streams - I `new` them here, because they will be
  // cleaned up when the program exits, and they need to live for the
  // program's full length anyways, and this makes my life easier.
  std::istream *is;
  std::ostream *os;
  if (ifileName == "-") {
    is = &std::cin;
  } else {
    std::ifstream *ifs = new std::ifstream;
    ifs->open(ifileName);
    is = ifs;
  }

  if (ofileName == "-") {
    os = &std::cout;
  } else {
    std::ofstream *ofs = new std::ofstream;
    ofs->open(ofileName);
    os = ofs;
  }


  std::unordered_map< std::string, int64_t > symbolValues;
  std::unordered_map< std::string, std::vector< int64_t > > targets;

  // Push is a special operation, as it is the only operation which contains
  // a payload. It contains the value which it is pushing in its upper 63 bits, and
  // has it's lowest bit unset. All other operations MUST have their lowest bit set
  // to be valid.

  symbolValues["load"] = 0x1;
  symbolValues["store"] = 0x3;
  symbolValues["exit"] = 0x5;
  symbolValues["loadbyte"] = 0x7;
  symbolValues["storebyte"] = 0x9;

  // Basic Math!
  symbolValues["add"] = 0x11;
  symbolValues["sub"] = 0x13;
  symbolValues["mul"] = 0x15;
  symbolValues["div"] = 0x17;
  symbolValues["mod"] = 0x19;
  symbolValues["negate"] = 0x1B;

  // Stack operations
  symbolValues["dup"] = 0x21;
  symbolValues["swap"] = 0x23;
  symbolValues["pop"] = 0x25;

  // Bitwise operations
  symbolValues["band"] = 0x31;
  symbolValues["bor"] = 0x33;
  symbolValues["bnot"] = 0x35;
  symbolValues["xor"] = 0x37;

  // bit-shifts
  symbolValues["shr"] = 0x39; // Logical shift right
  symbolValues["shl"] = 0x3B; // Logical shift left
  symbolValues["sar"] = 0x3D; // Arethmetic shift right
  symbolValues["sal"] = 0x3F; // Arethmetic shift left

  // Boolean operations
  symbolValues["and"] = 0x41;
  symbolValues["or"] = 0x43;
  symbolValues["not"] = 0x45;

  // Comparison operations
  symbolValues["eq"] = 0x47;
  symbolValues["ne"] = 0x49;
  symbolValues["gt"] = 0x4B;
  symbolValues["lt"] = 0x4D;
  symbolValues["gte"] = 0x4F;
  symbolValues["lte"] = 0x51;

  // Control flow operations
  symbolValues["jmp"] = 0x61;
  symbolValues["jmpz"] = 0x63;

  // Built in operations
  symbolValues["malloc"] = 0xA1;
  symbolValues["free"] = 0xA3;
  symbolValues["write"] = 0xA5;
  symbolValues["read"] = 0xA7;


  PState ps(*is);

  // Read until we get to the end of the file
  int64_t depth = 0;
  for (;;) {
    skipWhitespace(ps);
    int c = ps.peek();

    // When we hit EOF, we're done
    if (c == EOF) {
      break;
    } else if ((c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z')) {
      // Labels, commands, and identifiers
      std::string id = getIdentifier(ps);
      skipWhitespace(ps);

      // This could be either a label, a reference, or a push
      c = ps.peek();
      if (c == ':') {
        // A label was read in, consume the ':', and then record the current output
        // address for later usage in the substitution phase
        ps.get();
        if (depth != 0) {
          ps.reportError("You can not place labels in push instructions");
        }

        if (symbolValues.count(id) > 0) {
          ps.reportError("Cannot define the same label twice!");
        }

        // Labels should always be word-aligned
        ps.alignToWord();
        symbolValues[id] = ps.getOffset();
      } else if (id == "push") { // An push instruction!
        // A push instruction was read in, increase the depth, and go around the loop again.
        if (c == '(') {
          ps.get();
          depth++;
          continue;
        } else {
          ps.reportError("The `push` instruction takes an argument (like push(5))");
        }
      } else {
        // Just an identifier - fill it in later
        ps.alignToWord();
        int64_t offset = ps.getOffset();
        targets[id].push_back(offset);

        // And fill the space in memory
        ps.writeWord(depth); // Depth is stored for later
      }
    } else if (c >= '0' && c <= '9') {
      // Integer literal - echo the bytes out to the output
      int64_t i = getInt(ps);
      i <<= depth;
      ps.writeWord(i);
    } else {
      ps.reportError("Unrecognised token in input stream");
    }

    // Check if we need to close out our push statement
    if (depth > 0) {
      skipWhitespace(ps);

      if ((c = ps.get()) == ')') {
        depth--;
      } else {
        ps.reportError("Expected closing bracket for push expression");
      }
    }
  }

  if (depth > 0) {
    ps.reportError("Unexpected end of file while parsing push expression");
  }

  // We've reached the end, now it's time to fill in the unset values
  for (auto &it : targets) {
    if (symbolValues.count(it.first) == 0) {
      std::cerr << "ERROR: Undeclared symbol: " << it.first << std::endl;
      exit(-1);
    }

    int64_t value = symbolValues[it.first];

    for (int64_t addr : it.second) {
      int64_t thisValue = value;
      int64_t depth = ps.getWordAt(addr);

      // Wrap in levels of push calls
      thisValue <<= depth;

      ps.setWordAt(addr, thisValue);
    }
  }

  ps.flush();
  // Now we can dump it all out
  *os << ps.data();
  os->flush();
}
