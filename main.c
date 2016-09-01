
// MT, 2016aug22

//NDEBUG

#include <stdlib.h>
#include <assert.h>

#include "Mt3d.h"
#include "MapSample.h"
#include "Bmp.h"
#include "Sys.h"

static int const WIDTH = 320;
static int const HEIGHT = 200;
static int const ALPHA = 64;
static int const BETA = 40;
static double const H = 0.3;

int main()
{
    struct Mt3d * const o = Mt3d_create(WIDTH, HEIGHT, ALPHA, BETA, H);
    
    o->map = MapSample_create();
    o->pixels = malloc(WIDTH*HEIGHT*3*sizeof *(o->pixels));
    assert(o->pixels!=NULL);
    o->posX = o->map->posX;
    o->posY = o->map->posY;
    o->gamma = o->map->gamma;

    for(int row = 0, col = 0;row<HEIGHT;++row)
    {
        unsigned char * const rowPtr = o->pixels+row*WIDTH*3; 

        for(col = 0;col<3*WIDTH;col+=3)
        {
            rowPtr[col+0] = 0xFF; // Blue
            rowPtr[col+1] = 0x00; // Green
            rowPtr[col+2] = 0xFF; // Red
        }
    }
    
    bool exit = false;
    do
    {
        Mt3d_draw(o);
    
        int const playerX = (int)o->posX,
            playerY = (int)o->posY;
        Map_print(o->map, &playerX, &playerY);
        Bmp_write(WIDTH, HEIGHT, o->pixels, "out.bmp");
        
        char * const input = Sys_get_stdin();
        switch(input[0])
        {
            case 'a':
                o->gamma = o->gamma+23.0;
                break;
            case 's':
                break;
            case 'd':
                o->gamma = o->gamma-23.0;
                break;
            case 'w':
                break;
            
            default:
                exit = true;
                break;
        }
        if(o->gamma<0.0)
        {
            o->gamma = 360.0+o->gamma;
        }
        else
        {
            if(o->gamma>=360.0)
            {
                o->gamma -= 360.0;
            }
        }
    }while(!exit);
    
    Map_delete(o->map);
    o->map = NULL;
    free(o->pixels);
    o->pixels = NULL;
    Mt3d_delete(o);
    return EXIT_SUCCESS;
}
