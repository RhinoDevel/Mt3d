
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

static const double CEILING_HEIGHT = 1.0; // 1.0 = Height equals length of one floor/ceiling cell.
static const double PLAYER_STEP_LEN = 0.2; // Cell lengths.
static double const PLAYER_ANG_STEP = CALC_TO_RAD(5.0);

static int getFloorYandFill(
    int const inWidth,
    int const inHeight,
    double const inAlpha,
    double const inBeta,
    double const inH,
    double * const inOutD,
    double * const inOutE,
    double * const inOutEta)
{
    int retVal = -1,
        x = 0,
        y = 0;
    
    assert(inAlpha>0.0 && inAlpha<M_PI);
    assert(inBeta>0.0 && inBeta<M_PI);
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
        sYmiddleSqr = sYmiddle*sYmiddle,
        sXmiddleSqr = sXmiddle*sXmiddle,
        floorToEye = inH*CEILING_HEIGHT, // (cell lengths)
        ceilingToEye = CEILING_HEIGHT-floorToEye; // (cell lengths)
    
    for(x = 0;x<inWidth;++x)
    {
        double const diff = xMiddle-(double)x,
            sX = sqrt(diff*diff+sXmiddleSqr);

        betaTopX[x] = asin(yMiddle/sX);
        assert(betaTopX[x]>0.0 && betaTopX[x]<M_PI_2);
        aX[x] = yMiddle/tan(betaTopX[x]);
    }
    
    for(y = 0;y<inHeight;++y)
    {
        double const dY = (double)y,
            diff = yMiddle-dY,
            sY = sqrt(diff*diff+sYmiddleSqr),
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
                
                double const angle = betaTopX[x]-delta;
                assert(angle<M_PI_2);
                
                inOutE[pos] = ceilingToEye/sin(angle);
                inOutD[pos] = inOutE[pos]*cos(angle);
            }
            else
            {
                assert(dY>yMiddle);
                delta = betaTopX[x]+atan((dY-yMiddle)/aX[x]);
                assert(delta>betaTopX[x]);
                
                double const angle = delta-betaTopX[x];
                assert(angle<M_PI_2);
                
                inOutE[pos] = floorToEye/sin(angle);
                inOutD[pos] = inOutE[pos]*cos(angle);

                if(retVal==-1)
                {
                    retVal = y;
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

    free(betaTopX);
    free(aX);
    return retVal;
}

static bool posStep(struct Mt3d * const inOutObj, double const inIota) // Iota: Complete angle in wanted direction (0 rad <= a < 2*PI rad).
{
    double addX = 0.0, // Cell length to add to X position.
        subY = 0.0, // Cell length to subtract from Y position.
        x = inOutObj->posX,
        y = inOutObj->posY;
    
    Calc_fillDeltas(inIota, PLAYER_STEP_LEN, &addX, &subY);
    
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

bool Mt3d_ang_leftOrRight(struct Mt3d * const inOutObj, bool inLeft)
{
    if(inLeft)
    {
        inOutObj->gamma += PLAYER_ANG_STEP;
    }
    else
    {
        inOutObj->gamma -= PLAYER_ANG_STEP;
    }
    inOutObj->gamma = CALC_ANGLE_TO_POS(inOutObj->gamma);
    
    return true;
}

bool Mt3d_pos_forwardOrBackward(struct Mt3d * const inOutObj, bool inForward)
{
    return posStep(inOutObj, inForward?inOutObj->gamma:CALC_ANGLE_TO_POS(inOutObj->gamma-M_PI));
}

bool Mt3d_pos_leftOrRight(struct Mt3d * const inOutObj, bool inLeft)
{
    return posStep(inOutObj, CALC_ANGLE_TO_POS(inOutObj->gamma+(inLeft?1.0:-1.0)*M_PI_2));
}

static void fillPixel(struct Mt3d const * const inObj, enum CellType const inCellType, int const inY, uint8_t * const inOutPix)
{
    switch(inCellType)
    {
        case CellType_block_default:
            inOutPix[2] = 0xFF;
            inOutPix[1] = 0;
            inOutPix[0] = 0;
            break;
        case CellType_floor_default:
            if(inY<inObj->floorY)
            {
                inOutPix[2] = 0;
                inOutPix[1] = 0;
                inOutPix[0] = 0xFF;
            }
            else
            {
                inOutPix[2] = 0;
                inOutPix[1] = 0xFF;
                inOutPix[0] = 0;
            }
            break;
        case CellType_floor_exit:
            if(inY<inObj->floorY)
            {
                inOutPix[2] = 0xFF;
                inOutPix[1] = 0xFF;
                inOutPix[0] = 0;
            }
            else
            {
                inOutPix[2] = 0;
                inOutPix[1] = 0xFF;
                inOutPix[0] = 0xFF;   
            }
            break;

        default:
            assert(false);
            break;
    }   
}

void Mt3d_draw(struct Mt3d * const inObj)
{
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
    
    for(int y = 0;y<inObj->height;++y)
    {
        uint32_t * const rowPix = (uint32_t*)inObj->pixels+y*inObj->width;
        
        for(int x = 0;x<inObj->width;++x)
        {   
            double deltaX = 0.0,
                deltaY = 0.0,
                countLen = -1.0; // Means unset.
            int cellX = (int)inObj->posX,
                cellY = (int)inObj->posY;
            
            int const pos = y*inObj->width+x;
            
            {
                double const zetaUnchecked = inObj->eta[pos]+inObj->gamma; // (might be out of expected range, see usage below)

                Calc_fillDeltas(CALC_ANGLE_TO_POS(zetaUnchecked), inObj->d[pos], &deltaX, &deltaY);
            }
            
            assert(deltaX!=0.0); // MT_TODO: TEST: Implement special case! 
            
            uint8_t * const colPix = (uint8_t*)(rowPix+x);
            
            // Get coordinates of cell where the line/"ray" reaches either floor or ceiling:
            //
            int const dCellX = (int)(deltaX+inObj->posX),
                dCellY = (int)((double)(inObj->map->height-1)-(deltaY+kPosY)); // Cartesian Y to cell Y coordinate conversion.
            
            if((cellX==dCellX)&&(cellY==dCellY)) // Line/"ray" hits floor or ceiling in current/player's cell.
            {            
                countLen = inObj->e[pos];
                fillPixel(inObj, (enum CellType)inObj->map->cells[cellY*inObj->map->width+cellX], y, colPix);
            }
            else
            {
                int const addX = CALC_SIGN_FROM_DOUBLE(deltaX),
                    addY = CALC_SIGN_FROM_DOUBLE(deltaY);
                
                bool noHit = true;
                int xForHit = cellX+(int)(addX>0),
                    yForHit = (int)kPosY+(int)(addY>0); // (Cartesian Y coordinate)
                
                // Store last reached coordinates for brightness (and more) calculations:
                //
                double lastX = inObj->posX,
                    kLastY = kPosY; // (Cartesian Y coordinate)
                
                // Values to represent line in Slope-intercept form:
                //
                double const m = deltaY/deltaX, // Slope
                    b = kPosY-m*inObj->posX; // Y-intercept
                
                do
                {   
                    double const hitX = (((double)yForHit)-b)/m,
                        hitY = m*((double)xForHit)+b;

                    bool nextY = false,
                        nextX = false;

                    if(addX==1)
                    {
                        nextY = (int)hitX<xForHit;
                    }
                    else
                    {
                        nextY = hitX>=(double)xForHit;
                    }
                    if(addY==1)
                    {
                        nextX = (int)hitY<yForHit;
                    }
                    else
                    {
                        nextX = hitY>=(double)yForHit;
                    }

                    assert(nextY||nextX);

                    if(nextY)
                    {
                        // Update last reached coordinates:
                        //
                        lastX = hitX;
                        kLastY = (double)yForHit;
                        
                        cellY -= addY;
                        yForHit += addY;
                    }
                    if(nextX)
                    {
                        // Update last reached coordinates:
                        //
                        lastX = (double)xForHit;
                        kLastY = hitY;
                        
                        cellX += addX;
                        xForHit += addX;
                    }
                    
                    assert(cellX>=0 && cellX<inObj->map->width);
                    assert(cellY>=0 && cellY<inObj->map->height);
                    
                    enum CellType const cellType = (enum CellType)inObj->map->cells[cellY*inObj->map->width+cellX];
                    
                    noHit = cellX!=dCellX || cellY!=dCellY; // noHit = false => Last cell is reached, where line/"ray" (at least theoretically) hits either floor or ceiling.
                    
                    switch(cellType)
                    {
                        case CellType_block_default: // Line/"ray" hits block's surface.
                        {
                            double const diffY = kLastY-kPosY,
                                diffX = lastX-inObj->posX,
                                diffXY = sqrt(diffY*diffY+diffX*diffX);
                            
                            noHit = false;
                            countLen = diffXY*inObj->e[pos]/inObj->d[pos];
                            fillPixel(inObj, CellType_block_default, y, colPix);
                            
//                            {
//                                double const opposite = sqrt(countLen*countLen-diffXY*diffXY);
//                                double imgX = nextX?(double)(inObj->map->height-1)-kLastY:lastX, // (Cartesian Y to cell Y coordinate conversion, if necessary)
//                                    imgY = opposite;
//
//                                imgX -= (double)((int)imgX); // Removes integer part.
//                                if(nextX)
//                                {
//                                    if(addX<0.0)
//                                    {
//                                        imgX = 1.0-imgX;
//                                    }
//                                }
//                                else
//                                {
//                                    if(addY<0.0)
//                                    {
//                                        imgX = 1.0-imgX;
//                                    }
//                                }
//                                assert(imgX>=0.0 && imgX<1.0);
//
//                                if(y<inObj->floorY)
//                                {
//                                    imgY += CEILING_HEIGHT*inObj->h;
//                                }
//                                else
//                                {
//                                    imgY = CEILING_HEIGHT*inObj->h-imgY;
//                                }
//                                imgY = CEILING_HEIGHT-imgY;
//                                assert(imgY>=0.0 && imgY<1.0);
//
//                                if(imgY>0.25&&imgY<0.35)
//                                {
//                                    colPix[2] = 0;
//                                    colPix[1] = 0;
//                                    colPix[0] = 0;
//                                }
//                                if(imgX>0.25&&imgX<0.35)
//                                {
//                                    colPix[2] = 0xFF;
//                                    colPix[1] = 0xFF;
//                                    colPix[0] = 0xFF;
//                                }
//                            }

                            break;
                        }

                        case CellType_floor_default: // (falls through).
                        case CellType_floor_exit:
                            if(!noHit) // Line/"ray" hits floor or ceiling.
                            {
                                countLen = inObj->e[pos];
                                fillPixel(inObj, cellType, y, colPix);
                            }
                            //
                            // Otherwise: Line/"ray" passes through current cell to the next.
                            
                            break;

                        default:
                            assert(false);
                            break;
                    }
                }while(noHit);
            }

            // Set brightness based on map constants and line length from player:
            //
            {
                assert(countLen!=-1.0);

                double const brightness = (inObj->map->maxVisible-fmin(countLen, inObj->map->maxVisible))/inObj->map->maxVisible; // countLen 0 = 1.0, countLen maxVisible = 0.0;
                int const sub = (int)((inObj->map->maxDarkness*255.0)*(1.0-brightness)+0.5), // Rounds
                    r = (int)colPix[2]-sub,
                    g = (int)colPix[1]-sub,
                    blue = (int)colPix[0]-sub;

                colPix[2] = r>0?(unsigned char)r:0;
                colPix[1] = g>0?(unsigned char)g:0;
                colPix[0] = blue>0?(unsigned char)blue:0;
            }
        }
    }
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
    
    assert(inObj->eta!=NULL);
    free((double*)inObj->eta);
    
    free(inObj);
}

void Mt3d_update(double const inAlpha, double const inBeta, double const inH, struct Mt3d * const inOutObj)
{
    inOutObj->floorY = getFloorYandFill(inOutObj->width, inOutObj->height, inAlpha, inBeta, inH, inOutObj->d, inOutObj->e, inOutObj->eta);
    inOutObj->alpha = inAlpha;
    inOutObj->beta = inBeta;
    inOutObj->h = inH;
    
    //Deb_line("Alpha = %f degree, beta = %f degree, h(-eight) = %f cell length.", CALC_TO_DEG(inAlpha), CALC_TO_DEG(inBeta), inH)
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
        .floorY = -1, // Invalidates
            
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
