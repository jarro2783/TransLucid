#!/bin/bash

#make a copy of this file and set USERNAME to your sourceforge username.
#then run this from the built doc/html directory.

USERNAME=

rsync -avPd -e ssh * ${USERNAME},translucid@web.sourceforge.net:htdocs/doc
