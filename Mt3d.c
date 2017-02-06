
// MT, 2016aug22

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "Deb.h"
#include "Mt3d.h"
#include "Sys.h"
#include "Calc.h"
#include "Bmp.h"

static double const CEILING_HEIGHT = 1.0; // 1.0 = Height equals length of one floor/ceiling cell.
static double const PLAYER_STEP_LEN = 0.2; // Cell lengths.
static double const PLAYER_ANG_STEP = CALC_TO_RAD(5.0);

/** Return current second of game time.
 */
static uint64_t getSecond(struct Mt3d const * const inObj)
{
    return (inObj->updateCount*(uint64_t)inObj->msPerUpdate)/1000;
}

static void fill(
    int const inWidth,
    int const inHeight,
    double const inAlpha,
    double const inBeta,
    double const inTheta, // CCW angle in radian describing z-axis rotation.
    double const inH,
    double * const inOutD,
    double * const inOutE,
    double * const inOutEta,
    enum HitType * const inOutHitType)
{
    assert(inAlpha>0.0 && inAlpha<M_PI);
    assert(inBeta>0.0 && inBeta<M_PI);
    assert(inTheta>=0.0 && inTheta<Calc_PiMul2);
    assert(inHeight%2==0);
    assert(inWidth%2==0);

    int x = 0,
        y = 0;

    double const dHeight = (double)inHeight,
        xMiddle = (double)(inWidth-1)/2.0,
        yMiddle = (dHeight-1.0)/2.0,
        floorToEye = inH*CEILING_HEIGHT, // (cell lengths)
        ceilingToEye = CEILING_HEIGHT-floorToEye, // (cell lengths)
        sXmiddle = yMiddle/sin(inBeta/2.0),
        sYmiddle = xMiddle/sin(inAlpha/2.0),
        sXmiddleSqr = sXmiddle*sXmiddle,
        sYmiddleSqr = sYmiddle*sYmiddle;

    for(y = 0;y<inHeight;++y)
    {
        int const rowPos = y*inWidth;
        double const cY = CALC_CARTESIAN_Y((double)y, dHeight);

        for(x = 0;x<inWidth;++x)
        {
            int const pos = rowPos+x;
            double xRot = -1.0,
                yRot = -1.0,
                betaTopX = 0.0,
                aX = 0.0;

            { // Calculate rotated x and y coordinates in pixel/bitmap coordinates (Y starts at top-left):
                double cYrot = -1.0;

                Calc_fillRotated(x-xMiddle, cY-yMiddle, inTheta, &xRot, &cYrot);

                xRot = xRot+xMiddle;

                cYrot = cYrot+yMiddle;
                yRot = CALC_CARTESIAN_Y(cYrot, dHeight);
            }

            {
                double const diff = xMiddle-xRot,
                    sX = sqrt(diff*diff+sXmiddleSqr);

                betaTopX = asin(yMiddle/sX);
                assert(betaTopX>0.0 && betaTopX<M_PI_2);
                aX = yMiddle/tan(betaTopX);
            }

            if(yRot==yMiddle)
            {
                inOutHitType[pos] = HitType_none;
                inOutE[pos] = -1.0; // Infinite
                inOutD[pos] = -1.0; // Infinite
            }
            else
            {
                if(yRot<yMiddle)
                {
                    inOutHitType[pos] = HitType_ceil;

                    double const delta = betaTopX-atan((yMiddle-yRot)/aX);
                    assert(delta<betaTopX);

                    double const angle = betaTopX-delta;
                    assert(angle<M_PI_2);

                    inOutE[pos] = ceilingToEye/sin(angle);
                    inOutD[pos] = inOutE[pos]*cos(angle);
                }
                else
                {
                    inOutHitType[pos] = HitType_floor;

                    double const delta = betaTopX+atan((yRot-yMiddle)/aX);
                    assert(delta>betaTopX);

                    double const angle = delta-betaTopX;
                    assert(angle<M_PI_2);

                    inOutE[pos] = floorToEye/sin(angle);
                    inOutD[pos] = inOutE[pos]*cos(angle);
                }
            }

            {
                double const diff = yMiddle-yRot,
                    sY = sqrt(diff*diff+sYmiddleSqr),
                    alphaLeftY = asin(xMiddle/sY); // To hold alphaX/2.
                double epsilon = 0.0;

                if(xRot<=xMiddle) // atan(0) equals 0, so it is OK, if equal.
                {
                    epsilon = alphaLeftY-atan((xMiddle-xRot)/aX);
                }
                else
                {
                    epsilon = alphaLeftY+atan((xRot-xMiddle)/aX);
                }

                inOutEta[pos] = alphaLeftY-epsilon; // Eta is a pre-calculated angle to be used with player view angle, later.
                //inOutEta[pos] = CALC_ANGLE_TO_POS(inOutEta[pos]); // Not necessary, here.
            }
        }
    }
}

static bool posStep(struct Mt3d * const inOutObj, double const inIota) // Iota: Complete angle in wanted direction (0 rad <= a < 2*PI rad).
{
    double addX = 0.0, // Cell length to add to X position.
        subY = 0.0, // Cell length to subtract from Y position.
        x = inOutObj->posX,
        y = inOutObj->posY;

    Calc_fillDeltas(inIota, PLAYER_STEP_LEN, &addX, &subY); // MT_TODO: TEST: Assertion "assert(inAngle>=0.0 && inAngle<Calc_PiMul2);" from Calc_fillDeltas() implementation can fail, here (very rare)!

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

static inline void fillPixel_floor(struct Mt3d const * const inObj, enum CellType const inCellType, double const inDx, double const inDy, int const inY, uint8_t * const inOutPix)
{
    double const imgX = inDx-(double)((int)inDx), // Removes integer part.
        imgY = inDy-(double)((int)inDy); // Removes integer part.
    int const row = (int)((double)inObj->bmpH[inCellType]*imgY),
        col = (int)((double)inObj->bmpH[inCellType]*imgX);

    unsigned char const * const channelZeroPtr = inObj->bmpPix[inCellType]+3*inObj->bmpW[inCellType]*row+3*col;

    inOutPix[0] = channelZeroPtr[0];
    inOutPix[1] = channelZeroPtr[1];
    inOutPix[2] = channelZeroPtr[2];

    switch(inCellType)
    {
        case CellType_floor_default:
            break;
        case CellType_floor_exit:
            if(getSecond(inObj)%2)
            {
                inOutPix[0] = ~inOutPix[0];
                inOutPix[1] = ~inOutPix[1];
                inOutPix[2] = ~inOutPix[2];
            }
            break;

        default:
            assert(false);
            break;
    }
}

static inline void fillPixel(struct Mt3d const * const inObj, enum CellType const inCellType, double const inDx, double const inDy, int const inY, uint8_t * const inOutPix)
{
    switch(inCellType)
    {
        case CellType_block_default:
            inOutPix[2] = 0xFF;
            inOutPix[1] = 0;
            inOutPix[0] = 0;
            break;
        case CellType_floor_default: // (falls through)
        case CellType_floor_exit:
            fillPixel_floor(inObj, inCellType, inDx, inDy, inY, inOutPix);
            break;

        default:
            assert(false);
            break;
    }
}

/** Set brightness based on map constants and line length from player.
 */
static inline void setBrightness(double const inCountLen, double const inMaxVisible, double const inMaxDarkness, unsigned char inOutPix[3])
{
    assert(inCountLen!=-1.0);

    double const brightness = (inMaxVisible-fmin(inCountLen, inMaxVisible))/inMaxVisible; // inCountLen 0 = 1.0, inCountLen maxVisible = 0.0;
    int const sub = (int)((inMaxDarkness*255.0)*(1.0-brightness)+0.5), // Rounds
        r = (int)inOutPix[2]-sub,
        g = (int)inOutPix[1]-sub,
        b = (int)inOutPix[0]-sub;

    inOutPix[2] = r>0?(unsigned char)r:0;
    inOutPix[1] = g>0?(unsigned char)g:0;
    inOutPix[0] = b>0?(unsigned char)b:0;
}

void Mt3d_update(struct Mt3d * const inOutObj)
{
    ++inOutObj->updateCount;
}

void Mt3d_draw(struct Mt3d * const inOutObj)
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

    if(inOutObj->doFill) // <=> Invalid.
    {
        inOutObj->doFill = false;
        fill(
            inOutObj->width,
            inOutObj->height,
            inOutObj->alpha,
            inOutObj->beta,
            inOutObj->theta,
            inOutObj->h,
            inOutObj->d,
            inOutObj->e,
            inOutObj->eta,
            inOutObj->hitType);
    }

    double const kPosY = (double)(inOutObj->map->height-1)-inOutObj->posY,
        hCellLengths = CEILING_HEIGHT*inOutObj->h; // Gets height of player's eye in cell lengths.

    for(int y = 0;y<inOutObj->height;++y)
    {
        int const truncPosX = (int)inOutObj->posX,
            truncPosY = (int)inOutObj->posY,
            rowByWidth = y*inOutObj->width;
        uint32_t * const rowPix = (uint32_t*)inOutObj->pixels+rowByWidth;

        for(int x = 0;x<inOutObj->width;++x)
        {
            double deltaX = -1.0,
                deltaY = -1.0,
                countLen = -1.0, // Means unset.
                dX = -1.0,
                dY = -1.0,
                zeta = -1.0;
            int cellX = truncPosX,
                cellY = truncPosY,
                dCellX = -1,
                dCellY = -1;

            int const pos = rowByWidth+x;
            bool const hitsFloorOrCeil = inOutObj->hitType[pos]!=HitType_none;

            // If hitsFloorOrCeil is NOT true, given object's d and e properties are invalid!

            {
                double const zetaUnchecked = inOutObj->eta[pos]+inOutObj->gamma; // (might be out of expected range, see usage below)

                zeta = CALC_ANGLE_TO_POS(zetaUnchecked);
            }

            if(hitsFloorOrCeil)
            {
                Calc_fillRotated(inOutObj->d[pos], 0.0, zeta, &deltaX, &deltaY);

                // Get coordinates of cell where the line/"ray" reaches either floor or ceiling:
                //
                dX = deltaX+inOutObj->posX;
                dY = (double)(inOutObj->map->height-1)-(deltaY+kPosY); // Cartesian Y to cell Y coordinate conversion.
                dCellX = (int)dX;
                dCellY = (int)dY;
            }
            else
            {
                Calc_fillRotated(1.0, 0.0, zeta, &deltaX, &deltaY);

                // dX, dY, dCellX and dCellY are invalid!
            }

            assert(deltaX!=0.0); // Implement special case!

            uint8_t * const colPix = (uint8_t*)(rowPix+x);

            if((cellX==dCellX)&&(cellY==dCellY)) // Line/"ray" hits floor or ceiling in current/player's cell.
            {
                assert(hitsFloorOrCeil);

                countLen = inOutObj->e[pos];
                fillPixel(inOutObj, (enum CellType)inOutObj->map->cells[cellY*inOutObj->map->width+cellX], dX, dY, y, colPix);
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
                double lastX = inOutObj->posX,
                    kLastY = kPosY; // (Cartesian Y coordinate)

                // Values to represent line in Slope-intercept form:
                //
                double const m = deltaY/deltaX, // Slope
                    b = kPosY-m*inOutObj->posX; // Y-intercept

                do
                {
                    double const dblYForHit = (double)yForHit,
                        dblXForHit = (double)xForHit,
                        hitX = (dblYForHit-b)/m,
                        hitY = m*dblXForHit+b;
                    bool const nextY = ( addX==1 && (int)hitX<xForHit ) || ( addX!=1 && hitX>=dblXForHit ),
                        nextX = ( addY==1 && (int)hitY<yForHit ) || ( addY!=1 && hitY>=dblYForHit );

                    assert(nextY||nextX);

                    // Debug code to ignore error and try to show frame:
                    //
//                    if(!(nextY||nextX))
//                    {
//                        colPix[0] = 255;
//                        colPix[1] = 255;
//                        colPix[2] = 255;
//
//                        countLen = 1.0;
//                        break;
//                    }

                    if(nextY)
                    {
                        // Update last reached coordinates:
                        //
                        lastX = hitX;
                        kLastY = dblYForHit;

                        cellY -= addY;
                        yForHit += addY;
                    }
                    if(nextX)
                    {
                        // Update last reached coordinates:
                        //
                        lastX = dblXForHit;
                        kLastY = hitY;

                        cellX += addX;
                        xForHit += addX;
                    }

                    assert(cellX>=0 && cellX<inOutObj->map->width);
                    assert(cellY>=0 && cellY<inOutObj->map->height);

                    enum CellType const cellType = (enum CellType)inOutObj->map->cells[cellY*inOutObj->map->width+cellX];

                    noHit = cellX!=dCellX || cellY!=dCellY; // noHit = false => Last cell is reached, where line/"ray" (at least theoretically) hits either floor or ceiling.

                    assert(noHit || hitsFloorOrCeil);

                    switch(cellType)
                    {
                        case CellType_block_default: // Line/"ray" hits block's surface.
                        {
                            double const diffY = kLastY-kPosY,
                                diffX = lastX-inOutObj->posX,
                                diffXY = sqrt(diffY*diffY+diffX*diffX);

                            noHit = false;
                            if(hitsFloorOrCeil)
                            {
                                countLen = diffXY*inOutObj->e[pos]/inOutObj->d[pos];
                            }
                            else
                            {
                                countLen = diffXY;
                            }

                            {
                                double const opposite = countLen-diffXY>0?sqrt(countLen*countLen-diffXY*diffXY):0; // Assuming less than 0 occurring for very small values only caused by rounding errors, where countLen and diffXY should be equal. See: http://stackoverflow.com/questions/4453372/sqrt1-0-pow1-0-2-returns-nan
                                double imgX = nextX?(double)(inOutObj->map->height-1)-kLastY:lastX, // (Cartesian Y to cell Y coordinate conversion, if necessary)
                                    imgY = opposite;

                                imgX -= (double)((int)imgX); // Removes integer part.
                                if(nextX)
                                {
                                    if(addX<0.0)
                                    {
                                        imgX = 1.0-imgX;
                                    }
                                }
                                else
                                {
                                    if(addY<0.0)
                                    {
                                        imgX = 1.0-imgX;
                                    }
                                }
                                assert(imgX>=0.0 && imgX<1.0);

                                switch(inOutObj->hitType[pos])
                                {
                                    case HitType_none:
                                        assert(!hitsFloorOrCeil);
                                        assert(hCellLengths-imgY == imgY+hCellLengths);
                                        imgY += hCellLengths;
                                        break;
                                    //
                                    case HitType_ceil:
                                        assert(hitsFloorOrCeil);
                                        imgY += hCellLengths;
                                        break;
                                    case HitType_floor:
                                        assert(hitsFloorOrCeil);
                                        imgY = hCellLengths-imgY;
                                        break;

                                    default:
                                        assert(false);
                                        break;
                                }

                                imgY = CEILING_HEIGHT-imgY;
                                assert(imgY>=0.0 && imgY<1.0);

                                {
                                    int const row = (int)((double)inOutObj->bmpH[cellType]*imgY),
                                        col = (int)((double)inOutObj->bmpH[cellType]*imgX);

                                    unsigned char const * const channelZeroPtr = inOutObj->bmpPix[cellType]+3*inOutObj->bmpW[cellType]*row+3*col;

                                    colPix[0] = channelZeroPtr[0];
                                    colPix[1] = channelZeroPtr[1];
                                    colPix[2] = channelZeroPtr[2];
                                }
                            }

                            break;
                        }

                        case CellType_floor_default: // (falls through).
                        case CellType_floor_exit:
                            if(!noHit) // Line/"ray" hits floor or ceiling.
                            {
                                assert(hitsFloorOrCeil);

                                countLen = inOutObj->e[pos];
                                fillPixel(inOutObj, cellType, dX, dY, y, colPix);
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
            setBrightness(countLen, inOutObj->map->maxVisible, inOutObj->map->maxDarkness, colPix);
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

    for(int i = 0;i<CellType_COUNT;++i)
    {
        free(inObj->bmpPix[i]);
    }

    free(inObj);
}

void Mt3d_setValues(double const inAlpha, double const inBeta, double const inTheta, double const inH, struct Mt3d * const inOutObj)
{
    inOutObj->doFill = true; // Triggers late fill() call by Mt3d_draw(), when needed.
    inOutObj->alpha = inAlpha;
    inOutObj->beta = inBeta;
    inOutObj->theta = inTheta;
    inOutObj->h = inH;

    //Deb_line("Alpha = %f degree, beta = %f degree, h(-eight) = %f cell length.", CALC_TO_DEG(inAlpha), CALC_TO_DEG(inBeta), inH)
}

struct Mt3d * Mt3d_create(int const inWidth, int const inHeight, double const inAlpha, double const inBeta, double const inTheta, double const inH, int const inMsPerUpdate)
{
    struct Mt3d * const retVal = malloc(sizeof *retVal);
    assert(retVal!=NULL);

    size_t const pixelCount = inHeight*inWidth;

    double * const d = malloc(pixelCount*sizeof *d);
    assert(d!=NULL);
    double * const e = malloc(pixelCount*sizeof *e);
    assert(e!=NULL);
    enum HitType * const hitType = malloc(pixelCount*sizeof *hitType);
    assert(hitType!=NULL);

    double * const eta = malloc(pixelCount*sizeof *eta);
    assert(eta!=NULL);

    struct Mt3d /*const*/ buf = (struct Mt3d)
    {
        .msPerUpdate = inMsPerUpdate,
        .updateCount = 0,

        .width = inWidth,
        .height = inHeight,
        .alpha = 0.0, // Invalidates
        .beta = 0.0, // Invalidates
        .theta = 0.0,
        .h = -1.0, // Invalidates

        .d = d,
        .e = e,
        .doFill = false,
        .hitType = hitType,

        .eta = eta,

        .posX = 0.0,
        .posY = 0.0,
        .gamma = 0.0,
        .map = NULL,
        .pixels = NULL,

//        .bmpPix
//        .bmpW
//        .bmpH
    };

    buf.bmpPix[CellType_floor_default] = Bmp_read("wood-320x320.bmp", &buf.bmpW[CellType_floor_default], &buf.bmpH[CellType_floor_default]);
    buf.bmpPix[CellType_block_default] = Bmp_read("brick-320x320.bmp", &buf.bmpW[CellType_block_default], &buf.bmpH[CellType_block_default]);
    buf.bmpPix[CellType_floor_exit] = Bmp_read("gradient-redblue-120x120.bmp", &buf.bmpW[CellType_floor_exit], &buf.bmpH[CellType_floor_exit]);

    memcpy(retVal, &buf, sizeof *retVal);

    Mt3d_setValues(inAlpha, inBeta, inTheta, inH, retVal);

    return retVal;
}
