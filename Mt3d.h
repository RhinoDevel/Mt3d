
// MT, 2016aug22

#ifndef MT_3D
#define MT_3D

#include <stdbool.h>
#include <stdint.h>

#include "Dim.h"
#include "Map.h"
#include "HitType.h"
#include "Mt3dParams.h"
#include "Mt3dConstants.h"
#include "Mt3dVariables.h"

#ifdef __cplusplus
extern "C" {
#endif

/*

alpha
beta
gamma
delta
epsilon
zeta
eta
theta
iota

*/

struct Mt3d
{
    // ************
    // *** DATA ***
    // ************

    struct Bmp * bmp[CellType_COUNT];

    // ***********************
    // *** INPUT CONSTANTS ***
    // ***********************

    struct Mt3dConstants const constants;

    // ***********************
    // *** INPUT VARIABLES ***
    // ***********************
    //
    struct Mt3dVariables variables;

    // *****************************
    // *** CALCULATED FROM INPUT ***
    // *****************************

    double * const iota; // One angle (radian) for each (x,y) pixel coordinate. Angle is between "d" (adjacent) and "e" (hypotenuse).
    enum HitType * const hitType; // e reaches floor, ceiling, or none.
    double * const eta; // One value for each (x/y) pixel coordinate (in radian).

    // **********************
    // *** CURRENT VALUES ***
    // **********************

    bool doFill;
    uint64_t updateCount; // Elapsed steps of game time (updateCount * msPerUpdate = elapsed milliseconds of game time).

    // Sub-pixel position of player:
    //
    double posX;
    double posY;

    double gamma; // Angle (radian) telling the player's view direction.

    struct Map * map; // Does NOT take ownership.
    uint32_t * pixels; // Does NOT take ownership.
};

bool Mt3d_ang_leftOrRight(struct Mt3d * const inOutObj, bool inLeft);
bool Mt3d_pos_forwardOrBackward(struct Mt3d * const inOutObj, bool inForward);
bool Mt3d_pos_leftOrRight(struct Mt3d * const inOutObj, bool inLeft);
void Mt3d_update(struct Mt3d * const inOutObj);
void Mt3d_draw(struct Mt3d * const inOutObj);
void Mt3d_delete(struct Mt3d * const inObj);
void Mt3d_setVariables(struct Mt3dVariables const * const inVariables, struct Mt3d * const inOutObj);
struct Mt3d * Mt3d_create(struct Mt3dParams const * const inParams);

#ifdef __cplusplus
}
#endif

#endif // MT_3D
