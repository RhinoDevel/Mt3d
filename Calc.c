
// MT, 2016sep11

#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include "Calc.h"

int Calc_getZeroSector(double const inAngle)
{
    assert(inAngle>=0.0&&inAngle<Calc_PiMul2);
    
    return (int)CALC_TO_DEG(inAngle)/90; // (integer division)
}

void Calc_fillDeltas(double const inAngle, double const inHypotenuse, double * const inOutDeltaX, double * const inOutDeltaY)
{
    assert(inOutDeltaX!=NULL && inOutDeltaY!=NULL);
    
    static double const xFactor[] = { 1.0, -1.0, -1.0, 1.0 }, // One entry for each sector as index to get correct sign.
        yFactor[] = { 1.0, 1.0, -1.0, -1.0 }; // One entry for each sector as index to get correct sign.
    int const zeroSector = Calc_getZeroSector(inAngle);
    double const angle = inAngle-zeroSector*Calc_PiMul0_5,
        opposite = inHypotenuse*sin(angle),
        adjacent = sqrt(inHypotenuse*inHypotenuse-opposite*opposite),
        zeroTwo = (double)((3-zeroSector)%2), // 1.0 for sectors 0 and 2, zero otherwise.
        oneThree = (double)(zeroSector%2); // 1.0 for sectors 1 and 3, zero otherwise.
    
    *inOutDeltaX = xFactor[zeroSector]*(zeroTwo*adjacent+oneThree*opposite);
    *inOutDeltaY = yFactor[zeroSector]*(zeroTwo*opposite+oneThree*adjacent);
}

double Calc_getTriangleSideA(double const inGammaRad, double const inCleftOfAltitudeC, double const inCrightOfAltitudeC)
{
    double const c = inCleftOfAltitudeC+inCrightOfAltitudeC, // Full length of side c.
        
        // Using law of cosines (for the actual triangle)
        // and Pythagorean theorem (for the two right triangles at left and right of side c's altitude):
        //
        d = pow(inCleftOfAltitudeC, 2.0)-pow(inCrightOfAltitudeC, 2.0),
        e = pow(c, 2.0)-d,
        f = pow(cos(inGammaRad), 2.0),
        g = 2.0-2.0*f,
        h = (e+d*f)/g,
        i = sqrt(pow(h, 2.0)-pow(e, 2.0)/(2.0*g)),
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
