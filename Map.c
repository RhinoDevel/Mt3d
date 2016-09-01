
// MT, 2016aug23

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "Map.h"

void Map_print(struct Map const * const inOutObj, int const * const inPlayerX, int const * const inPlayerY)
{
    bool const player = (inPlayerX!=NULL)/*&&(inPlayerY!=NULL)*/;
    
    for(int row = 0, col = 0;row<inOutObj->height;++row)
    {
        unsigned char const * const rowPtr = inOutObj->cells+row*inOutObj->width;
        bool const playerRow = player&&(row==*inPlayerY);
        
        for(col = 0;col<inOutObj->width;++col)
        {
            char c = '?';
            
            if(playerRow&&(col==*inPlayerX))
            {
                c = 'p';
            }
            else
            {
                switch((enum CellType)rowPtr[col])
                {
                    case CellType_block_default:
                        c = 'X';
                        break;
                    case CellType_floor_default:
                        c = '.';
                        break;
                    case CellType_floor_exit:
                        c = 'E';
                        break;

                    default:
                        assert(false);
                        break;
                }
            }
            
            printf("%c", c);
        }
        printf("\n");
    }
}

void Map_set(struct Map * const inOutObj, int const inRow, int const inCol, enum CellType const inType)
{
    assert(inOutObj!=NULL);
    assert(inOutObj->cells!=NULL);
    assert(inOutObj->height>inRow);
    assert(inOutObj->width>inCol);
    assert(((int)inOutObj->posX!=inCol)||((int)inOutObj->posY!=inRow));
    
    ((unsigned char *)inOutObj->cells)[inRow*inOutObj->width+inCol] = (unsigned char)inType;
}

void Map_delete(struct Map * const inObj)
{
    assert(inObj!=NULL);
    
    assert(inObj->cells!=NULL);
    free((unsigned char *)(inObj->cells));
    
    free(inObj);
}
