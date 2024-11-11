#!/bin/sh

CFLAGS='-std=gnu18 -Wall -Wextra -Wno-override-init -Werror'
gcc -g -O0 $CFLAGS input_test.c input.c -L deps -I deps -lcircbuf_static -lvector_static -lhashmap_static -o input_test
