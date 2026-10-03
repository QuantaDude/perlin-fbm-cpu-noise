#include "image.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define GRID_SIZE 256

typedef struct {
  float x;
  float y;
} Vec2;

Vec2 gradients[GRID_SIZE][GRID_SIZE];

Vec2 GetRandomGradient() {
  struct timeval tv;

  gettimeofday(&tv, NULL);
  srand(tv.tv_sec ^ tv.tv_usec ^ getpid());
  static const Vec2 gradients[] = {

      {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {-1, 1}, {1, -1}, {-1, -1},

  };

  return gradients[rand() % 8];
}

float Vec2DotProduct(Vec2 v1, Vec2 v2) { return v1.x * v2.x + v1.y * v2.y; }

float Lerp(float p, float start, float end) {
  return start + p * (end - start);
}
float Fade(float t) { return ((6 * t - 15) * t + 10) * t * t * t; }

float Noise2D(Vec2 p) {

  float x0 = floorf(p.x);
  float y0 = floorf(p.y);

  float x = p.x - x0;
  float y = p.y - y0;

  const Vec2 bLVector = (Vec2){.x = x, .y = y};
  const Vec2 bRVector = (Vec2){.x = x - 1.0f, .y = y};
  const Vec2 tLVector = (Vec2){.x = x, .y = y - 1.0f};
  const Vec2 tRVector = (Vec2){.x = x - 1.0f, .y = y - 1.0f};

  int iX = (int)x0 & 255;
  int iY = (int)y0 & 255;
  int ix = (iX + 1) & 255;
  int iy = (iY + 1) & 255;
  const Vec2 bLGradient = gradients[iy][ix];
  const Vec2 bRGradient = gradients[iy][ix + 1];
  const Vec2 tLGradient = gradients[iy + 1][ix];
  const Vec2 tRGradient = gradients[iy + 1][ix + 1];

  const float dotBL = Vec2DotProduct(bLVector, bLGradient);
  const float dotBR = Vec2DotProduct(bRVector, bRGradient);
  const float dotTL = Vec2DotProduct(tLVector, tLGradient);
  const float dotTR = Vec2DotProduct(tRVector, tRGradient);

  const float u = Fade(x);
  const float v = Fade(y);

  const float result = Lerp(u, Lerp(v, dotBL, dotTL), Lerp(v, dotBR, dotTR));
  return result;
}
float fbm(Vec2 pos, float H) {

  float G = pow(2, -H);
  float f = 1.0f;
  float a = 1.0f;
  float a_Sum = 0.0;
  float t = 0.0f;

  for (int i = 0; i < 6; i++) {

    t += a * Noise2D((Vec2){.x = pos.x * f, .y = pos.y * f});
    f *= 2.0f;
    a_Sum += a / 2;
    a *= G;
  }

  return t / a_Sum;
}
void generateRandomNoise(uint8_t *cell, int width, int height, int offset_x,
                         int offset_y, int size) {
  struct timeval tv;

  gettimeofday(&tv, NULL);
  srand(tv.tv_sec ^ tv.tv_usec ^ getpid());
  for (int y = 0; y < size && y < offset_y + height; y++) {
    for (int x = 0; x < size && x < offset_x + width; x++) {
      cell[((y + offset_y) * width) + (x + offset_x)] = rand() % 256;
    }
  }
}

int main(void) {

  for (int y = 0; y < GRID_SIZE; y++) {
    for (int x = 0; x < GRID_SIZE; x++) {
      gradients[y][x] = GetRandomGradient();
    }
  }
  const int width = 500;
  const int height = 500;

  uint8_t *pixels = malloc(width * height);

  if (!pixels)
    return 1;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      // generateRandomNoise(pixels, width, height, x, y, 10);

      // float n = Noise2D((Vec2){.x = x * 0.1f, .y = y * 0.1f});

      Vec2 q = (Vec2){
          .x = fbm((Vec2){.x = x * 0.001f, .y = y * 0.001f}, 0.4f),
          .y = fbm((Vec2){.x = x * 0.001f + 5.2f, .y = y * 0.001f + 1.3f},
                   0.4f)};
      float n = fbm((Vec2){.x = x * 0.005f + (4.0f * q.y),
                           .y = y * 0.005f + (4.0f * q.x)},
                    0.4f);
      // float n = fbm((Vec2){.x = x * 0.01f, .y = y * 0.01f}, 0.6f);

      n += 1.0f;
      n /= 2.0f;

      pixels[y * width + x] = (int)roundf(255 * n) & 255;
      // pixels[y * width + x] = (uint16_t)roundf(fabs(n) * 255) & 255;
    }
  }
  // for (int y = 0; y < height; ++y) {
  //   for (int x = 0; x < width; ++x) {
  //     pixels[y * width + x] = (uint8_t)(x * 255 / width);
  //   }
  // }

  write_grayscale_png("noise2.png", pixels, width, height);

  free(pixels);

  return 0;
}
