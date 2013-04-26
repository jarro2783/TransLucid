#!/bin/bash

aclocal -I m4 --force
autopoint --force
if (test "X`which glibtoolize`" != "X")
then
  glibtoolize --copy --force
elif (test "X`which libtoolize`" != "X")
then
  libtoolize --copy --force
else 
  echo "No libtoolize"
  exit 1
fi
autoheader --force
automake --add-missing --copy --force
autoconf --force
