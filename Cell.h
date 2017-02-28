
// MT, 2017feb26

#ifndef MT_CELL
#define MT_CELL

#include "CellType.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Cell
{
    enum CellType type;
    double floor; // Floor height (from 0.0 to ...).
    double height; // Distance between floor and ceiling (from 0.0 to ...). Ceiling height = .floor + .height.
};

#ifdef __cplusplus
}
#endif

#endif // MT_CELL
