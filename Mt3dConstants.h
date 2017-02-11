
// MT, 2017feb11

#ifndef MT_3D_CONSTANTS
#define MT_3D_CONSTANTS

#include "Dim.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Constant input parameters for Mt3d "object" creation [see Mt3dParams].
 */
struct Mt3dConstants
{
    int msPerUpdate; // Milliseconds of game time per update call.
    struct Dim res; // Pixel resolution.
};

#ifdef __cplusplus
}
#endif

#endif // MT_3D_CONSTANTS
