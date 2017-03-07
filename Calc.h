
// MT, 2016sep11

#ifndef MT_CALC
#define MT_CALC

#include <math.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif //M_PI

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif //M_PI_2

static double const Calc_PiMul2 = 2.0*M_PI; // 360 degree in radian.
static double const Calc_PiMul1_5 = 1.5*M_PI; // 270 degree in radian.

#define CALC_TO_RAD(x) (((x)*M_PI)/180.0) // Converts degree to radian.
#define CALC_TO_DEG(x) (((x)*180.0)/M_PI) // Converts radian to degree.
#define CALC_ANGLE_TO_POS(x) ((x)<0.0?Calc_PiMul2+(x):(x)>=Calc_PiMul2?(x)-Calc_PiMul2:(x)) // Converts given angle to be between 0 and 2xPI.

#define CALC_SIGN_FROM_DOUBLE(x) ((int)(0.0<(x))-(int)((x)<0.0))
#define CALC_DOUBLE_TO_INT_ROUND(x) ((int)((x)+0.5*(double)CALC_SIGN_FROM_DOUBLE(x)))

#define CALC_CARTESIAN_Y(y, h) ((h)-1.0-(y)) // Converts y coordinate of a pixel (Y starts at top-left) given as double to Cartesion coordinate (Y starts at bottom-left). Double height of pixel's bitmap is also given.
#define CALC_INT_TO_DOUBLE_CARTESIAN_Y(y, h) ((double)((h)-1)-(double)(y)) // Converts y coordinate of a pixel (Y starts at top-left) given as integer to Cartesion coordinate (Y starts at bottom-left). Integer height of pixel's bitmap is also given.

/** Return sector of Cartesian coordinate system from 0 to 3 instead of I, II, III, IV (counter-clockwise).
 *
 * - Assumes: 0 rad <= inAngle < 2*PI rad.
 */
int Calc_getZeroSector(double const inAngle);

/** Rotate given point (v, w) in Cartesian coordinates (Y starts at bottom-left and not at top-left)
 *  counter-clockwise (CCW) by angle given in radian
 *  and fill given output parameters x and y, representing rotation result point (x, y).
 */
void Calc_fillRotated(double const inV, double const inW, double const inAngle, double * const inOutX, double * const inOutY);

/** Create and return Sine lookup-table (from 0 to 360 degree).
 *
 * - Given length corresponds to angle 360.0 degree.
 * - Values stored in LUT are from 0 to 65535 (where 65535+1 = 65536 corresponds to 0 at 360 degree).
 * - Caller takes ownership.
 */
uint16_t* Calc_createSinLut(size_t const inLen);

double Calc_sin(uint16_t const * const inLut, size_t const inLen, double const inRad);
double Calc_cos(uint16_t const * const inSinLut, size_t const inLen, double const inRad);
double Calc_tan(uint16_t const * const inSinLut, size_t const inLen, double const inRad);

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

/** ...
 *
 * * Returns -1.0 on error (no precise enough result found with allowed step count).
 * * Source: http://forums.getpebble.com/discussion/5792/sqrt-function
 *
 */
double Calc_getSquareRoot(double const inNum);

#ifdef __cplusplus
}
#endif

#endif // MT_CALC
