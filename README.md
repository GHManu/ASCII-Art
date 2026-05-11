# ASCII-Art
* This program, written in **C**, converts digital images (JPG, PNG, etc.) into colored ASCII text that can be displayed directly in the terminal, using the `stb_image` library for pixel decoding and ANSI color codes.

- curl -O https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
- gcc -O2 -o ascii_art ascii_art.c -lm

> ./ascii_art img.png           # default: 100 colonne
> ./ascii_art img.png 150 1.5 20  # larghezza, contrasto, luminosità