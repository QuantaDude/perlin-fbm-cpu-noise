#include "image.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define WIDTH 500
#define HEIGHT 500

typedef struct {
  float x;
  float y;
} Vec2;

uint32_t seed = 1337;

float 
Vector2DotProduct (Vec2 v1, Vec2 v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

static uint32_t 
hash2 (int32_t x, int32_t y)
{
  uint32_t h = seed;
  h ^= (uint32_t)x * 0x9E3779B1u;
  h = (h << 13) | (h >> 19);
  h *= 0x85EBCA6Bu;
  h ^= (uint32_t)y * 0xC2B2AE35u;
  h ^= h >> 16;  h *= 0x7FEB352Du;
  h ^= h >> 15;  h *= 0x846CA68Bu;
  h ^= h >> 16;
  return h;
}

static Vec2 
GradientAt (int ix, int iy)
{

    Vec2 gradients[8] = {
        { 1.0f,  0.0f},
        {-1.0f,  0.0f},
        { 0.0f,  1.0f},
        { 0.0f, -1.0f},
        //The diagonal gradients here give the noise a directional bias.
        //We can set the diagonal set it to 0.7071 so that the length/ magnitude is the same,
        //or use random theta angles, then divide them with cos and sine
       /*
        { 1,  1},
        {-1,  1},
        { 1, -1},
        {-1, -1},
        */ 
        { 0.7071f,  0.7071f},
        {-0.7071f,  0.7071f},
        { 0.7071f, -0.7071f},
        {-0.7071f, -0.7071f},
    };
    
  return gradients[hash2(ix, iy) & 7];
}

float
Fade (float t)
{
    return (( 6 * t - 15) * t + 10) * t * t * t; 
}

float 
Lerp (float dx, float start, float end)
{
    return start + dx * (end - start);
}

float 
PerlinNoise2D (Vec2 v)
{
    float x = floorf (v.x);
    float y = floorf (v.y);

    float fx = v.x - x;
    float fy = v.y - y;
    
    int iX = (int)x;
    int iY = (int)y;

    Vec2 BottomLeft  = {fx       , fy       };
    Vec2 TopLeft     = {fx       , fy - 1.0f};
    Vec2 BottomRight = {fx - 1.0f, fy       };
    Vec2 TopRight    = {fx - 1.0f, fy - 1.0f};

    Vec2 gBottomLeft  = GradientAt (iX    , iY    );
    Vec2 gTopLeft     = GradientAt (iX    , iY + 1);
    Vec2 gBottomRight = GradientAt (iX + 1, iY    );
    Vec2 gTopRight    = GradientAt (iX + 1, iY + 1);

    float bLProduct = Vector2DotProduct (BottomLeft , gBottomLeft);
    float tLProduct = Vector2DotProduct (TopLeft    , gTopLeft);
    float bRProduct = Vector2DotProduct (BottomRight, gBottomRight);
    float tRProduct = Vector2DotProduct (TopRight   , gTopRight);

    float dx = Fade(fx);
    float dy = Fade(fy);

    return Lerp(dx, Lerp(dy, bLProduct, tLProduct)
                  , Lerp(dy, bRProduct, tRProduct));    
}

                      // Hurst Exponent 
                      // For more info https://iquilezles.org/articles/fbm/
float               
fbm (float x, float y, float H)
{
float G = powf(2, -H);
float f = 1.0f; //frequency or wavelength
float a = 1.0f; //amplitude
float t = 0.0f; //total

float aSum = 0.0f;

    for(int i =0; i < 5; i++){
        // this is optional, recommended for perlin noise as at lattice 
        // points(meaning x and y are whole numbers), which can produce 
        // grid patterns. Here, we rotate the image as a whole 
        float rx = 0.8f * x - 0.6f * y;
        float ry = 0.6f * x + 0.8f * y;
        t += a * PerlinNoise2D((Vec2){rx * f,ry * f});
        f *= 1.99f; //2.0f is fine as well
        aSum += a/ 2;
        a *= G;
    }
    //at higher octaves in FBM, the values go beyound 1.0f, if the output is too "gray"/
    //low contrast, add a fraction of the amplitude to aSum.
    return t / aSum;
}

int 
main()
{
    srand(time(0));
    seed = rand();
    uint8_t *image = (uint8_t *) malloc (WIDTH * HEIGHT);
    uint8_t *image_rgb = (uint8_t *) malloc (WIDTH * HEIGHT * 3);
    char filename[25] = "frame0.png";
    if(!image)
        return 1;
    
    for(int frame = 0; frame < 300; frame++){
        float t = frame * 0.1f;
        sprintf(filename, "./frames/frame%d.png", frame);
    for(int y = 0; y< HEIGHT; y++){
        for(int x = 0; x < WIDTH; x++){
            //float value = PerlinNoise2D((Vec2){.x = x * 0.1f, .y = y * 0.1f});
           
            Vec2 v1 = {   fbm(x * 0.001f + (t *0.05f) , y * 0.001f, 0.4f)
                        , fbm(x * 0.001f + 5.2f + (t* 0.05f), y * 0.001f + 1.3f, 0.4f)
                    };

            Vec2 v2 = { fbm(x * 0.005f + (4.0f * v1.y) + (t * 0.075f), y * 0.005f + (4.0f * v1.x), 0.4f),
                        fbm(x * 0.005f + (4.0f * v1.x) + (t * 0.085f), y * 0.005f + (4.0f * v1.y), 0.6f)
                    };
            float value = fbm(x * 0.005f + (4.0f * v2.y) + t, y * 0.005f + (4.0f * v2.x), 0.3f);
            
            // more "turbulent" look:
            //uint8_t w = (uint16_t)roundf(fabsf(value) * 255) & 255;
            //image[y * WIDTH + x] = w;
            float w = value * 0.5f + 0.5f;              
            w = w < 0 ? 0 : (w > 1 ? 1 : w);
            image[y * WIDTH + x] = (uint8_t)(w * 255.0f + 0.5f);

            int c = ( y * WIDTH + x) * 3;

            float r = (2 * w * v1.x) ;
            r = r < 0 ? 0 : (r > 1 ? 1 : r);
            
            float g = (w * v1.y) ;
            g = g < 0 ? 0 : (g > 1 ? 1 : g);
            
            float b = value * 0.5f + 0.5f;
            b = b < 0 ? 0 : (b > 1 ? 1 : b);
            
            image_rgb[c + 2] = (uint8_t)(r * 255.0f + 0.5f);
            image_rgb[c + 1] = (uint8_t)(g * 255.0f + 0.5f);
            image_rgb[c + 0] = (uint8_t)(b * 255.0f + 0.5f);
        }
    }
    //write the image to disk using a stb_image_write.h wrapper.
    //https://github.com/nothings/stb/blob/master/stb_image_write.h
    //write_grayscale_png(filename, image, WIDTH, HEIGHT);
    write_rgb_png(filename, image_rgb, WIDTH, HEIGHT);

    }
    free(image);
    return 0;
};
