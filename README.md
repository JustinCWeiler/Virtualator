# Virtualator

## Description

The Virtualator is a "virtual emulator" designed to make testing bare metal code
fast and easy. It's neither "virtual" nor an "emulator", as it neither
virtualizes the instructions nor purely emulates them.

Rather, it compiles the source code into x86, catches all memory accesses
dynamically, and emulates only those and the related devices.

This gives Virtualator the advantage of being much faster than naive emulation.
However, it also therefore cannot emulate assembly at all without static
translation during compilation, or doing it dynamically (which would likely
be more of a hassle). It's also possible that there are certain side effects
that compiling to x86 have that don't happen when compiling to the emulation
target architecture.

When it is started, Virtualator will look for a config file (name TBD) that has
information about the program to emulate. Namely, it will contain the filename
of the program itself and what address the first instruction is.

## Dependencies

1. [Zydis](https://github.com/zyantific/zydis)
   (included as a submodule and linked statically)
