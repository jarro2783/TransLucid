#!/bin/bash

echo "Invoking aclocal"
aclocal -I m4 --force
echo "Invoking autopoint"
autopoint --force
if (test "X`which glibtoolize`" != "X")
then
  LIBTOOLIZE=glibtoolize
elif (test "X`which libtoolize`" != "X")
then
  LIBTOOLIZE=libtoolize
else 
  echo "No libtoolize"
  exit 1
fi
echo "Invoking $LIBTOOLIZE"
$LIBTOOLIZE --copy --force
echo "Invoking autoheader"
autoheader --force
echo "Invoking automake"
automake --add-missing --copy --force
echo "Invoking autoconf"
autoconf --force
