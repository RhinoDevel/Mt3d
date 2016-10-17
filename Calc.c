
// MT, 2016sep11

#include <stdlib.h>
#include <assert.h>

#include "Calc.h"

static uint16_t const maxScaledSine = UINT16_MAX;
static double const maxScaledSineD = (double)UINT16_MAX;

int Calc_getZeroSector(double const inAngle)
{
    assert(inAngle>=0.0&&inAngle<Calc_PiMul2);
    
    return (int)CALC_TO_DEG(inAngle)/90; // (integer division)
}

void Calc_fillDeltas(double const inAngle, double const inHypotenuse, double * const inOutDeltaX, double * const inOutDeltaY)
{
    assert(inAngle>=0.0 && inAngle<Calc_PiMul2);
    assert(inOutDeltaX!=NULL && inOutDeltaY!=NULL);
    
    static double const xFactor[] = { 1.0, -1.0, -1.0, 1.0 }, // One entry for each sector as index to get correct sign.
        yFactor[] = { 1.0, 1.0, -1.0, -1.0 }; // One entry for each sector as index to get correct sign.
    int const zeroSector = Calc_getZeroSector(inAngle);
    double const angle = inAngle-zeroSector*M_PI_2,
        opposite = inHypotenuse*sin(angle),
        adjacent = sqrt(inHypotenuse*inHypotenuse-opposite*opposite),
        zeroTwo = (double)((3-zeroSector)%2), // 1.0 for sectors 0 and 2, zero otherwise.
        oneThree = (double)(zeroSector%2); // 1.0 for sectors 1 and 3, zero otherwise.
    
    *inOutDeltaX = xFactor[zeroSector]*(zeroTwo*adjacent+oneThree*opposite);
    *inOutDeltaY = yFactor[zeroSector]*(zeroTwo*opposite+oneThree*adjacent);
}

uint16_t* Calc_createFirstQuadrantSinLut(size_t const inLen)
{
    /*static*/ double const sineScaling = (double)(maxScaledSine+1); // Corresponds to 1.0.
               
    uint16_t * const retVal = malloc(inLen*sizeof *retVal);
    double const angleScaling = M_PI_2/inLen; // inLen corresponds to 90 degree.
    
    assert(retVal!=NULL);
    
    for(int i = 0;i<inLen;++i)
    {
        double const angle = angleScaling*(double)i,
            sine = sin(angle),
            scaledSine = sineScaling*sine;
        
        assert(angle>=0.0 && angle<M_PI_2);
        assert(sine>=0.0 && sine<1.0);
        assert(scaledSine>=0.0/*&& scaledSine<=(double)maxScaledSine*/);
        
        retVal[i] = (uint16_t)scaledSine; // Truncates
        
        assert(retVal[i]<=maxScaledSine);
    }
    
    return retVal;
}

double Calc_sin(uint16_t const * const inLut, size_t const inLen, double const inRad)
{
    assert(inLut!=NULL);
    assert(inLen>0);
    
    assert(inRad>=0.0 && inRad<M_PI_2); // MT_TODO: TEST: Currently not implemented!
    
    return ((double)(inLut[(int)((inLen*inRad)/M_PI_2)]))/maxScaledSineD;
}

double Calc_getTriangleSideA(double const inGammaRad, double const inCleftOfAltitudeC, double const inCrightOfAltitudeC)
{
    double const c = inCleftOfAltitudeC+inCrightOfAltitudeC, // Full length of side c.
        
        // Using law of cosines (for the actual triangle)
        // and Pythagorean theorem (for the two right triangles at left and right of side c's altitude):
        //
        d = inCleftOfAltitudeC*inCleftOfAltitudeC-inCrightOfAltitudeC*inCrightOfAltitudeC,
        e = c*c-d,
        cosGammaRad = cos(inGammaRad),
        f = cosGammaRad*cosGammaRad,
        g = 2.0-2.0*f,
        h = (e+d*f)/g,
        i = sqrt(h*h-(e*e)/(2.0*g)),
        //
        // Completing the square leads to two results: 
        //
        a1Sqr = h+i,
        a2Sqr = h-i;
    
    if(a1Sqr>=0.0)
    {
        double const a1 = sqrt(a1Sqr);
        
        if(a2Sqr>=0)
        {
            // Calculate triangle's side c's lengths by using the results
            // and also calculate deviations from actually known length of side c:
            //
            double const a2 = sqrt(a2Sqr),
                a1c = sqrt(2.0*a1Sqr+d-2.0*a1*cos(inGammaRad)*sqrt(a1Sqr+d)),
                a2c = sqrt(2.0*a2Sqr+d-2.0*a2*cos(inGammaRad)*sqrt(a2Sqr+d)),
                deviation1 = fabs(a1c-c),
                deviation2 = fabs(a2c-c);
            
            if(deviation2<deviation1)
            {
                return a2; // Return second result, because it deviates less from actual length of side c.
            }
            //
            // Otherwise: Return first result, because it deviates less from actual length of side c.
        }
        //
        // Otherwise: Return the first result, as it is the only positive one.
        
        return a1;
    }
    if(a2Sqr>=0.0)
    {
        return sqrt(a2Sqr); // Return second result, as it is the only positive one.
    }
    return 0.0; // No positive result (must not happen for supported triangles).
}

/** ...
 * 
 * * Returns -1.0 on error (no precise enough result found with allowed step count).
 * * Source: http://forums.getpebble.com/discussion/5792/sqrt-function
 * 
 */
double Calc_getSquareRoot(double const inNum)
{
    static unsigned int const maxSteps = 40;
    static double const maxErr = 0.001;
    double retVal = -1.0,
        result = inNum;
    unsigned int step = 0;

    for(;step<maxSteps;++step)
    {
        result = 0.5*(result+(inNum/result));
        if((result*result-inNum)<=maxErr)
        {
            retVal = result;
            break; // Precise enough result.
        }
    }

    return retVal;
}
