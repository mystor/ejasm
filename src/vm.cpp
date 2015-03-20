#include <iostream>
#include <fstream>
#include <stack>

enum Command {
  CMD_LOAD = 0x1,
  CMD_STORE = 0x3,
  CMD_EXIT = 0x5,
  CMD_LOADBYTE = 0x7,
  CMD_STOREBYTE = 0x9,

  // Basic Math!
  CMD_ADD = 0x11,
  CMD_SUB = 0x13,
  CMD_MUL = 0x15,
  CMD_DIV = 0x17,
  CMD_MOD = 0x19,
  CMD_NEGATE = 0x1B,

  // Stack operations
  CMD_DUP = 0x21,
  CMD_SWAP = 0x23,
  CMD_POP = 0x25,

  // Bitwise operations
  CMD_BAND = 0x31,
  CMD_BOR = 0x33,
  CMD_BNOT = 0x35,
  CMD_XOR = 0x37,

  // bit-shifts
  CMD_SHR = 0x39, // Logical shift right
  CMD_SHL = 0x3B, // Logical shift left
  CMD_SAR = 0x3D, // Arethmetic shift right
  CMD_SAL = 0x3F, // Arethmetic shift left

  // Boolean operations
  CMD_AND = 0x41,
  CMD_OR = 0x43,
  CMD_NOT = 0x45,

  // Comparison operations
  CMD_EQ = 0x47,
  CMD_NE = 0x49,
  CMD_GT = 0x4B,
  CMD_LT = 0x4D,
  CMD_GTE = 0x4F,
  CMD_LTE = 0x51,

  // Control flow operations
  CMD_JMP = 0x61,
  CMD_JMPZ = 0x63,

  // Built in operations
  CMD_MALLOC = 0xA1,
  CMD_FREE = 0xA3,
  CMD_WRITE = 0xA5,
  CMD_READ = 0xA7,
};

std::stack<uint64_t> es;

char *getPtr(char *buf, int64_t size, int64_t ptr) {
  if (ptr >= 0 && ptr < size) {
    return (buf + ptr);
  } else {
    // TODO(michael): Make this safer (maybe)
    return (char *)(ptr - size);
  }
}

int64_t loadWord(char *buf, int64_t size, int64_t ptr) {
  return *(int64_t *)getPtr(buf, size, ptr);
}

void storeWord(char *buf, int64_t size, int64_t ptr, int64_t value) {
  *(int64_t *)getPtr(buf, size, ptr) = value;
}

void run(char *buf, int64_t size) {
  int64_t ip = 0;

  for (;;) {
    // Load the instruction
    int64_t inst = loadWord(buf, size, ip);
    ip+=sizeof(int64_t);

#define BIN_OPS(a, b) int64_t a = es.top(); es.pop(); int64_t b = es.top(); es.pop()
    if (inst & 0x1) {
      // Non-push
      switch (inst) {
      case CMD_LOAD: {
        int64_t ptr = es.top();
        es.pop();
        es.push(loadWord(buf, size, ptr));
      } break;
      case CMD_STORE: {
        int64_t v = es.top();
        es.pop();
        storeWord(buf, size, es.top(), v);
        es.pop();
      } break;
      case CMD_LOADBYTE: {
        int64_t ptr = es.top();
        es.pop();
        es.push(*getPtr(buf, size, ptr));
      } break;
      case CMD_STOREBYTE: {
        int8_t v = es.top(); // Cast to a char
        es.pop();
        *getPtr(buf, size, es.top()) = v; // Store it
        es.pop();

      } break;
      case CMD_EXIT: {
        exit(es.top()); // es.pop() is unnecessary
      } break;

        // Basic Math!
      case CMD_ADD: {
        BIN_OPS(v1, v2);
        es.push(v1 + v2);
      } break;
      case CMD_SUB: {
        BIN_OPS(v1, v2);
        es.push(v1 - v2);
      } break;
      case CMD_MUL: {
        BIN_OPS(v1, v2);
        es.push(v1 * v2);
      } break;
      case CMD_DIV: {
        BIN_OPS(v1, v2);
        es.push(v1 / v2);
      } break;
      case CMD_MOD: {
        BIN_OPS(v1, v2);
        es.push(v1 % v2);
      } break;
      case CMD_NEGATE: {
        int64_t v = es.top();
        es.pop();
        es.push(-v);
      } break;

        // Stack operations
      case CMD_DUP: {
        es.push(es.top());
      } break;
      case CMD_SWAP: {
        BIN_OPS(v1, v2);
        es.push(v2);
        es.push(v1);
      } break;
      case CMD_POP: {
        es.pop();
      } break;

        // Bitwise operations
      case CMD_BAND: {
        BIN_OPS(v1, v2);
        es.push(v1 & v2);
      } break;
      case CMD_BOR: {
        BIN_OPS(v1, v2);
        es.push(v1 | v2);
      } break;
      case CMD_BNOT: {
        int64_t v = es.top();
        es.pop();
        es.push(~v);
      } break;
      case CMD_XOR: {
        BIN_OPS(v1, v2);
        es.push(v1 ^ v2);
      } break;

        // bit-shifts
      case CMD_SHR: {
        BIN_OPS(v1, v2);
        es.push((uint64_t)v1 >> (uint64_t)v2);
      } break; // Logical shift right
      case CMD_SHL: {
        BIN_OPS(v1, v2);
        es.push((uint64_t)v1 << (uint64_t)v2);
      } break; // Logical shift left
      case CMD_SAR: {
        BIN_OPS(v1, v2);
        es.push(v1 >> v2);
      } break; // Arethmetic shift right
      case CMD_SAL: {
        BIN_OPS(v1, v2);
        es.push(v1 << v2);
      } break; // Arethmetic shift left

        // Boolean operations
      case CMD_AND: {
        BIN_OPS(v1, v2);
        es.push(v1 && v2);
      } break;
      case CMD_OR: {
        BIN_OPS(v1, v2);
        es.push(v1 || v2);
      } break;
      case CMD_NOT: {
        int64_t v = es.top();
        es.pop();
        es.push(!v);
      } break;

        // Comparison operations
      case CMD_EQ: {
        BIN_OPS(v1, v2);
        es.push(v1 == v2);
      } break;
      case CMD_NE: {
        BIN_OPS(v1, v2);
        es.push(v1 != v2);
      } break;
      case CMD_GT: {
        BIN_OPS(v1, v2);
        es.push(v1 > v2);
      } break;
      case CMD_LT: {
        BIN_OPS(v1, v2);
        es.push(v1 < v2);
      } break;
      case CMD_GTE: {
        BIN_OPS(v1, v2);
        es.push(v1 >= v2);
      } break;
      case CMD_LTE: {
        BIN_OPS(v1, v2);
        es.push(v1 <= v2);
      } break;

        // Control flow operations
      case CMD_JMP: {
        ip = es.top();
        es.pop();
      } break;
      case CMD_JMPZ: {
        BIN_OPS(v1, v2);
        if (v1) {
          ip = v2;
        }
      } break;

        // Built in operations
      case CMD_MALLOC: {
        int64_t v = es.top();
        es.pop();
        size_t ptr = (size_t)malloc(v);
        es.push(size + ptr);
      } break;
      case CMD_FREE: {
        int64_t v = es.top();
        es.pop();
        free((void *)(v - size));
      } break;
      case CMD_WRITE: {
        BIN_OPS(length, ptr);
        std::cout.write(getPtr(buf, size, ptr), length);
      } break;
      case CMD_READ: {
        BIN_OPS(length, ptr);
        std::cin.read(getPtr(buf, size, ptr), length);
      } break;
      default: {
        std::cerr << "ERROR: Unrecognized Command " << inst << "\n";
        exit(-1);
      } break;
      }
    } else {
      es.push(inst >> 1);
    }
  }
}

int main(int argv, char **argc) {
  if (argv != 2) {
    std::cerr << "Usage: ejvm PROGRAM\n";
  }

  std::ifstream prgm;
  prgm.open(argc[1]);

  prgm.seekg(0, std::ios::end);
  int64_t size = prgm.tellg();
  prgm.seekg(0, std::ios::beg);

  char buffer[size];
  if (prgm.read(buffer, size)) {
    prgm.close();

    run(buffer, size);

  } else {
    std::cerr << "Unable to open program file: " << argc[1] << "\n";
  }
}
