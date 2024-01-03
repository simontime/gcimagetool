#include "image.h"

// Palette for default LCC value (TODO: support others?)
const uint8_t palette[] = { 0xff, 0xc0, 0x80, 0x00 };

// Buffers for image coding
static uint8_t buf1[BUF_LENGTH];
static uint8_t buf2[BUF_LENGTH];

// Clamp int to byte range
int saturate(int n)
{
    if (n < 0x00) return 0x00;
    if (n > 0xff) return 0xff;

    return n;
}

// Find closest palette shade for pixel
uint8_t palette_match_closest(uint8_t px)
{
    int i, best, diff, match;

    best = 0x100;

    // Iterate through palette entries
    for (i = 0; i < sizeof(palette); i++)
    {
        // Calculate difference between pixel and palette shade
        diff = abs(px - palette[i]);

        // Set match if closer
        if (best > diff)
        {
            best  = diff;
            match = palette[i];
        }
    }

    return match;
}

// Pixel -> 2-bit palette index
uint8_t palette_index(uint8_t px)
{
    switch (px)
    {
        case 0xff: return 0;
        case 0xc0: return 1;
        case 0x80: return 2;
        case 0x00: return 3;
    }
}

// Performs error-diffusion dithering on image data to reduce grey scales
void reduce_dither(uint8_t *buf, int w, int h)
{
    int error, match, px, x, y;

    for (y = 0; y < h; y++)
    for (x = 0; x < w; x++)
    {
        // Get original pixel
        px = buf[(y * w) + x];

        // Find closest palette match for pixel
        match = palette_match_closest(px);

        // Error is the difference between the original pixel and its closest palette match
        error = px - match;

        // Store palette match
        buf[(y * w) + x] = match;

        // Diffuse error to neighbouring pixels (within bounds)
        if (x + 1 < w)               buf[ (y      * w) + (x + 1)] = saturate(buf[ (y      * w) + (x + 1)] + ((error * 7) / 16));    
        if (y + 1 < h && x - 1 >= 0) buf[((y + 1) * w) + (x - 1)] = saturate(buf[((y + 1) * w) + (x - 1)] + ((error * 3) / 16));
        if (y + 1 < h)               buf[((y + 1) * w) +  x     ] = saturate(buf[((y + 1) * w) +  x     ] + ((error * 5) / 16));
        if (y + 1 < h && x + 1 < w)  buf[((y + 1) * w) + (x + 1)] = saturate(buf[((y + 1) * w) + (x + 1)] + ((error * 1) / 16));
    }
}

// Finds nearest palette match for each pixel in image data to reduce grey scales
void reduce_palettise(uint8_t *buf, int w, int h)
{
    int i;

    for (i = 0; i < w * h; i++)
        buf[i] = palette_match_closest(buf[i]);
}

// Encode 8-bit image to 2-bit game.com image data
void image_encode(uint8_t *in, uint8_t *out, size_t *out_len, int w, int h, bool compressed, bool dithered)
{
    int      i, x, y;
    uint8_t *buf_ptr1, *buf_ptr2, ch;
    size_t   img_len;

    // Set pointers
    buf_ptr1 = buf1;
    buf_ptr2 = buf2;

    // Calculate image length
    img_len = (w * h) / 4;

    // Reduce grey scales
    dithered ? reduce_dither(in, w, h) : reduce_palettise(in, w, h);

    // Rotate 270 degrees and flip horizontally
    for (y = 0; y < h; y++)
    for (x = 0; x < w; x++)
        buf1[y + ((w - x - 1) * h)] = in[y * w + (w - x - 1)];

    // Pack image data
    for (i = 0; i < img_len; i++)
    {
        ch  = palette_index(*buf_ptr1++) << 6;
        ch |= palette_index(*buf_ptr1++) << 4;
        ch |= palette_index(*buf_ptr1++) << 2;
        ch |= palette_index(*buf_ptr1++);

        *buf_ptr2++ = ch;
    }

    if (compressed)
    {
        // Compress and set output length
        *out_len = compress(buf2, buf2 + img_len, out);
    }
    else
    {
        // Copy buffer to output
        memcpy(out, buf2, img_len);

        // Set output length
        *out_len = img_len;
    }
}

// Decode 2-bit game.com image data to 8-bit image
void image_decode(uint8_t *in, uint8_t *in_end, uint8_t *out, int w, int h, bool compressed)
{
    int      i, x, y;
    uint8_t *buf_ptr, *buf_start, *img;

    if (compressed)
    {
        // Decompress input to buffer 1
        decompress(in, in_end, buf1);

        // Image in buffer 1
        img = buf1;

        // Use buffer 2
        buf_start = buf_ptr = buf2;
    }
    else
    {
        // Image in input
        img = in;

        // Use buffer 1
        buf_start = buf_ptr = buf1;
    }

    // Unpack image data
    for (i = 0; i < (w * h) / 4; i++)
    {
        *buf_ptr++ = palette[(img[i] >> 6) & 3];
        *buf_ptr++ = palette[(img[i] >> 4) & 3];
        *buf_ptr++ = palette[(img[i] >> 2) & 3];
        *buf_ptr++ = palette[(img[i] >> 0) & 3];
    }

    // Rotate 270 degrees and flip horizontally
    for (y = 0; y < h; y++)
    for (x = 0; x < w; x++)
        out[y * w + (w - x - 1)] = buf_start[y + ((w - x - 1) * h)];
}
