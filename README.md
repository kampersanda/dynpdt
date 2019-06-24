# Dynpdt: dynamic path-decomposed trie

This experimental library implements a space-efficient dynamic keyword dictionary, namely DynPDT, described in

- S. Kanda, K. Morita, and M. Fuketa. Practical implementation of space-efficient dynamic keyword dictionaries. In _Proc. SPIRE_, 2017. (accepted) [[pdf](https://drive.google.com/open?id=1SoCfb4IV51aS5wgQBVuEkIx9L-fK6_ke)]

You can tentatively build and run the source code using two shell scripts and a sample dataset.

## Notice

[`poplar-trie`](https://github.com/kampersanda/poplar-trie) is an enhanced version of this library.

## Build instructions

You can download and compile Dynpdt as the following commands:

```
$ git clone git@github.com:kampersanda/dynpdt.git
$ cd dynpdt
$ mkdir build
$ cd build
$ cmake ..
$ make
```

If you want to use the SSE4.2 POPCNT instruction by adding `-DDYNPDT_USE_POPCNT=ON`.
Note that, the source code has been tested only on Mac OS X and Linux. That is, this library considers only UNIX-compatible OS.
