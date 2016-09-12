
// MT, 2016aug22

#ifndef MT_3D
#define MT_3D

#include "Map.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Mt3d
{
    // ***********************
    // *** INPUT CONSTANTS ***
    // ***********************
    
    // Pixel resolution:
    //
    int const width;
    int const height;
    
    int /*const*/ alpha; // Horizontal range of view (degrees).
    double /*const*/ beta; // Vertical range of view (degrees).
    
    double /*const*/ h; // Height of players eye as fraction of ceiling height (which is 1.0).
    
    // ****************************
    // *** CALCULATED CONSTANTS ***
    // ****************************
    
    double /*const*/ * const d; // One d value for each y value.
    double /*const*/ * const e; // One e value for each y value.
    int /*const*/ floorY; // First y value that has "an e" reaching the floor and not the ceiling.
    
    double /*const*/ * const eta; // One epsilon angle (degrees) for each x value subtracted from halve of alpha angle (degrees).
    
    // **********************
    // *** CURRENT VALUES ***
    // **********************
    
    // Sub-pixel position of player:
    //
    double posX;
    double posY;
    
    double gamma; // Angle (degrees) telling the player's view direction.
    
    struct Map * map; // Does NOT take ownership.
    unsigned char * pixels; // Does NOT take ownership.
};

void Mt3d_draw(struct Mt3d * const inObj);
void Mt3d_delete(struct Mt3d * const inObj);
void Mt3d_update(int const inAlpha, double const inBeta, double const inH, struct Mt3d * const inOutObj);
struct Mt3d * Mt3d_create(int const inWidth, int const inHeight, int const inAlpha, double const inBeta, double const inH);

#ifdef __cplusplus
}
#endif

#endif // MT_3D
