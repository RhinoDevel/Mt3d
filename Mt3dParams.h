
// MT, 2017feb08

#ifndef MT_3D_PARAMS
#define MT_3D_PARAMS

#include "Mt3dConstants.h"
#include "Mt3dVariables.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Input parameters for Mt3d "object" creation [see Mt3d_create()].
 */
struct Mt3dParams
{
    struct Mt3dConstants constants;
    struct Mt3dVariables variables;
};

#ifdef __cplusplus
}
#endif

#endif // MT_3D_PARAMS
