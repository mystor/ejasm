erijake VM
==========

A really really simple stack-based VM, inspired by Erik and Jake.

Included is:
a) An assembler which takes eja assembly, and compiles it to ejo object code
b) A virtual machine, which emulates an ejo machine and executes it

## Building
To build the `ejasm` and `ejvm` binaries, run the following:
```
$ cmake .
$ make
```

Assembling a file:
```
$ ejasm in.eja -o out.ejo
```

Running the program
```
$ ejvm prgm.ejo
```

## Machine Documentation
Every command in the ej machine code is 64-bits wide, and represented as a little-endian signed integer. The program starts executing at address 0x0, and proceeds one instruction at a time. The commands and their function are documented below:

    TODO
