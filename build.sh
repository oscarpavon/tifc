#!/bin/sh

gcc -g -I. -L. tifc.c canvas.c display.c input.c frame.c -lhashmap_static -lsparse_static -ldynarr_static -lvector_static -lm
