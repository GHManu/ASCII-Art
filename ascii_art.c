/*
 * ASCII Art Colorata in C
 * Dipendenze: stb_image.h (scarica da https://github.com/nothings/stb)
 *
 * Compilazione:
 *   gcc -O2 -o ascii_art ascii_art.c -lm
 *
 * Uso:
 *   ./ascii_art immagine.png
 *   ./ascii_art immagine.jpg 120        (larghezza 120 caratteri)
 *   ./ascii_art immagine.png 100 1.5    (larghezza 100, contrasto 1.5)
 *   ./ascii_art immagine.png 100 1.5 20 (larghezza 100, contrasto 1.5, luminosità +20)
 */

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ── Charset disponibili ─────────────────────────────────────────────────── */
static const char *CHARSET_DETAILED = "@#S%?*+;:,. ";
static const char *CHARSET_BLOCKS   = "@%#*+=-:. "; /* blocchi unicode non vanno bene sul terminale C */
static const char *CHARSET_MINIMAL  = "@:. ";
static const char *CHARSET_SIMPLE   = "Ww:. ";

/* ── Codici ANSI per colori true-color (24-bit) ──────────────────────────── */
/* \033[38;2;R;G;Bm  → imposta colore foreground RGB */
#define ANSI_COLOR(r,g,b) printf("\033[38;2;%d;%d;%dm", (r), (g), (b))
#define ANSI_RESET()      printf("\033[0m")

/* ── Clamp: mantieni un valore nell'intervallo [min, max] ────────────────── */
static inline int clamp(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

/* ── Applica contrasto e luminosità a un canale ──────────────────────────── */
/*    Formula contrasto identica a quella usata in Photoshop                  */
static inline int apply_contrast_channel(int val, double contrast, int brightness) {
    double factor = (259.0 * (contrast * 128.0 + 255.0))
                  / (255.0 * (259.0 - contrast * 128.0));
    double result = factor * (val - 128.0) + 128.0 + brightness;
    return clamp((int)round(result), 0, 255);
}

/* ── Luminosità percettiva ITU-R BT.601 ─────────────────────────────────── */
static inline double luminance(int r, int g, int b) {
    return 0.299 * r + 0.587 * g + 0.114 * b;
}

/* ── Stampa usage ed esce ────────────────────────────────────────────────── */
static void usage(const char *prog) {
    fprintf(stderr,
        "Uso: %s <immagine> [larghezza] [contrasto] [luminosità]\n"
        "  larghezza   : numero di colonne (default: 100)\n"
        "  contrasto   : 0.5 – 2.5         (default: 1.2)\n"
        "  luminosità  : -80 – 80          (default: 0)\n"
        "\nEsempio:\n"
        "  %s foto.jpg 120 1.4 10\n", prog, prog);
    exit(EXIT_FAILURE);
}

/* ═══════════════════════════════════════════════════════════════════════════ */
int main(int argc, char **argv) {
    if (argc < 2) usage(argv[0]);

    /* ── Parametri da riga di comando ────────────────────────────────────── */
    const char *filename  = argv[1];
    int         cols      = (argc >= 3) ? atoi(argv[2]) : 100;
    double      contrast  = (argc >= 4) ? atof(argv[3]) : 1.2;
    int         brightness= (argc >= 5) ? atoi(argv[4]) : 0;

    if (cols < 10 || cols > 400) {
        fprintf(stderr, "Larghezza deve essere tra 10 e 400\n");
        return EXIT_FAILURE;
    }

    /* ── Carica immagine con stb_image ───────────────────────────────────── */
    /*    n = numero canali originali; forziamo RGBA (4 canali)               */
    int orig_w, orig_h, n;
    unsigned char *img = stbi_load(filename, &orig_w, &orig_h, &n, 4);
    if (!img) {
        fprintf(stderr, "Errore: impossibile aprire '%s'\n", filename);
        fprintf(stderr, "stb_image: %s\n", stbi_failure_reason());
        return EXIT_FAILURE;
    }

    /* ── Calcola dimensioni dell'output ──────────────────────────────────── */
    /*    I caratteri monospace sono circa 2x più alti che larghi (aspect 0.55) */
    const double CHAR_ASPECT = 0.55;
    int rows = (int)round((double)orig_h / orig_w * cols * CHAR_ASPECT);
    if (rows < 1) rows = 1;

    /* ── Charset ─────────────────────────────────────────────────────────── */
    const char *charset = CHARSET_DETAILED;
    int charset_len = (int)strlen(charset);

    /* ── Alloca buffer per l'immagine ridimensionata ─────────────────────── */
    /*    Ogni pixel = 4 byte (R, G, B, A)                                    */
    unsigned char *small = (unsigned char *)malloc(cols * rows * 4);
    if (!small) {
        fprintf(stderr, "Errore: memoria insufficiente\n");
        stbi_image_free(img);
        return EXIT_FAILURE;
    }

    /* ── Ridimensionamento bilineare semplice ────────────────────────────── */
    /*    Per ogni pixel dell'immagine piccola, campiona il pixel corrispondente
         nell'immagine originale (nearest-neighbor; sufficiente per ASCII art) */
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            /* Coordinate nel sorgente (con clamp ai bordi) */
            int src_x = clamp((int)((double)x / cols * orig_w),  0, orig_w  - 1);
            int src_y = clamp((int)((double)y / rows * orig_h),  0, orig_h  - 1);

            int src_idx = (src_y * orig_w + src_x) * 4;
            int dst_idx = (y     * cols   + x)     * 4;

            small[dst_idx + 0] = img[src_idx + 0]; /* R */
            small[dst_idx + 1] = img[src_idx + 1]; /* G */
            small[dst_idx + 2] = img[src_idx + 2]; /* B */
            small[dst_idx + 3] = img[src_idx + 3]; /* A */
        }
    }

    stbi_image_free(img); /* immagine originale non serve più */

    /* ── Render ASCII ────────────────────────────────────────────────────── */
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            int base = (y * cols + x) * 4;

            /* Leggi canali e applica contrasto/luminosità */
            int r = apply_contrast_channel(small[base + 0], contrast, brightness);
            int g = apply_contrast_channel(small[base + 1], contrast, brightness);
            int b = apply_contrast_channel(small[base + 2], contrast, brightness);
            int a = small[base + 3];

            /* Pixel trasparente -> spazio */
            if (a < 128) {
                putchar(' ');
                continue;
            }

            /* Luminosità percettiva -> indice nel charset */
            double lum = luminance(r, g, b);
            int idx = (int)(lum / 255.0 * (charset_len - 1));
            idx = charset_len - 1 - idx; /* inverti: buio = char pesante */
            idx = clamp(idx, 0, charset_len - 1);

            char ch = charset[idx];

            if (ch == ' ') {
                putchar(' ');
            } else {
                /* Imposta colore ANSI true-color e stampa il carattere */
                ANSI_COLOR(r, g, b);
                putchar(ch);
                ANSI_RESET();
            }
        }
        putchar('\n');
    }

    free(small);
    return EXIT_SUCCESS;
}
