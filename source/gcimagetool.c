#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Buffers for image coding
static uint8_t buf1[BUF_LENGTH];
static uint8_t buf2[BUF_LENGTH];

// Verifies image dimensions and prints relevant errors
bool verify_image_dimensions(int width, int height, bool compress)
{
	if (!compress) // Uncompressed limited to 256x256 (bank size)
    {
        if (width > BANK_WIDTH)
        {
            fputs("Error: Image width cannot be greater than 256px.\n", stderr);
            return false;
        }

        if (height > BANK_HEIGHT)
        {
            fputs("Error: Image height cannot be greater than 256px.\n", stderr);
            return false;
        }
    }
    else // Compressed limited to 200x160 (screen size)
    {
        if (width > DISPLAY_WIDTH)
        {
            fputs("Error: Image width cannot be greater than 200px.\n", stderr);
            return false;
        }

        if (height > DISPLAY_HEIGHT)
        {
            fputs("Error: Image height cannot be greater than 160px.\n", stderr);
            return false;
        }
    }

	return true;
}

int main(int argc, char **argv)
{
	bool     compressed, dithered, encode;
	FILE    *in, *out;
    int      i, width, height, ch;
	uint8_t *img;
	size_t   in_len, out_len;

    if (argc < 4)
    {
print_usage:
        printf("SimonTime's game.com image tool\n\n"
               "Usage: %s option in out [width] [height] [-c] [-n]\n\n"
			   "Arguments:\n"
               "   option: encode, decode\n"
               "       in: input filename\n"
               "      out: output filename\n"
               "   width*: image width (px)\n"
               "  height*: image height (px)\n\n"
               "Optional arguments:\n"
               "       -c: (de)compress\n"
               "       -n: no dithering\n\n"
			   "* decoding only\n", argv[0]);
        return 0;
    }

	// Parse option
	if (strcmp(argv[1], "encode") == 0)
		encode = true;
	else if (strcmp(argv[1], "decode") == 0)
		encode = false;
	else
		goto print_usage;

	// Set defaults
	compressed = false;
	dithered   = true;

    if (encode)
    {
		// Ensure no more than 6 arguments
		if (argc > 6)
			goto print_usage;

		// Check remaining arguments
		for (i = 4; i < argc; i++)
		{
			if (argv[i][0] != '-')
				goto print_usage;

			// Parse character and set
			switch (argv[i][1])
			{
				case 'c':
					compressed = true;
					break;
				case 'n':
					dithered = false;
					break;
				default:
					goto print_usage;
			}
		}
		
		// Read input file
        img = stbi_load(argv[2], &width, &height, &ch, STBI_grey);

        if (img == NULL)
        {
            fprintf(stderr, "Error reading input file: %s\n", stbi_failure_reason());
            return 1;
        }

		// Open output file
        if ((out = fopen(argv[3], "wb")) == NULL)
        {
            perror("Error opening output file");
            return 1;
        }

		// Verify image dimensions
		if (!verify_image_dimensions(width, height, compressed))
			return 1;

		// Encode image
		image_encode(img, buf1, &out_len, width, height, compressed, dithered);
		stbi_image_free(img);

		// Write output file
		fwrite(buf1, 1, out_len, out);
        fclose(out);
    }
    else
    {
		// Ensure 6-7 arguments
		if (argc < 6 || argc > 7)
			goto print_usage;

		// Parse width and height
		width  = atoi(argv[4]);
        height = atoi(argv[5]);

		// Ensure valid numbers
		if (width == 0 || height == 0)
			goto print_usage;

		// Check compress argument
		if (argc == 7)
		{
			if (argv[6][0] == '-' && argv[6][1] == 'c')
				compressed = true;
			else
				goto print_usage;
		}

		// Verify image dimensions
		if (!verify_image_dimensions(width, height, compressed))
			return 1;

		// Open input file
		if ((in = fopen(argv[2], "rb")) == NULL)
        {
            perror("Error opening input file");
            return 1;
        }

		// Open output file
        if ((out = fopen(argv[3], "wb")) == NULL)
        {
            perror("Error opening output file");
            return 1;
        }

		// Read input file size
		fseek(in, 0, SEEK_END);
        in_len = ftell(in);
        rewind(in);

		// Read input file
        fread(buf1, 1, in_len, in);
        fclose(in);

		// Decode image
		image_decode(buf1, buf1 + in_len, buf2, width, height, compressed);

		// Write output file
		if (stbi_write_png(argv[3], width, height, 1, buf2, width) == 0)
		{
            fputs("Error writing output file.\n", stderr);
            return 1;
		}
    }

    puts("Done!");

    return 0;
}
