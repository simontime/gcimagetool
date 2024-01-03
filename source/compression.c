#include "compression.h"

size_t compress(uint8_t *in, uint8_t *in_end, uint8_t *out)
{
    uint8_t  ch, *out_start;
    uint16_t rl;

	out_start = out;

    while (in < in_end)
    {
        // Read byte
        ch = *in++;

        // If not EOF and new byte matches last, compress
        if (in < in_end && ch == *in)
        {
            // Count number of bytes to compress
            for (rl = 2, in++; in < in_end && ch == *in; rl++, in++)
                ;

            // Write control byte and run length
            if (rl < 64) // Under 64 bytes
            {
                // Write 2 byte token
                *out++ = 0xc0 | rl;
                *out++ = ch;
            }
            else // 64 bytes and over
            {
                // Write 4 byte token
                *out++ = 0xc0;
                *out++ = rl & 0xff;
                *out++ = rl >> 8;
                *out++ = ch;
            }
        }
        else
        {
            // Any literal under 0xc0 is written verbatim
            if (ch < 0xc0)
            {
                *out++ = ch;
            }
            else // Literals 0xc0 and over
            {
                // Write literal marker
                *out++ = 0xc1;
                *out++ = ch;
            }
        }
    }

    // Compressed length
    return out - out_start;
}

size_t decompress(uint8_t *in, uint8_t *in_end, uint8_t *out)
{
	uint8_t  ch, *out_start;
	uint16_t rl;

	out_start = out;

	while (in < in_end)
	{
		ch = *in++;

		if (ch < 0xc0) // Literal (in range)
		{
			*out++ = ch;
		}
		else if (ch == 0xc0) // RLE (16-bit)
		{
			rl = (in[1] << 8) | in[0];
			ch = in[2];
			in += 3;

			while (rl--)
				*out++ = ch;
		}
		else // RLE (6-bit)
		{
			rl = ch & ~0xc0;
			ch = *in++;

			while (rl--)
				*out++ = ch;
		}
	}

	// Decompressed length
	return out - out_start;
}
