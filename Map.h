
// MT, 2016aug23

#ifndef MT_MAP
#define MT_MAP

#include "CellType.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Map
{
    int const width;
    int const height;
    
    unsigned char const * const cells;
};

void Map_set(struct Map * const inOutObj, int const inRow, int const inCol, enum CellType const inType);

void Map_delete(struct Map * const inObj);

#ifdef __cplusplus
}
#endif

#endif // MT_MAP
