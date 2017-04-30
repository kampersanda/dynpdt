#!/bin/sh

mkdir build
cd build
cmake ..
make
cd ..

bzip2 -d sample.txt.bz2
