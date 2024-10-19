#!/bin/sh

gcc -I. -L. tifc.c canvas.c display.c input.c -lsparse_static -ldynarr_static -lvector_static -lm
