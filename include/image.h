#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "compression.h"

#define BANK_WIDTH  256
#define BANK_HEIGHT 256

#define DISPLAY_WIDTH  200
#define DISPLAY_HEIGHT 160

#define BUF_LENGTH (BANK_WIDTH * BANK_HEIGHT)

void image_encode(uint8_t *in, uint8_t *out, size_t *out_len, int w, int h, bool compressed, bool dithered);
void image_decode(uint8_t *in, uint8_t *in_end, uint8_t *out, int w, int h, bool compressed);
