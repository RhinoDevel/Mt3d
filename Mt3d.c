
// MT, 2016aug22

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

#include "Deb.h"
#include "Mt3d.h"
#include "Sys.h"
#include "Calc.h"

//#define MT_DOUBLE_TO_INT_ROUND(x) ((int)(((x)<0.0)?((x)-0.5):((x)+0.5)))

static const double CEILING_HEIGHT = 1.0; // 1.0 = Height equals length of one floor/ceiling cell.
static const double PLAYER_STEP_LEN = 0.2; // Cell lengths.

static void fill(
    int const inWidth,
    int const inHeight,
    double const inAlpha,
    double const inBeta,
    double const inH,
    double * const inOutD,
    double * const inOutE,
    int * const inOutFloorY,
    double * const inOutEta)
{
    int x = 0,
        y = 0;
    
    assert(inBeta>0.0);
    assert(inHeight%2==0);
    assert(inWidth%2==0);
    
    double * const betaTopX = malloc(inWidth*sizeof *betaTopX), // To hold betaX/2.
        * const aX = malloc(inWidth*sizeof *aX); // Lengths in pixel between point of view and pixel plane for each x.
   
    assert(betaTopX!=NULL);
    assert(aX!=NULL);
    
    double const yMiddle = (double)(inHeight-1)/2.0,
        xMiddle = (double)(inWidth-1)/2.0,
        sYmiddle = xMiddle/sin(inAlpha/2.0),
        sXmiddle = yMiddle/sin(inBeta/2.0),
        sYmiddleSqr = pow(sYmiddle, 2.0),
        sXmiddleSqr = pow(sXmiddle, 2.0),
        floorToEye = inH*CEILING_HEIGHT, // (cell lengths)
        ceilingToEye = CEILING_HEIGHT-floorToEye; // (cell lengths)
    
    for(x = 0;x<inWidth;++x)
    {
        double const dX = (double)x,
            sX = sqrt(pow(xMiddle-dX, 2.0)+sXmiddleSqr);
        
        betaTopX[x] = asin(yMiddle/sX);
        assert(betaTopX[x]>0.0);
        aX[x] = yMiddle/tan(betaTopX[x]);
        
        inOutFloorY[x] = -1;
    }
    
    for(y = 0;y<inHeight;++y)
    {
        double const dY = (double)y,
            sY = sqrt(pow(yMiddle-dY, 2.0)+sYmiddleSqr),
            alphaLeftY = asin(xMiddle/sY); // To hold alphaX/2.
        double delta = 0.0;

        for(x = 0;x<inWidth;++x)
        {
            int const pos = y*inWidth+x;
            double epsilon = -1.0; // Epsilon is the angle from pixel 0 = 0 degree to pixel inWidth-1 = inAlpha degree.
            
            if(dY<yMiddle)
            {
                delta = betaTopX[x]-atan((yMiddle-dY)/aX[x]);
                assert(delta<betaTopX[x]);
                
                inOutE[pos] = ceilingToEye/sin(betaTopX[x]-delta); 
                inOutD[pos] = inOutE[pos]*cos(betaTopX[x]-delta);
            }
            else
            {
                assert(dY>yMiddle);
                delta = betaTopX[x]+atan((dY-yMiddle)/aX[x]);
                assert(delta>betaTopX[x]);
                
                inOutE[pos] = floorToEye/sin(delta-betaTopX[x]);
                inOutD[pos] = inOutE[pos]*cos(delta-betaTopX[x]);

                if(inOutFloorY[x]==-1)
                {
                    inOutFloorY[x] = y;
                }
            }
            
            double const dX = (double)x;

            if(dX<xMiddle)
            {
                epsilon = alphaLeftY-atan((xMiddle-dX)/aX[x]);
            }
            else
            {   
                assert(dX!=xMiddle);
                epsilon = alphaLeftY+atan((dX-xMiddle)/aX[x]);
            }

            inOutEta[pos] = alphaLeftY-epsilon; // Eta is a pre-calculated angle to be used with player view angle, later.
            //inOutEta[pos] = CALC_ANGLE_TO_POS(inOutEta[pos]); // Not necessary, here.

            //Deb_line("y = %d, x = %d: betaTopX = %f degree, delta = %f degree, e = %f cell length, d = %f cell length, epsilon = %f degree, eta = %f degree.", y, x, CALC_TO_DEG(betaTopX[x]), CALC_TO_DEG(delta), inOutE[pos], inOutD[pos], CALC_TO_DEG(epsilon), CALC_TO_DEG(inOutEta[x]))
        }
    }
    
#ifndef NDEBUG
    {
        int firstY = -1;
        
        for(x = 0;x<inWidth;++x)
        {
            if(firstY==-1)
            {
                assert(inOutFloorY[x]>0);
                firstY = inOutFloorY[x];
            }
            else
            {
                assert(firstY==inOutFloorY[x]);
            }
        }
    }
#endif //NDEBUG

    free(betaTopX);
    free(aX);
}

static bool Mt3d_pos_step(struct Mt3d * const inOutObj, double const inIota) // Iota: Complete angle in wanted direction (0 rad <= a < 2*PI rad).
{
    double const MIN = 0.001;
    int const zeroSector = (int)CALC_TO_DEG(inIota)/90; // Sector of Cartesian coordinate system from 0 to 3 instead of I, II, III, IV (counter-clockwise).
    double kappa = -1.0;
    double addX = 0.0, // Cell length to add to X position.
        subY = 0.0, // Cell length to subtract from Y position.
        x = inOutObj->posX,
        y = inOutObj->posY;
    
    switch(zeroSector)
    {
        case 0:
            kappa = inIota;
            if(kappa<MIN)
            {
                addX = PLAYER_STEP_LEN;
                break;    
            }
            addX = PLAYER_STEP_LEN*cos(kappa);
            subY = PLAYER_STEP_LEN*sin(kappa);
            break;
        case 1:
            kappa = inIota-CALC_PI_MUL_0_5;
            if(kappa<MIN)
            {
                subY = PLAYER_STEP_LEN;
                break;    
            }
            addX = -PLAYER_STEP_LEN*sin(kappa);
            subY = PLAYER_STEP_LEN*cos(kappa);
            break;
        case 2:
            kappa = inIota-M_PI;
            if(kappa<MIN)
            {
                addX = -PLAYER_STEP_LEN;
                break;    
            }
            addX = -PLAYER_STEP_LEN*cos(kappa);
            subY = -PLAYER_STEP_LEN*sin(kappa);
            break;
        case 3:
            kappa = inIota-CALC_PI_MUL_1_5;
            if(kappa<MIN)
            {
                subY = -PLAYER_STEP_LEN;
                break;    
            }
            addX = PLAYER_STEP_LEN*sin(kappa);
            subY = -PLAYER_STEP_LEN*cos(kappa);
            break;
            
        default:
            assert(false);
            break;
    }
    x += addX;
    y -= subY; // Subtraction, because cell coordinate system starts on top, Cartesian coordinate system at bottom.
    
    if((enum CellType)inOutObj->map->cells[(int)y*inOutObj->map->width+(int)x]==CellType_block_default) // MT_TODO: TEST: Player has no width!
    {
        return false;
    }
    inOutObj->posX = x;
    inOutObj->posY = y; 
    return true;
}

bool Mt3d_pos_forwardOrBackward(struct Mt3d * const inOutObj, bool inForward)
{
    return Mt3d_pos_step(inOutObj, inForward?inOutObj->gamma:CALC_ANGLE_TO_POS(inOutObj->gamma-M_PI));
}

bool Mt3d_pos_leftOrRight(struct Mt3d * const inOutObj, bool inLeft)
{
    return Mt3d_pos_step(inOutObj, CALC_ANGLE_TO_POS(inOutObj->gamma+(inLeft?1.0:-1.0)*CALC_PI_MUL_0_5));
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
            zeta[pos] = CALC_ANGLE_TO_POS(zeta[pos]);

            sector[pos] = (unsigned char)(((int)CALC_TO_DEG(zeta[pos]))/90+1); // Integer division (truncates).
            assert((sector[pos]>=1)&&(sector[pos]<=4));

            //Deb_line("y = %d, x = %d: zeta = %f degree, sector = %d", y, x, CALC_TO_DEG(zeta[x]), sector[x])
        }
    }

    // Cell coordinates     Cartesian coordinates
    //
    //  00000000001          00000000001
    //  01234567890          01234567890
    // 0XXXXXXXXXXX         8
    // 1X.........X         7
    // 2X.........X         6
    // 3X...E.....X         5
    // 4X.........X         4
    // 5X...p.X...X         3
    // 6X.........X         2
    // 7X.........X         1
    // 8XXXXXXXXXXX         0
    //
    // => yCartesian = heightCell-1-yCell
    // => yCell      = heightCell-1-yCartesian
    
    double const kPosY = ((double)(inObj->map->height-1))-inObj->posY;
    
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
                dCellY = -1,
                xCount = 0,
                yCount = 0;
            bool done = false;
            int const pos = y*inObj->width+x;
            
            uint8_t * const colPix = (uint8_t*)(rowPix+x);
            
            switch(sector[pos])
            {
                case 1:
                {
                    assert(zeta[pos]!=0.0); // MT_TODO: TEST: Implement special case!
                    
                    double const theta = M_PI/2.0-zeta[pos];
                    assert(theta>0.0 && theta<M_PI/2.0);
                    
                    deltaY = cos(theta)*inObj->d[pos];
                    assert(deltaY>0.0);
                    deltaX = sin(theta)*inObj->d[pos];
                    addX = 1;
                    addY = 1;
                    dCellX = (int)(deltaX+inObj->posX);
                    
                    double const kY = deltaY+kPosY;
                    dCellY = (int)((double)(inObj->map->height-1)-kY);//(kY>0.0?kY+0.5:kY-0.5);                    
                    break;
                }
                case 2:
                {
                    assert(zeta[pos]!=M_PI/2.0); // MT_TODO: TEST: Implement special case!
                    
                    double const theta = M_PI-zeta[pos];
                    assert(theta>0.0 && theta<M_PI/2.0);
                    
                    deltaY = -sin(theta)*inObj->d[pos];
                    assert(deltaY<0.0);
                    deltaX = cos(theta)*inObj->d[pos];
                    addX = -1;
                    addY = 1;
                    dCellX = (int)(inObj->posX-deltaX);
                    
                    double const kY = kPosY-deltaY;
                    dCellY = (int)((double)(inObj->map->height-1)-kY);//(kY>0.0?kY+0.5:kY-0.5);
                    break;
                }
                case 3:
                {
                    assert(zeta[pos]!=M_PI); // MT_TODO: TEST: Implement special case!
                    
                    double const theta = 3.0*M_PI/2.0-zeta[pos];
                    assert(theta>0.0 && theta<M_PI/2.0);
                    
                    deltaY = cos(theta)*inObj->d[pos];
                    assert(deltaY>0.0);
                    deltaX = sin(theta)*inObj->d[pos];
                    addX = -1;
                    addY = -1;
                    dCellX = (int)(inObj->posX-deltaX);
                    
                    double const kY = kPosY-deltaY;
                    dCellY = (int)((double)(inObj->map->height-1)-kY);//(kY>0.0?kY+0.5:kY-0.5);
                    break;
                }
                case 4:
                {
                    assert(zeta[pos]!=3.0*M_PI/2.0); // MT_TODO: TEST: Implement special case!
                    
                    double const theta = CALC_PI_MUL_2_0-zeta[pos];
                    assert(theta>0.0 && theta<M_PI/2.0);
                    
                    deltaY = -sin(theta)*inObj->d[pos];
                    assert(deltaY<0.0);
                    deltaX = cos(theta)*inObj->d[pos];
                    addX = 1;
                    addY = -1;
                    dCellX = (int)(deltaX+inObj->posX);
                    
                    double const kY = deltaY+kPosY;
                    dCellY = (int)((double)(inObj->map->height-1)-kY);//(kY>0.0?kY+0.5:kY-0.5);
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
            yForHit = (int)(kPosY/*+0.5*/); // MT_TODO: TEST: There is some bug (maybe here) causing wrong frame content, if player's Y position coordinate is non-integer!
            if(addY==1)
            {
                yForHit += 1;
            }
            
            if((cellX==dCellX)&&(cellY==dCellY))
            {
                switch((enum CellType)inObj->map->cells[cellY*inObj->map->width+cellX])
                {
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
                            colPix[2] = 0xFF;
                            colPix[1] = 0xFF;
                            colPix[0] = 0;
                        }
                        else
                        {
                            colPix[2] = 0;
                            colPix[1] = 0xFF;
                            colPix[0] = 0xFF;   
                        }
                        break;

                    case CellType_block_default: // (falls through)
                    default:
                        assert(false);
                        break;
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
                        ++yCount;
                    }
                    if(nextX)
                    {
                        cellX += addX;
                        xForHit += addX;
                        ++xCount;
                    }
                    
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
                                        colPix[2] = 0xFF;
                                        colPix[1] = 0xFF;
                                        colPix[0] = 0;
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
                            assert(false); // Shouldn't this be impossible?
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

            double const maxVisible = 7.0, // In cell length.
                maxDarkness = 0.8,
                countLen = xCount==0?(double)yCount:yCount==0?(double)xCount:sqrt(pow((double)xCount, 2.0)+pow((double)yCount, 2.0)),
                brightness = (maxVisible-fmin(countLen, maxVisible))/maxVisible; // countLen 0 = 1.0, countLen maxVisible = 0.0;
            int const sub = (int)((maxDarkness*255.0)*(1.0-brightness)+0.5), // Rounds
                r = (int)colPix[2]-sub,
                g = (int)colPix[1]-sub,
                blue = (int)colPix[0]-sub;
            colPix[2] = r>0?(unsigned char)r:0;
            colPix[1] = g>0?(unsigned char)g:0;
            colPix[0] = blue>0?(unsigned char)blue:0;
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

void Mt3d_update(double const inAlpha, double const inBeta, double const inH, struct Mt3d * const inOutObj)
{
    fill(inOutObj->width, inOutObj->height, inAlpha, inBeta, inH, inOutObj->d, inOutObj->e, inOutObj->floorY, inOutObj->eta);
    
    inOutObj->alpha = inAlpha;
    inOutObj->beta = inBeta;
    inOutObj->h = inH;
    
    Deb_line("Alpha = %f degree, beta = %f degree, h(-eight) = %f cell length.", CALC_TO_DEG(inAlpha), CALC_TO_DEG(inBeta), inH)
}

struct Mt3d * Mt3d_create(int const inWidth, int const inHeight, double const inAlpha, double const inBeta, double const inH)
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
        .alpha = 0.0, // Invalidates
        .beta = 0.0, // Invalidates
        .h = -1.0, // Invalidates
            
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
