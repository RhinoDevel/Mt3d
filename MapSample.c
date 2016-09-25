
// MT, 2016aug23

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "CellType.h"
#include "MapSample.h"
#include "Calc.h"

static int const WIDTH = 11;
static int const HEIGHT = 9;

static double const POS_X = 4.123;
static double const POS_Y = 5.321;

static double const GAMMA = CALC_TO_RAD(90.0);

static void fillCells(struct Map * const inOutMap)
{
    unsigned char * const cells = (unsigned char *)(inOutMap->cells);
    
    for(int row = 0, col = 0;row<HEIGHT;++row)
    {
        unsigned char * const rowPtr = cells+row*WIDTH;
        
        for(col = 0;col<WIDTH;++col)
        {
            if((col==0)||(col==(WIDTH-1))||(row==0)||(row==(HEIGHT-1)))
            {
                rowPtr[col] = CellType_block_default;
            }
            else
            {
                rowPtr[col] = CellType_floor_default;
            }
        }
    }
    
    Map_set(inOutMap, HEIGHT/2+1, WIDTH/2+1, CellType_block_default);
    Map_set(inOutMap, HEIGHT/2-1, WIDTH/2-1, CellType_floor_exit);
}

struct Map * MapSample_create()
{
    struct Map * const retVal = malloc(sizeof *retVal);
    assert(retVal!=NULL);
    
    unsigned char * const cells = malloc(WIDTH*HEIGHT*sizeof *cells);
    assert(cells!=NULL);
    
    struct Map const buf = (struct Map)
    {
        .width = WIDTH,
        .height = HEIGHT,
        
        .posX = POS_X,
        .posY = POS_Y,
            
        .gamma = GAMMA,
            
        .cells = cells
    };
    
    memcpy(retVal, &buf, sizeof *retVal);

    fillCells(retVal);
    
    return retVal;
}
