
// MT, 2017feb08

#ifndef MT_3D_PARAMS
#define MT_3D_PARAMS

#ifdef __cplusplus
extern "C" {
#endif

/** Input parameters for Mt3d "object" creation [see Mt3d_create()].
 */
struct Mt3dParams
{
    int width;
    int height;
    double alpha;
    double beta;
    double theta;
    double h;
    int msPerUpdate;
};

#ifdef __cplusplus
}
#endif

#endif // MT_3D_PARAMS
