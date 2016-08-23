
// MT, 2016aug23

#include <assert.h>
#include <stdlib.h>

#include "Map.h"

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
