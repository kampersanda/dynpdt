# Dynpdt: dynamic path-decomposed trie

This experimental library implements a space-efficient dynamic keyword dictionary, namely DynPDT, described in

- S. Kanda, K. Morita, and M. Fuketa. Practical implementation of space-efficient dynamic keyword dictionaries. In _Proc. SPIRE_, 2017. (accepted) [[pdf](https://sites.google.com/site/shnskknd/SPIRE2017.pdf)]

You can tentatively build and run the source code using two shell scripts and a sample dataset.

## Build instructions

You can download and compile Dynpdt as the following commands:

```
$ git clone https://github.com/kamp78/dynpdt.git
$ cd dynpdt
$ mkdir build
$ cd build
$ cmake ..
$ make
```

If you want to use the SSE4.2 POPCNT instruction by adding `-DDYNPDT_USE_POPCNT=ON`.
Note that, the source code has been tested only on Mac OS X and Linux. That is, this library considers only UNIX-compatible OS.
