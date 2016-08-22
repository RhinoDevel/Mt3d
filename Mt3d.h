
// MT, 2016aug22

#ifndef MT_3D
#define MT_3D

// Add includes here.

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
    
    int const alpha; // Horizontal range of view (degrees).
    int const beta; // Vertical range of view (degrees).
    
    double const h; // Height of players eye as fraction of ceiling height (which is 1.0).
    
    // ****************************
    // *** CALCULATED CONSTANTS ***
    // ****************************
    
    double const * const d; // One d value for each y value.
    double const * const e; // One e value for each y value.
    
    double const * const epsilon; // One epsilon angle (degrees) for each x value subtracted from halve of alpha angle (degrees).
    
    // **********************
    // *** CURRENT VALUES ***
    // **********************
    
    // Sub-pixel position of player:
    //
    double posX;
    double posY;
    
    double gamma; // Angle (degrees) telling the player's view direction.
};

void Mt3d_delete(struct Mt3d * const inObj);
struct Mt3d * Mt3d_create(int const inWidth, int const inHeight, int const inAlpha, int const inBeta, double const inH);

#ifdef __cplusplus
}
#endif

#endif // MT_3D
