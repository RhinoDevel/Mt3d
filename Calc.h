
// MT, 2016sep11

#ifndef MT_CALC
#define MT_CALC

#ifdef __cplusplus
extern "C" {
#endif

/** Return the length of triangle's side a for given parameters.
 * 
 * Given parameters are:
 * 
 * - Angle Gamma in radians (being the angle at point C opposite to side c).
 * - Length of the part of side c before the altitude of side c (left).
 * - Length of the part of side c after the altitude of side c (right).
 * 
 * Returns 0.0, if there is no positive result.
 * 
 * * May NOT support all kinds of triangles, function should work for triangles with angles Alpha and Beta having less than 90 degrees.
 */
double Calc_getTriangleSideA(double const inGammaRad, double const inCleftOfAltitudeC, double const inCrightOfAltitudeC);

#ifdef __cplusplus
}
#endif

#endif // MT_CALC
