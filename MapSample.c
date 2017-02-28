
// MT, 2016aug23

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "CellType.h"
#include "MapSample.h"
#include "Calc.h"

static int const WIDTH = 32;
static int const HEIGHT = 64;

static double const POS_X = 23.359270864847975;
static double const POS_Y = 2.07568468132119;

static double const GAMMA = CALC_TO_RAD(270.0);

static double const MAX_VISIBLE = 32.0;
static double const MAX_DARKNESS = 1.0;

static void fillCells(struct Map * const inOutMap)
{
    for(int row = 0, col = 0;row<HEIGHT;++row)
    {
        struct Cell * const rowPtr = inOutMap->cells+row*WIDTH;
        
        for(col = 0;col<WIDTH;++col)
        {
//            if(col==20&&row==1)
//            {
//                rowPtr[col].floor = 0.25;
//                rowPtr[col].height = 0.5;
//                rowPtr[col].type = CellType_floor_default;
//                continue;
//            }
            
            rowPtr[col].floor = 0.0;
            rowPtr[col].height = 1.0;
            
            if((col==0)||(col==(WIDTH-1))||(row==0)||(row==(HEIGHT-1)))
            {
                rowPtr[col].type = CellType_block_default;
            }
            else
            {
                if(col%4==0&&row%8==0)
                {
                    rowPtr[col].type = CellType_block_default;
                }
                else
                {
                    rowPtr[col].type = CellType_floor_default;
                }
            }
        }
    }

    Map_set(inOutMap, HEIGHT/2-1, WIDTH/2-1, CellType_floor_exit);
}

struct Map * MapSample_create()
{
    struct Map * const retVal = malloc(sizeof *retVal);
    assert(retVal!=NULL);
    
    struct Cell * const cells = malloc(WIDTH*HEIGHT*sizeof *cells);
    assert(cells!=NULL);
    
    struct Map const buf = (struct Map)
    {
        .width = WIDTH,
        .height = HEIGHT,
        
        .posX = POS_X,
        .posY = POS_Y,
            
        .gamma = GAMMA,
            
        .maxVisible = MAX_VISIBLE,
        .maxDarkness = MAX_DARKNESS,
            
        .cells = cells
    };
    
    memcpy(retVal, &buf, sizeof *retVal);

    fillCells(retVal);
    
    return retVal;
}
