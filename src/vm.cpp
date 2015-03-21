#include <iostream>
#include <fstream>
#include <stack>

enum Command {
#define COMMAND(k, v) CMD_##k = v,
#include "cmds.h"
};

typedef int64_t mword;

struct Buffer {
  char *buf;
  mword size;

  char *ptr(mword addr) {
    if (addr >= 0 && addr < size) {
      return (buf + addr);
    } else {
      // TODO(michael): Make this safer (maybe)
      return (char *)(addr - size);
    }
  }
};

static mword loadWord(Buffer buf, mword ptr) {
  return *(mword *)buf.ptr(ptr);
}

static void storeWord(Buffer buf, mword ptr, mword value) {
  *(mword *)buf.ptr(ptr) = value;
}

static void run(Buffer buf) {
  std::stack<mword> es;
  mword ip = 0;

  for (;;) {
    // Load the instruction
    mword inst = loadWord(buf, ip);
    ip+=sizeof(mword);

#define ARG(a) mword a = es.top(); es.pop()
    if (inst & 0x1) {
      // Non-push
      switch (inst) {
      case CMD_load: {
        mword ptr = es.top();
        es.pop();
        es.push(loadWord(buf, ptr));
      } break;
      case CMD_store: {
        mword v = es.top();
        es.pop();
        storeWord(buf, es.top(), v);
        es.pop();
      } break;
      case CMD_loadbyte: {
        mword ptr = es.top();
        es.pop();
        es.push(*buf.ptr(ptr));
      } break;
      case CMD_storebyte: {
        int8_t v = es.top(); // Cast to a char
        es.pop();
        *buf.ptr(es.top()) = v; // Store it
        es.pop();

      } break;
      case CMD_exit: {
        exit(es.top()); // es.pop() is unnecessary
      } break;

        // Basic Math!
      case CMD_add: {
        ARG(v1); ARG(v2);
        es.push(v1 + v2);
      } break;
      case CMD_sub: {
        ARG(v1); ARG(v2);
        es.push(v1 - v2);
      } break;
      case CMD_mul: {
        ARG(v1); ARG(v2);
        es.push(v1 * v2);
      } break;
      case CMD_div: {
        ARG(v1); ARG(v2);
        es.push(v1 / v2);
      } break;
      case CMD_mod: {
        ARG(v1); ARG(v2);
        es.push(v1 % v2);
      } break;
      case CMD_negate: {
        mword v = es.top();
        es.pop();
        es.push(-v);
      } break;

        // Stack operations
      case CMD_dup: {
        es.push(es.top());
      } break;
      case CMD_swap: {
        ARG(v1); ARG(v2);
        es.push(v2);
        es.push(v1);
      } break;
      case CMD_pop: {
        es.pop();
      } break;

        // Bitwise operations
      case CMD_band: {
        ARG(v1); ARG(v2);
        es.push(v1 & v2);
      } break;
      case CMD_bor: {
        ARG(v1); ARG(v2);
        es.push(v1 | v2);
      } break;
      case CMD_bnot: {
        mword v = es.top();
        es.pop();
        es.push(~v);
      } break;
      case CMD_xor: {
        ARG(v1); ARG(v2);
        es.push(v1 ^ v2);
      } break;

        // bit-shifts
      case CMD_shr: {
        ARG(v1); ARG(v2);
        es.push((uint64_t)v1 >> (uint64_t)v2);
      } break; // Logical shift right
      case CMD_shl: {
        ARG(v1); ARG(v2);
        es.push((uint64_t)v1 << (uint64_t)v2);
      } break; // Logical shift left
      case CMD_sar: {
        ARG(v1); ARG(v2);
        es.push(v1 >> v2);
      } break; // Arethmetic shift right
      case CMD_sal: {
        ARG(v1); ARG(v2);
        es.push(v1 << v2);
      } break; // Arethmetic shift left

        // Boolean operations
      case CMD_and: {
        ARG(v1); ARG(v2);
        es.push(v1 && v2);
      } break;
      case CMD_or: {
        ARG(v1); ARG(v2);
        es.push(v1 || v2);
      } break;
      case CMD_not: {
        mword v = es.top();
        es.pop();
        es.push(!v);
      } break;

        // Comparison operations
      case CMD_eq: {
        ARG(v1); ARG(v2);
        es.push(v1 == v2);
      } break;
      case CMD_ne: {
        ARG(v1); ARG(v2);
        es.push(v1 != v2);
      } break;
      case CMD_gt: {
        ARG(v1); ARG(v2);
        es.push(v1 > v2);
      } break;
      case CMD_lt: {
        ARG(v1); ARG(v2);
        es.push(v1 < v2);
      } break;
      case CMD_gte: {
        ARG(v1); ARG(v2);
        es.push(v1 >= v2);
      } break;
      case CMD_lte: {
        ARG(v1); ARG(v2);
        es.push(v1 <= v2);
      } break;

        // Control flow operations
      case CMD_jmp: {
        ip = es.top();
        es.pop();
      } break;
      case CMD_jmpz: {
        ARG(v1); ARG(v2);
        if (v1) {
          ip = v2;
        }
      } break;

        // Built in operations
      case CMD_malloc: {
        mword v = es.top();
        es.pop();
        size_t ptr = (size_t)malloc(v);
        es.push(buf.size + ptr);
      } break;
      case CMD_free: {
        mword v = es.top();
        es.pop();
        free((void *)(v - buf.size));
      } break;
      case CMD_write: {
        ARG(length); ARG(ptr);
        std::cout.write(buf.ptr(ptr), length);
      } break;
      case CMD_read: {
        ARG(length); ARG(ptr);
        std::cin.read(buf.ptr(ptr), length);
      } break;
      case CMD_memcpy: {
        ARG(n); ARG(src); ARG(dest);
        memcpy(buf.ptr(dest), buf.ptr(src), n);
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
  mword size = prgm.tellg();
  prgm.seekg(0, std::ios::beg);

  char buffer[size];
  if (prgm.read(buffer, size)) {
    prgm.close();

    Buffer buf = { buffer, size };
    run(buf);
  } else {
    std::cerr << "Unable to open program file: " << argc[1] << "\n";
  }
}
