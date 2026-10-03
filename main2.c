/*
 * This version uses a gradients grid which uses 4-8 types of 2D Vectors. 
 * It is created once in the startup and used throughout so that the grid
 * values (cell for the current sample) remain consistent all the way.
 * 
 * For an infinite, non-repeating domain see the integer hash variation,
 * which also avoids wrapping the x and y values using % GRADIENTS_SIZE -1 MASK
 */

#include "image.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define GRADIENTS_SIZE 256
#define MASK (GRADIENTS_SIZE - 1)

#define WIDTH 500
#define HEIGHT 500

typedef struct {
  float x;
  float y;
} Vec2;

Vec2 gradients[GRADIENTS_SIZE * GRADIENTS_SIZE];

float 
Vector2DotProduct (Vec2 v1, Vec2 v2) 
{
    return v1.x * v2.x + v1.y * v2.y;
}

float 
Fade (float t) 
{ return ((6 * t - 15) * t + 10) * t * t * t; }

float
Lerp (float dx, float start, float end)
{
    return ( start + dx * (end - start));
}

Vec2 
getRandomGradient()
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

    return gradients[rand() % 8];
}

void
createGradientGrid()
{
    srand(time(NULL));

    for(int i =0; i < GRADIENTS_SIZE * GRADIENTS_SIZE; i++) {
        gradients[i] = getRandomGradient();
    }
}

float 
PerlinNoise2D (Vec2 v)
{
    float x = floorf (v.x);
    float y = floorf (v.y);

    float fx = v.x - x;
    float fy = v.y - y;
                                      //255
    int x0 = (int)x   & MASK, y0 = (int)y   & MASK;
    int x1 = (x0 + 1) & MASK, y1 = (y0 + 1) & MASK;
    
    Vec2 BottomLeft  = {fx       , fy       };
    Vec2 TopLeft     = {fx       , fy - 1.0f};
    Vec2 BottomRight = {fx - 1.0f, fy       };
    Vec2 TopRight    = {fx - 1.0f, fy - 1.0f};

    Vec2 gBottomLeft  = gradients[ y0 * GRADIENTS_SIZE  +  x0 ];
    Vec2 gTopLeft     = gradients[ y1 * GRADIENTS_SIZE  +  x0 ];
    Vec2 gBottomRight = gradients[ y0 * GRADIENTS_SIZE  +  x1 ];
    Vec2 gTopRight    = gradients[ y1 * GRADIENTS_SIZE  +  x1 ];

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

    for(int i = 0; i < 5; i++){
        // this is optional, recommended for perlin noise as at lattice 
        // points(meaning x and y are whole numbers), which can produce 
        // grid patterns. Here, we rotate the image as a whole 
        float rx = 0.8f * x - 0.6f * y;
        float ry = 0.6f * x + 0.8f * y;
        t += a * PerlinNoise2D( (Vec2){ .x = rx * f, .y = ry * f } );
        f *= 1.99f;
        aSum += a/2;
        a *= G;
    }
    //at higher octaves in FBM, the values go beyound 1.0f, if the output is too "gray"/
    //low contrast, add a fraction of the amplitude to aSum.
    return t / aSum;
}

int 
main(void)
{
    createGradientGrid();
    uint8_t *image = (uint8_t *) malloc (WIDTH * HEIGHT);
   
    if(!image)
        return 1; 
    
    for(int y = 0; y< HEIGHT; y++){
        for(int x = 0; x < WIDTH; x++){
            //float value = PerlinNoise2D((Vec2){.x = x * 0.1f, .y = y * 0.1f});

            Vec2 v1 = {   fbm(x * 0.001f, y * 0.001f, 0.4f)
                        , fbm(x * 0.001f + 5.2f, y * 0.001f + 1.3f, 0.4f)
                    };

            float value = fbm(x * 0.005f + (4.0f * v1.y), y * 0.005f + (4.0f * v1.x), 0.4f);
           
            // more "turbulent" look:
            //uint8_t w = (uint16_t)roundf(fabsf(value) * 255) & 255;
            //image[y * WIDTH + x] = w;
            float w = value * 0.5f + 0.5f;              
            w = w < 0 ? 0 : (w > 1 ? 1 : w);
            image[y * WIDTH + x] = (uint8_t)(w * 255.0f + 0.5f);
        }
    }
    //write the image to disk using a stb_image_write.h wrapper.
    //https://github.com/nothings/stb/blob/master/stb_image_write.h
    write_grayscale_png("noise.png", image, WIDTH,HEIGHT);

    free(image);
    return 0;
}
