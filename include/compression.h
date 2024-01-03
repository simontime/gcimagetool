#pragma once

#include <stddef.h>
#include <stdint.h>

size_t compress(uint8_t *in, uint8_t *in_end, uint8_t *out);
size_t decompress(uint8_t *in, uint8_t *in_end, uint8_t *out);
