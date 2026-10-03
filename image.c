#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "image.h"

int write_grayscale_png(const char *filename, const uint8_t *pixels, int width,
                        int height) {
  return stbi_write_png(filename, width, height,
                        1, // channels: grayscale
                        pixels,
                        width // stride in bytes
  );

  
}
int write_rgb_png(const char *filename,
                  const uint8_t *pixels,
                  int width,
                  int height) {
    return stbi_write_png(
        filename,
        width,
        height,
        3,              // RGB
        pixels,
        width * 3       // stride: 3 bytes per pixel
    );
}

int write_rgba_png(const char *filename,
                   const uint8_t *pixels,
                   int width,
                   int height) {
    return stbi_write_png(
        filename,
        width,
        height,
        4,              // RGBA
        pixels,
        width * 4       // stride: 4 bytes per pixel
    );
}
