#!/bin/bash

ROOT=$1
BUILDDIR=$2

cd $BUILDDIR

export PATH=$ROOT/bin:$PATH
export LD_LIBRARY_PATH=$ROOT/lib:$ROOT/lib64/:$LD_LIBRARY_PATH

ctest -D Nightly
