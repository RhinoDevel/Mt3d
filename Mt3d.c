
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
static const double CEILING_HEIGHT = 1.0; // 1.0 = Height equals length of one floor/ceiling cell.

static void fill(
    int const inWidth,
    int const inHeight,
    int const inAlpha,
    double const inBeta,
    double const inH,
    double * const inOutD,
    double * const inOutE,
    int * const inOutFloorY,
    double * const inOutEta)
{
    int x = 0,
        y = 0;
    
    assert(inHeight%2==0);
    assert(inWidth%2==0);
    
    double * const alphaLeftY = malloc(inHeight*sizeof *alphaLeftY), // To hold alphaX/2.
        * const betaTopX = malloc(inWidth*sizeof *betaTopX), // To hold betaX/2.
        * const aX = malloc(inWidth*sizeof *aX); // Lengths in pixel between point of view and pixel plane for each x.
   
    assert(alphaLeftY!=NULL);
    assert(betaTopX!=NULL);
    assert(aX!=NULL);
    
    double const yMiddle = (double)(inHeight-1)/2.0,
        xMiddle = (double)(inWidth-1)/2.0,
        betaMiddle = (double)inBeta*M_PI/180.0,
        alphaMiddle = (double)inAlpha*M_PI/180.0,
        sYmiddle = xMiddle/sin(alphaMiddle/2.0),
        sXmiddle = yMiddle/sin(betaMiddle/2.0),
        sYmiddleSqr = pow(sYmiddle, 2.0),
        sXmiddleSqr = pow(sXmiddle, 2.0),
        floorToEye = inH*CEILING_HEIGHT, // (cell lengths)
        ceilingToEye = CEILING_HEIGHT-floorToEye; // (cell lengths)
    
    for(y =0;y<inHeight;++y)
    {
        double const dY = (double)y,
            sY = sqrt(pow(yMiddle-dY, 2.0)+sYmiddleSqr);
        
        alphaLeftY[y] = asin(xMiddle/sY);
    }
    for(x =0;x<inWidth;++x)
    {
        double const dX = (double)x,
            sX = sqrt(pow(xMiddle-dX, 2.0)+sXmiddleSqr);
        
        betaTopX[x] = asin(yMiddle/sX);
        aX[x] = yMiddle/tan(betaTopX[x]);
    }

    for(x = 0;x<inWidth;++x)
    {
        inOutFloorY[x] = -1;
    }
    
    for(y = 0;y<inHeight;++y)
    {
        double const dY = (double)y;
        double delta = 0.0;

        for(x = 0;x<inWidth;++x)
        {
            int const pos = y*inWidth+x;
            double epsilon = -1.0; // Epsilon is the angle from pixel 0 = 0 degrees to pixel inWidth-1 = inAlpha degrees.
            
            if(dY<yMiddle)
            {
                delta = betaTopX[x]-atan((yMiddle-dY)/aX[x]);
            }
            else
            {
                delta = betaTopX[x]+atan((dY-yMiddle)/aX[x]);
            }
            
            if(delta<betaTopX[x])
            {
                assert(dY<yMiddle);
                inOutE[pos] = ceilingToEye/sin(betaTopX[x]-delta);
                inOutD[pos] = inOutE[pos]*cos(betaTopX[x]-delta);
            }
            else
            {
                if(delta>betaTopX[x])
                {
                    assert(dY>yMiddle);
                    inOutE[pos] = floorToEye/sin(delta-betaTopX[x]);
                    inOutD[pos] = inOutE[y]*cos(delta-betaTopX[x]);

                    if(inOutFloorY[x]==-1)
                    {
                        inOutFloorY[x] = y;
                    }
                }
                else
                {
                    assert(dY==yMiddle);
                    inOutD[pos] = EQUAL_D_AND_E;
                    inOutE[pos] = EQUAL_D_AND_E;

                    //Deb_line("Warning: Delta %f to use for y value %d exactly equals top part %f of beta %f!", delta*180.0/M_PI, y, betaTop*180.0/M_PI, inBeta)
                }
            }
            
            double const dX = (double)x;
            

            if(dX<xMiddle)
            {
                epsilon = alphaLeftY[y]-atan((xMiddle-dX)/aX[x]);
            }
            else
            {   
                epsilon = alphaLeftY[y]+atan((dX-xMiddle)/aX[x]);
            }

            inOutEta[pos] = (alphaLeftY[y]-epsilon)*180.0/M_PI; // Eta is a pre-calculated angle to be used with player view angle, later.

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

            //Deb_line("epsilon = %f, inOutEta[%d] = %f.", epsilon*180.0/M_PI, x, inOutEta[x])
        }
        
        //Deb_line("delta = %f, inOutE[%d] = %f cell lengths, inOutD[%d] = %f cell lengths.", delta*180.0/M_PI, y, inOutE[y], y, inOutD[y])
    }
    
    //assert(inOutFloorY[x]>0);
    free(alphaLeftY);
    free(betaTopX);
    free(aX);
}

void Mt3d_draw(struct Mt3d * const inObj)
{
    int x = 0,
        y = 0;
    double * const zeta = malloc(inObj->height*inObj->width*sizeof *zeta);            // MT_TODO: TEST: Move to Mt3d object creation/deletion!
    unsigned char * const sector = malloc(inObj->height*inObj->width*sizeof *sector); //
    assert(zeta!=NULL);
    assert(sector!=NULL);
    
    for(y = 0;y<inObj->height;++y)
    {
        for(x = 0;x<inObj->width;++x)
        {
            int const pos = y*inObj->width+x;
            
            zeta[pos] = inObj->eta[pos]+inObj->gamma;
            if(zeta[pos]<0.0)
            {
                zeta[pos] = 360.0+zeta[pos];
            }
            else
            {
                if(zeta[pos]>=360.0)
                {
                    zeta[pos] -= 360.0;
                }
            }
            assert((zeta[pos]>=0.0)&&(zeta[pos]<360.0));

            sector[pos] = (unsigned char)(((int)zeta[pos])/90+1);
            assert((sector[pos]>=1)&&(sector[pos]<=4));

            //Deb_line("zeta[%d] = %f | sector[%d] = %d", x, zeta[x], x, sector[x])
        }
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
            int const pos = y*inObj->width+x;
            
            uint8_t * const colPix = (uint8_t*)(rowPix+x);
            
//            if(x==27 && y==13)
//            {
//                Deb_line("HERE")
//            }
            
            switch(sector[pos])
            {
                case 1:
                {
                    assert(zeta[pos]!=0.0); // MT_TODO: TEST: Implement special case!
                    
                    double const theta = 90.0-zeta[pos];
                    assert(theta>0.0 && theta<90.0);
                    
                    deltaY = cos(theta*M_PI/180.0)*inObj->d[pos];
                    assert(deltaY>0.0);
                    deltaX = sin(theta*M_PI/180.0)*inObj->d[pos];
                    addX = 1;
                    addY = 1;
                    dCellX = (int)(deltaX+inObj->posX);
                    
                    dCellY = inObj->map->height-(int)(deltaY+kPosY);                    
                    break;
                }
                case 2:
                {
                    assert(zeta[pos]!=90.0); // MT_TODO: TEST: Implement special case!
                    
                    double const theta = 180.0-zeta[pos];
                    assert(theta>0.0 && theta<90.0);
                    
                    deltaY = -sin(theta*M_PI/180.0)*inObj->d[pos];
                    assert(deltaY<0.0);
                    deltaX = cos(theta*M_PI/180.0)*inObj->d[pos];
                    addX = -1;
                    addY = 1;
                    dCellX = (int)(inObj->posX-deltaX);
                    
                    dCellY = inObj->map->height-(int)(kPosY-deltaY);
                    break;
                }
                case 3:
                {
                    assert(zeta[pos]!=180.0); // MT_TODO: TEST: Implement special case!
                    
                    double const theta = 270.0-zeta[pos];
                    assert(theta>0.0 && theta<90.0);
                    
                    deltaY = cos(theta*M_PI/180.0)*inObj->d[pos];
                    assert(deltaY>0.0);
                    deltaX = sin(theta*M_PI/180.0)*inObj->d[pos];
                    addX = -1;
                    addY = -1;
                    dCellX = (int)(inObj->posX-deltaX);
                    dCellY = inObj->map->height-(int)(kPosY-deltaY);
                    break;
                }
                case 4:
                {
                    assert(zeta[pos]!=270.0); // MT_TODO: TEST: Implement special case!
                    
                    double const theta = 360.0-zeta[pos];
                    assert(theta>0.0 && theta<90.0);
                    
                    deltaY = -sin(theta*M_PI/180.0)*inObj->d[pos];
                    assert(deltaY<0.0);
                    deltaX = cos(theta*M_PI/180.0)*inObj->d[pos];
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
                if(y<inObj->floorY[y])
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
                                    if(y<inObj->floorY[y])
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
                                    if(y<inObj->floorY[y])
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
                            //Deb_line("x = %d, y = %d, zeta[x] = %f, sector[x] = %d, cellX = %d, cellY = %d.", x, y, zeta[x], sector[x], cellX, cellY)
                                
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
    free((double*)inObj->d);
    
    assert(inObj->e!=NULL);
    free((double*)inObj->e);
    
    assert(inObj->floorY!=NULL);
    free((int*)inObj->floorY);
    
    assert(inObj->eta!=NULL);
    free((double*)inObj->eta);
    
    free(inObj);
}

void Mt3d_update(int const inAlpha, double const inBeta, double const inH, struct Mt3d * const inOutObj)
{
    fill(inOutObj->width, inOutObj->height, inAlpha, inBeta, inH, inOutObj->d, inOutObj->e, inOutObj->floorY, inOutObj->eta);
    
    inOutObj->alpha = inAlpha;
    inOutObj->beta = inBeta;
    inOutObj->h = inH;
    
    Deb_line("Alpha = %d, beta = %f, h(-eight) = %f.", inAlpha, inBeta, inH)
}

struct Mt3d * Mt3d_create(int const inWidth, int const inHeight, int const inAlpha, double const inBeta, double const inH)
{
    struct Mt3d * const retVal = malloc(sizeof *retVal);
    assert(retVal!=NULL);
    
    size_t const pixelCount = inHeight*inWidth;
    
    double * const d = malloc(pixelCount*sizeof *d);
    assert(d!=NULL);
    double * const e = malloc(pixelCount*sizeof *e);
    assert(e!=NULL);
    
    int * const floorY = malloc(inWidth*sizeof *floorY);
    assert(floorY!=NULL);
    
    double * const eta = malloc(pixelCount*sizeof *eta);
    assert(eta!=NULL);
    
    struct Mt3d const buf = (struct Mt3d)
    {
        .width = inWidth,
        .height = inHeight,
        .alpha = 0, // Invalidates
        .beta = 0.0, // Invalidates
        .h = 0.0, // Invalidates
            
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

    Mt3d_update(inAlpha, inBeta, inH, retVal);
    
    return retVal;
}
