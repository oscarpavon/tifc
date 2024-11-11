#!/bin/sh

CFLAGS="
    -g
    -O0
    -std=gnu18
    -Wall
    -Wextra
    -Wno-override-init
    -Werror
"

CPPFLAGS="
    -Ldeps
    -Ideps
    -Iui
    -I.
"

OBJECTS="
    tifc.c
    canvas.c
    display.c
    input.c
    frame.c
"

LIBS="
    -lcircbuf_static
    -lhashmap_static
    -lsparse_static
    -ldynarr_static
    -lvector_static
    -lm
"

gcc $CPPFLAGS $CFLAGS $OBJECTS $LIBS

