#!/bin/sh

CFLAGS="-O0 -g -Wall -Wextra -Wno-override-init"

git submodule update --init --recursive --remote
git submodule foreach './autogen.sh; make distclean; ./configure && make -j8'


