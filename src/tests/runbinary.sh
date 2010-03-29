#!/bin/sh

export LD_LIBRARY_PATH=$1:$LD_LIBRARY_PATH
./$2
