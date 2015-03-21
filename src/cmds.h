#ifndef COMMAND
#define COMMAND(a, b)
#endif
COMMAND(load, 0x1)
COMMAND(store, 0x3)
COMMAND(exit, 0x5)
COMMAND(loadbyte, 0x7)
COMMAND(storebyte, 0x9)

// Basic Math!
COMMAND(add, 0x11)
COMMAND(sub, 0x13)
COMMAND(mul, 0x15)
COMMAND(div, 0x17)
COMMAND(mod, 0x19)
COMMAND(negate, 0x1B)

// Stack operations
COMMAND(dup, 0x21)
COMMAND(swap, 0x23)
COMMAND(pop, 0x25)

// Bitwise operations
COMMAND(band, 0x31)
COMMAND(bor, 0x33)
COMMAND(bnot, 0x35)
COMMAND(xor, 0x37)

// bit-shifts
COMMAND(shr, 0x39) // Logical shift right
COMMAND(shl, 0x3B) // Logical shift left
COMMAND(sar, 0x3D) // Arethmetic shift right
COMMAND(sal, 0x3F) // Arethmetic shift left

// Boolean operations
COMMAND(and, 0x41)
COMMAND(or, 0x43)
COMMAND(not, 0x45)

// Comparison operations
COMMAND(eq, 0x47)
COMMAND(ne, 0x49)
COMMAND(gt, 0x4B)
COMMAND(lt, 0x4D)
COMMAND(gte, 0x4F)
COMMAND(lte, 0x51)

// Control flow operations
COMMAND(jmp, 0x61)
COMMAND(jmpz, 0x63)

// Built in operations
COMMAND(malloc, 0xA1)
COMMAND(free, 0xA3)
COMMAND(write, 0xA5)
COMMAND(read, 0xA7)
COMMAND(memcpy, 0xA9)
