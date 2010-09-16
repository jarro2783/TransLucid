#!/bin/sh
# Run a binary file, adding one or more directories 
# temporarily to LD_LIBRARY_PATH and passing the remaining arguments to
# the binary

if [ "$#" -lt "2" ]
then
  echo "Usage: runbinary.sh libraryPath binaryFile args"
  exit 1
fi

export LD_LIBRARY_PATH=$1:$LD_LIBRARY_PATH
BINARY=$2

shift 2

$BINARY "$@"
