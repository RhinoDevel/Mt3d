
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
    
    // Player's start position:
    //
    double const posX;
    double const posY;
    
    double const gamma; // Player's start direction of view (angle in radian).
    
    double const maxVisible; // Maximum visibility in cell length.
    double const maxDarkness; // Maximum darkness 
    
    unsigned char const * const cells;
};

void Map_print(struct Map const * const inOutObj, int const * const inPlayerX, int const * const inPlayerY);

void Map_set(struct Map * const inOutObj, int const inRow, int const inCol, enum CellType const inType);

void Map_delete(struct Map * const inObj);

#ifdef __cplusplus
}
#endif

#endif // MT_MAP
