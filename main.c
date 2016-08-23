
// MT, 2016aug22

//NDEBUG

#include <stdlib.h>
#include <assert.h>

#include "Mt3d.h"
#include "MapSample.h"
#include "Bmp.h"

static int const WIDTH = 300;
static int const HEIGHT = 200;
static int const ALPHA = 90;
static int const BETA = 60;
static double const H = 0.5;

int main()
{
    struct Mt3d * const o = Mt3d_create(WIDTH, HEIGHT, ALPHA, BETA, H);
    
    o->map = MapSample_create();
    o->pixels = malloc(WIDTH*HEIGHT*3*sizeof *(o->pixels));
    assert(o->pixels!=NULL);
    o->posX = o->map->posX;
    o->posY = o->map->posY;
    o->gamma = o->map->gamma;
    
//    {
//        for(int row = 0, col = 0;row<HEIGHT;++row)
//        {
//            unsigned char * const rowPtr = o->pixels+row*WIDTH*3; 
//
//            for(col = 0;col<3*WIDTH;col+=3)
//            {
//                rowPtr[col+0] = 0x00; // Blue
//                rowPtr[col+1] = 0x00; // Green
//                rowPtr[col+2] = 0xFF; // Red
//            }
//        }
//    }	
    
    Bmp_write(WIDTH, HEIGHT, o->pixels, "out.bmp");
    
    Map_delete(o->map);
    o->map = NULL;
    free(o->pixels);
    o->pixels = NULL;
    Mt3d_delete(o);
    return EXIT_SUCCESS;
}
