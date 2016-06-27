#!/bin/bash

set -e
set -u
set -x

SRC=./terminator.cc
CCFLAGS="-std=c++11 -O0 -g3"

clang++ $SRC $CCFLAGS -o terminator
