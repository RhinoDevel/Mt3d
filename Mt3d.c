
// MT, 2016aug22

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "Deb.h"
#include "Mt3d.h"

static const double EQUAL_D_AND_E = 0.0;
static const double CEILING_HEIGHT = 1.0;

static int getFloorYAndFillDAndE(int const inHeight, int const inBeta, double const inH, double * const inOutD, double * const inOutE)
{
    int retVal = -1;
    double const fDelta = ((double)inBeta)/((double)(inHeight-1)),
        betaHalve = ((double)inBeta)/2.0,
        ceilingToEye = CEILING_HEIGHT-inH*CEILING_HEIGHT;
    
    for(int y = 0;y<inHeight;++y)
    {
        double const delta = fDelta*(double)y;
        
        if(delta<betaHalve)
        {
            inOutE[y] = ceilingToEye/sin(delta);
            inOutD[y] = inOutE[y]*cos(delta);
        }
        else
        {
            if(delta>betaHalve)
            {
                inOutE[y] = inH/sin(delta);
                inOutD[y] = inOutE[y]*cos(delta);
                
                if(retVal<0)
                {
                    retVal = y;
                }
            }
            else
            {
                inOutD[y] = EQUAL_D_AND_E;
                inOutE[y] = EQUAL_D_AND_E;
                
                Deb_line("Warning: Delta to use for y value %d exactly equals halve of beta %d!", y, inBeta)
            }
        }
    }
    
    assert(retVal>0);
    return retVal;
}

static void fillEpsilon(int const inWidth, int const inAlpha, double * const inOutEpsilon)
{
    double const fEpsilon = ((double)inAlpha)/((double)(inWidth-1));
    
    for(int x = 0;x<inWidth;++x)
    {
        inOutEpsilon[x] = fEpsilon*x;
    }
}

void Mt3d_delete(struct Mt3d * const inObj)
{
    assert(inObj!=NULL);
    
    assert(inObj->map==NULL); // No ownership of map.
    assert(inObj->pixels==NULL); // No ownership of pixels.
    
    assert(inObj->d!=NULL);
    free((double*)(inObj->d));
    
    assert(inObj->e!=NULL);
    free((double*)inObj->e);
    
    assert(inObj->epsilon!=NULL);
    free((double*)inObj->epsilon);
    
    free(inObj);
}

struct Mt3d * Mt3d_create(int const inWidth, int const inHeight, int const inAlpha, int const inBeta, double const inH)
{
    struct Mt3d * const retVal = malloc(sizeof *retVal);
    assert(retVal!=NULL);
    
    double * const d = malloc(inHeight*sizeof *d);
    assert(d!=NULL);
    double * const e = malloc(inHeight*sizeof *e);
    assert(e!=NULL);
    int const floorY = getFloorYAndFillDAndE(inHeight, inBeta, inH, d, e);
    
    double * const epsilon = malloc(inWidth*sizeof *epsilon);
    assert(epsilon!=NULL);
    fillEpsilon(inWidth, inAlpha, epsilon);
    
    struct Mt3d const buf = (struct Mt3d)
    {
        .width = inWidth,
        .height = inHeight,
        .alpha = inAlpha,
        .beta = inBeta,
        .h = inH,
            
        .d = d,
        .e = e,
        .floorY = floorY,
            
        .epsilon = epsilon,
            
        .posX = 0.0,
        .posY = 0.0,
        .gamma = 0.0,
        .map = NULL,
        .pixels = NULL
    };

    memcpy(retVal, &buf, sizeof *retVal);

    return retVal;
}
