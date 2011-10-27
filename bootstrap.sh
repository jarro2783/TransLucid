#!/bin/bash

aclocal -I m4 --force
libtoolize --copy --force
autoheader --force
automake --add-missing --copy --force
autoconf --force
