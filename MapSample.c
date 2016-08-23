
// MT, 2016aug23

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "CellType.h"
#include "MapSample.h"

static int const WIDTH = 48;
static int const HEIGHT = 32;

static void fillCells(struct Map * const inOutMap)
{
    unsigned char * const cells = (unsigned char *)(inOutMap->cells);
    
    for(int row = 0, col = 0;row<HEIGHT;++row)
    {
        unsigned char * const rowPtr = cells+row*HEIGHT;
        
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
    
    Map_set(inOutMap, HEIGHT/2, WIDTH/2, CellType_block_default);
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
        
        .cells = cells
    };
    
    memcpy(retVal, &buf, sizeof *retVal);

    fillCells(retVal);
    
    return retVal;
}
