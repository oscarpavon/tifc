#!/bin/sh

gcc -g -I. -L. tifc.c canvas.c display.c input.c frame.c -lsparse_static -ldynarr_static -lvector_static -lm
