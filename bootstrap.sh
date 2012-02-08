#!/bin/bash

aclocal -I m4 --force
autopoint --force
libtoolize --copy --force
autoheader --force
automake --add-missing --copy --force
autoconf --force
