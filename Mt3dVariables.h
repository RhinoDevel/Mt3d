
// MT, 2017feb11

#ifndef MT_3D_VARIABLES
#define MT_3D_VARIABLES

#ifdef __cplusplus
extern "C" {
#endif

/** Input parameters for Mt3d "object" update [see Mt3d_setVariables()].
 */
struct Mt3dVariables
{
    double alpha; // Horizontal range of view (radian). Better be below 180 degree.
    double beta; // Vertical range of view (radian). Better be below 180 degree.
    double theta; // CCW rotation of Z-axis.
    double playerEyeHeight; // Height of players eye in cell lengths measured from cell floor height.
};

#ifdef __cplusplus
}
#endif

#endif // MT_3D_VARIABLES
