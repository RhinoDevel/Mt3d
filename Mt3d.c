
// MT, 2016aug22

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>

#include "Deb.h"
#include "Mt3d.h"
#include "Sys.h"
#include "Calc.h"

//#define MT_DOUBLE_TO_INT_ROUND(x) ((int)(((x)<0.0)?((x)-0.5):((x)+0.5)))

static const double EQUAL_D_AND_E = 0.0;
static const double CEILING_HEIGHT = 1.0;

//static int getFloorYAndFillDAndE(int const inHeight, int const inBeta, double const inH, double * const inOutD, double * const inOutE)
//{
//    int retVal = -1;
//    double const fDelta = ((double)inBeta)/((double)(inHeight-1)),
//        betaHalve = ((double)inBeta)/2.0,
//        ceilingToEye = CEILING_HEIGHT-inH*CEILING_HEIGHT;
//    
//    assert((inBeta>=0.0)&&(inBeta<360.0));
//    
//    for(int y = 0;y<inHeight;++y)
//    {
//        double const delta = fDelta*(double)y;
//        
//        assert((delta>=0.0)&&(delta<360.0));
//        
//        if(delta<betaHalve)
//        {
//            inOutE[y] = ceilingToEye/sin((betaHalve-delta)*M_PI/180.0);
//            inOutD[y] = inOutE[y]*cos((betaHalve-delta)*M_PI/180.0);
//        }
//        else
//        {
//            if(delta>betaHalve)
//            {
//                inOutE[y] = inH/sin((delta-betaHalve)*M_PI/180.0);
//                inOutD[y] = inOutE[y]*cos((delta-betaHalve)*M_PI/180.0);
//                
//                if(retVal<0)
//                {
//                    retVal = y;
//                }
//            }
//            else
//            {
//                inOutD[y] = EQUAL_D_AND_E;
//                inOutE[y] = EQUAL_D_AND_E;
//                
//                Deb_line("Warning: Delta to use for y value %d exactly equals halve of beta %d!", y, inBeta)
//            }
//        }
//        
//        Deb_line("delta = %f, inOutE[%d] = %f, inOutD[%d] = %f.", delta, y, inOutE[y], y, inOutD[y])
//    }
//    
//    assert(retVal>0);
//    return retVal;
//}
//
static int getFloorYAndFillDAndE(int const inHeight, int const inBeta, double const inH, double * const inOutD, double * const inOutE)
{
    int retVal = -1;
    
    // No support for anything else implemented, yet (and probably never needed):
    //
    assert(CEILING_HEIGHT==1.0);
    
    assert((inBeta>=0.0)&&(inBeta<360.0));
    assert(inH>0.0 && inH<1.0);
    
    double const ceilingToEye = CEILING_HEIGHT-inH*CEILING_HEIGHT,
        lastPos = (double)(inHeight-1),
        bottomOpposite = lastPos*inH,
        topOpposite = lastPos-bottomOpposite,
        topHypotenuse = Calc_getTriangleSideA(inBeta*M_PI/180.0, bottomOpposite, topOpposite),
        betaTop = asin(topOpposite/topHypotenuse),
        a = topOpposite/tan(betaTop);
     
    for(int y = 0;y<inHeight;++y)
    {
        double const dY = (double)y;
        double delta = 0.0;
        
        if(dY<topOpposite)
        {
            delta = betaTop-atan((topOpposite-dY)/a);
        }
        else
        {
            assert(dY<=lastPos);
            delta = betaTop+atan((dY-topOpposite)/a);
        }
        
        if(delta<betaTop)
        {
            assert(dY<topOpposite);
            inOutE[y] = ceilingToEye/sin(betaTop-delta);
            inOutD[y] = inOutE[y]*cos(betaTop-delta);
        }
        else
        {
            if(delta>betaTop)
            {
                assert(dY>topOpposite);
                inOutE[y] = inH/sin(delta-betaTop);
                inOutD[y] = inOutE[y]*cos(delta-betaTop);
                
                if(retVal<0)
                {
                    retVal = y;
                }
            }
            else
            {
                assert(dY==topOpposite);
                inOutD[y] = EQUAL_D_AND_E;
                inOutE[y] = EQUAL_D_AND_E;
                
                Deb_line("Warning: Delta %f to use for y value %d exactly equals top part %f of beta %d!", delta*180.0/M_PI, y, betaTop*180.0/M_PI, inBeta)
            }
        }
        
        Deb_line("delta = %f, inOutE[%d] = %f, inOutD[%d] = %f.", delta*180.0/M_PI, y, inOutE[y], y, inOutD[y])
    }
    
    assert(retVal>0);
    return retVal;
}

static void fillEta(int const inWidth, int const inAlpha, double * const inOutEta)
{
    double const fEpsilon = ((double)inAlpha)/((double)(inWidth-1)),
        alphaHalve = ((double)inAlpha)/2.0;
    
    assert((inAlpha>=0.0)&&(inAlpha<360.0));
    
    for(int x = 0;x<inWidth;++x)
    {
        double const epsilon = fEpsilon*x; // Epsilon is the angle from pixel 0 = 0 degrees to pixel inWidth-1 = inAlpha degrees. 
        
        inOutEta[x] = alphaHalve-epsilon; // Eta is a pre-calculated angle to be used with player view angle, later.
        
        // Not necessary, here:
        //
//        if(inOutEta[x]<0.0)
//        {
//            inOutEta[x] = 360.0+inOutEta[x];
//        }
//        else
//        {
//            if(inOutEta[x]>=360.0)
//            {
//                inOutEta[x] -= 360.0;
//            }
//        }
//        assert((inOutEta[x]>=0)&&(inOutEta[x]<360.0));
        
        //Deb_line("epsilon = %f, inOutEta[%d] = %f.", epsilon, x, inOutEta[x])
    }
}

void Mt3d_draw(struct Mt3d * const inObj)
{
    int x = 0;
    double * const zeta = malloc(inObj->width*sizeof *zeta);            // MT_TODO: TEST: Move to Mt3d object creation/deletion!
    unsigned char * const sector = malloc(inObj->width*sizeof *sector); //
    assert(zeta!=NULL);
    assert(sector!=NULL);
    
    for(x = 0;x<inObj->width;++x)
    {
        zeta[x] = inObj->eta[x]+inObj->gamma;
        if(zeta[x]<0.0)
        {
            zeta[x] = 360.0+zeta[x];
        }
        else
        {
            if(zeta[x]>=360.0)
            {
                zeta[x] -= 360.0;
            }
        }
        assert((zeta[x]>=0.0)&&(zeta[x]<360.0));
        
        sector[x] = (unsigned char)(((int)zeta[x])/90+1);
        assert((sector[x]>=1)&&(sector[x]<=4));
        
        //Deb_line("zeta[%d] = %f | sector[%d] = %d", x, zeta[x], x, sector[x])
    }
    
    double const kPosY = ((double)(inObj->map->height))-inObj->posY;
    
    //Deb_line("kPosY = %f", kPosY)
    
    for(int y = 0;y<inObj->height;++y)
    {
        uint32_t * const rowPix = (uint32_t*)inObj->pixels+y*inObj->width;
        
        for(x = 0;x<inObj->width;++x)
        {   
            double deltaX = 0.0,
                deltaY = 0.0,
                m = 0.0,
                b = 0.0;
            int xForHit = 0.0,
                yForHit = 0.0,
                addX = 0,
                addY = 0,
                cellX = -1,
                cellY = -1,
                dCellX = -1,
                dCellY = -1;
            bool done = false;

            uint8_t * const colPix = (uint8_t*)(rowPix+x);
            
//            if(x==27 && y==13)
//            {
//                Deb_line("HERE")
//            }
            
            switch(sector[x])
            {
                case 1:
                {
                    assert(zeta[x]!=0.0); // MT_TODO: TEST: Implement special case!
                    
                    double const theta = 90.0-zeta[x];
                    assert(theta>0.0 && theta<90.0);
                    
                    deltaY = cos(theta*M_PI/180.0)*inObj->d[y];
                    assert(deltaY>0.0);
                    deltaX = sin(theta*M_PI/180.0)*inObj->d[y];
                    addX = 1;
                    addY = 1;
                    dCellX = (int)(deltaX+inObj->posX);
                    
                    dCellY = inObj->map->height-(int)(deltaY+kPosY);                    
                    break;
                }
                case 2:
                {
                    assert(zeta[x]!=90.0); // MT_TODO: TEST: Implement special case!
                    
                    double const theta = 180.0-zeta[x];
                    assert(theta>0.0 && theta<90.0);
                    
                    deltaY = -sin(theta*M_PI/180.0)*inObj->d[y];
                    assert(deltaY<0.0);
                    deltaX = cos(theta*M_PI/180.0)*inObj->d[y];
                    addX = -1;
                    addY = 1;
                    dCellX = (int)(inObj->posX-deltaX);
                    
                    dCellY = inObj->map->height-(int)(kPosY-deltaY);
                    break;
                }
                case 3:
                {
                    assert(zeta[x]!=180.0); // MT_TODO: TEST: Implement special case!
                    
                    double const theta = 270.0-zeta[x];
                    assert(theta>0.0 && theta<90.0);
                    
                    deltaY = cos(theta*M_PI/180.0)*inObj->d[y];
                    assert(deltaY>0.0);
                    deltaX = sin(theta*M_PI/180.0)*inObj->d[y];
                    addX = -1;
                    addY = -1;
                    dCellX = (int)(inObj->posX-deltaX);
                    dCellY = inObj->map->height-(int)(kPosY-deltaY);
                    break;
                }
                case 4:
                {
                    assert(zeta[x]!=270.0); // MT_TODO: TEST: Implement special case!
                    
                    double const theta = 360.0-zeta[x];
                    assert(theta>0.0 && theta<90.0);
                    
                    deltaY = -sin(theta*M_PI/180.0)*inObj->d[y];
                    assert(deltaY<0.0);
                    deltaX = cos(theta*M_PI/180.0)*inObj->d[y];
                    addX = 1;
                    addY = -1;
                    dCellX = (int)(deltaX+inObj->posX);
                    dCellY = inObj->map->height-(int)(deltaY+kPosY);
                    break;
                }
                    
                default:
                    assert(false);
                    break;
            }
            
            assert(deltaX>0.0);
            m = deltaY/deltaX;
               
            b = kPosY-m*inObj->posX;
            
            cellX = (int)inObj->posX;
            cellY = (int)inObj->posY;
           
            xForHit = (int)inObj->posX;
            if(addX==1)
            {
                xForHit += 1;
            }
            yForHit = (int)kPosY;
            if(addY==1)
            {
                yForHit += 1;
            }
            
            if((cellX==dCellX)&&(cellY==dCellY))
            {
                if(y<inObj->floorY)
                {
                    colPix[2] = 0;
                    colPix[1] = 0;
                    colPix[0] = 0xFF;
                }
                else
                {
                    colPix[2] = 0;
                    colPix[1] = 0xFF;
                    colPix[0] = 0;
                    
                    // MT_TODO: TEST: Add exit!
                }
            }
            else
            {
                do
                {   
                    double const hitX = (((double)yForHit)-b)/m,
                        hitY = m*((double)xForHit)+b;

                    bool nextY = false,
                        nextX = false;
                    
                    if(addX==1)
                    {
                        nextY = ((hitX>((double)(xForHit-1)-0.0001))&&(hitX<((double)(xForHit)+0.0001)));
                    }
                    else
                    {
                        nextY = ((hitX<((double)(xForHit+1)+0.0001))&&(hitX>((double)(xForHit)-0.0001)));
                    }
                    if(addY==1)
                    {
                        nextX = ((hitY>((double)(yForHit-1)-0.0001))&&(hitY<((double)(yForHit)+0.0001)));
                    }
                    else
                    {
                        nextX = ((hitY<((double)(yForHit+1)+0.0001))&&(hitY>((double)(yForHit)-0.0001)));
                    }

                    assert(nextY||nextX);

                    if(nextY)
                    {
                        cellY -= addY;
                        yForHit += addY;
                    }
                    if(nextX)
                    {
                        cellX += addX;
                        xForHit += addX;
                    }

                    // MT_TODO: TEST: Use distance to cell for luminance (or something)!

                    if((cellX==dCellX)&&(cellY==dCellY))
                    {
                        {
                            switch((enum CellType)inObj->map->cells[cellY*inObj->map->width+cellX])
                            {
                                case CellType_block_default:
                                    colPix[2] = 0xFF;
                                    colPix[1] = 0;
                                    colPix[0] = 0;
                                    break;
                                case CellType_floor_default:
                                    if(y<inObj->floorY)
                                    {
                                        colPix[2] = 0;
                                        colPix[1] = 0;
                                        colPix[0] = 0xFF;
                                    }
                                    else
                                    {
                                        colPix[2] = 0;
                                        colPix[1] = 0xFF;
                                        colPix[0] = 0;
                                    }
                                    break;
                                case CellType_floor_exit:
                                    if(y<inObj->floorY)
                                    {
                                        colPix[2] = 0;
                                        colPix[1] = 0;
                                        colPix[0] = 0xFF;
                                    }
                                    else
                                    {
                                        colPix[2] = 0;
                                        colPix[1] = 0xFF;
                                        colPix[0] = 0xFF;   
                                    }
                                    break;

                                default:
                                    assert(false);
                                    break;
                            }
                        }

                        done = true;
                    }
                    else
                    {
                        if((cellX<0)||(cellY<0)||(cellX>=inObj->map->width)||(cellY>=inObj->map->height))
                        {
                            Deb_line("x = %d, y = %d, zeta[x] = %f, sector[x] = %d, cellX = %d, cellY = %d.", x, y, zeta[x], sector[x], cellX, cellY)
                                
                            colPix[2] = 0xFF;
                            colPix[1] = 0xFF;
                            colPix[0] = 0;
                            done = true;
                        }
                        else
                        {
                            switch((enum CellType)inObj->map->cells[cellY*inObj->map->width+cellX])
                            {
                                case CellType_block_default:
                                    colPix[2] = 0xFF;
                                    colPix[1] = 0;
                                    colPix[0] = 0;
                                    done = true;
                                    break;
                                case CellType_floor_default:
                                    break;
                                case CellType_floor_exit:
                                    break;

                                default:
                                    assert(false);
                                    break;
                            }
                        }
                    }
                }while(!done);
            }
        }
    }
    
    free(zeta);
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
    
    assert(inObj->eta!=NULL);
    free((double*)inObj->eta);
    
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
    
    double * const eta = malloc(inWidth*sizeof *eta);
    assert(eta!=NULL);
    fillEta(inWidth, inAlpha, eta);
    
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
            
        .eta = eta,
            
        .posX = 0.0,
        .posY = 0.0,
        .gamma = 0.0,
        .map = NULL,
        .pixels = NULL
    };

    memcpy(retVal, &buf, sizeof *retVal);

    return retVal;
}
