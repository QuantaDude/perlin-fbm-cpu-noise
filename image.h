//https://github.com/nothings/stb/blob/master/stb_image_write.h
#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

int write_grayscale_png(const char *filename, const uint8_t *pixels, int width,
                        int height);

int write_rgb_png(const char *filename,
                  const uint8_t *pixels,
                  int width,
                  int height);

int write_rgba_png(const char *filename,
                   const uint8_t *pixels,
                   int width,
                   int height);
#endif
