#!/bin/bash

aclocal -I m4
libtoolize --copy
autoheader
automake --add-missing --copy
autoconf