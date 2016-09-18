
// MT, 2016sep11

#ifndef MT_CALC
#define MT_CALC

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CALC_PI_BY_2 (2.0*M_PI) // 360 degree in radian.
    
#define CALC_TO_RAD(x) (((x)*M_PI)/180.0) // Converts degree to radian.
#define CALC_TO_DEG(x) (((x)*180.0)/M_PI) // Converts radian to degree.
#define CALC_ANGLE_TO_POS(x) ((x)<0.0?CALC_PI_BY_2+(x):(x)>=CALC_PI_BY_2?(x)-CALC_PI_BY_2:(x)) // Given angle, or positive equivalent, if negative angle given.
    
/** Return the length of triangle's side a for given parameters.
 * 
 * Given parameters are:
 * 
 * - Angle Gamma in radian (being the angle at point C opposite to side c).
 * - Length of the part of side c before the altitude of side c (left).
 * - Length of the part of side c after the altitude of side c (right).
 * 
 * Returns 0.0, if there is no positive result.
 * 
 * * May NOT support all kinds of triangles, function should work for triangles with angles Alpha and Beta having less than 90 degree.
 */
double Calc_getTriangleSideA(double const inGammaRad, double const inCleftOfAltitudeC, double const inCrightOfAltitudeC);

#ifdef __cplusplus
}
#endif

#endif // MT_CALC
