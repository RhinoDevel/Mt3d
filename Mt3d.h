
// MT, 2016aug22

#ifndef MT_3D
#define MT_3D

#include <stdbool.h>
#include <stdint.h>

#include "Map.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Mt3d
{
    // ***********************
    // *** INPUT CONSTANTS ***
    // ***********************

    int const msPerUpdate;// Milliseconds of game time per update call.
    uint64_t updateCount; // Elapsed steps of game time (updateCount * msPerUpdate = elapsed milliseconds of game time).

    // Pixel resolution:
    //
    int const width;
    int const height;

    double /*const*/ alpha; // Horizontal range of view (radian). Better be below 180 degree.
    double /*const*/ beta; // Vertical range of view (radian). Better be below 180 degree.
    double /*const*/ theta; // CCW rotation of Z-axis.

    double /*const*/ h; // Height of players eye as fraction of ceiling height (ceiling height is measured in cell lengths).

    // ****************************
    // *** CALCULATED CONSTANTS ***
    // ****************************

    double /*const*/ * const d; // One d value for each (x,y) pixel coordinate (in cell lengths).
    double /*const*/ * const e; // One e value for each (x,y) pixel coordinate (in cell lengths).
    bool doFill;
    bool /*const*/ * const hitsFloor; // e reaches the floor and not the ceiling, if true.

    double /*const*/ * const eta; // One value for each (x/y) pixel coordinate (in radian).

    // **********************
    // *** CURRENT VALUES ***
    // **********************

    // Sub-pixel position of player:
    //
    double posX;
    double posY;

    double gamma; // Angle (radian) telling the player's view direction.

    unsigned char * bmpPix[CellType_COUNT];
    int bmpW[CellType_COUNT];
    int bmpH[CellType_COUNT];

    struct Map * map; // Does NOT take ownership.
    unsigned char * pixels; // Does NOT take ownership.
};

bool Mt3d_ang_leftOrRight(struct Mt3d * const inOutObj, bool inLeft);
bool Mt3d_pos_forwardOrBackward(struct Mt3d * const inOutObj, bool inForward);
bool Mt3d_pos_leftOrRight(struct Mt3d * const inOutObj, bool inLeft);
void Mt3d_update(struct Mt3d * const inOutObj);
void Mt3d_draw(struct Mt3d * const inOutObj);
void Mt3d_delete(struct Mt3d * const inObj);
void Mt3d_setValues(double const inAlpha, double const inBeta, double const inTheta, double const inH, struct Mt3d * const inOutObj);
struct Mt3d * Mt3d_create(int const inWidth, int const inHeight, double const inAlpha, double const inBeta, double const inTheta, double const inH, int const inMsPerUpdate);

#ifdef __cplusplus
}
#endif

#endif // MT_3D
