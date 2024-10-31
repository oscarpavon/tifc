#!/bin/sh

gcc -g -O0 -Ideps -Ldeps tifc.c canvas.c display.c input.c frame.c -lhashmap_static -lsparse_static -ldynarr_static -lvector_static -lm
