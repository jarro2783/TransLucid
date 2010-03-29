#!/bin/sh
# Run a binary file, adding one or more directories temporarily to LD_LIBRARY_PATH

if [ "$#" -ne "2" ]
then
  echo "Usage: runbinary.sh libraryPath binaryFile"
  exit 1
fi

export LD_LIBRARY_PATH=$1:$LD_LIBRARY_PATH
./$2
