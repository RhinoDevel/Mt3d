
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
    return (inObj->updateCount*(uint64_t)inObj->constants.msPerUpdate)/1000;
}

static void fill(
    struct Mt3dConstants const * const inC,
    struct Mt3dVariables const * const inV,
    double * const inOutD,
    double * const inOutE,
    double * const inOutEta,
    enum HitType * const inOutHitType,
    double * const inOutFloorToEye,
    double * const inOutCeilingToEye)
{
    assert(inV->alpha>0.0 && inV->alpha<M_PI);
    assert(inV->beta>0.0 && inV->beta<M_PI);
    assert(inV->theta>=0.0 && inV->theta<Calc_PiMul2);
    assert(inC->res.h%2==0);
    assert(inC->res.w%2==0);

    int x = 0,
        y = 0;

    double const dHeight = (double)inC->res.h,
        xMiddle = (double)(inC->res.w-1)/2.0,
        yMiddle = (dHeight-1.0)/2.0,
        sXmiddle = yMiddle/sin(inV->beta/2.0),
        sYmiddle = xMiddle/sin(inV->alpha/2.0),
        sXmiddleSqr = sXmiddle*sXmiddle,
        sYmiddleSqr = sYmiddle*sYmiddle;

    *inOutFloorToEye = inV->h*CEILING_HEIGHT; // (cell lengths)
    *inOutCeilingToEye = CEILING_HEIGHT-*inOutFloorToEye; // (cell lengths)

    for(y = 0;y<inC->res.h;++y)
    {
        int const rowPos = y*inC->res.w;
        double const cY = CALC_CARTESIAN_Y((double)y, dHeight);

        for(x = 0;x<inC->res.w;++x)
        {
            int const pos = rowPos+x;
            double xRot = -1.0,
                yRot = -1.0,
                betaTopX = 0.0,
                aX = 0.0;

            { // Calculate rotated x and y coordinates in pixel/bitmap coordinates (Y starts at top-left):
                double cYrot = -1.0;

                Calc_fillRotated(x-xMiddle, cY-yMiddle, inV->theta, &xRot, &cYrot);

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

                    inOutE[pos] = *inOutCeilingToEye/sin(angle);
                    inOutD[pos] = inOutE[pos]*cos(angle);
                }
                else
                {
                    inOutHitType[pos] = HitType_floor;

                    double const delta = betaTopX+atan((yRot-yMiddle)/aX);
                    assert(delta>betaTopX);

                    double const angle = delta-betaTopX;
                    assert(angle<M_PI_2);

                    inOutE[pos] = *inOutFloorToEye/sin(angle);
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
    double const x = inOutObj->posX+PLAYER_STEP_LEN*cos(inIota),
        y = inOutObj->posY-PLAYER_STEP_LEN*sin(inIota); // Subtraction, because cell coordinate system starts on top, Cartesian coordinate system at bottom.

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

static inline void fillPixel_block(
    struct Mt3d const * const inObj,
    enum CellType const inCellType,
    double const inCountLen,
    bool const inNextX,
    double const inLastX,
    double const inKlastY,
    int const inAddX,
    int const inAddY,
    int const inPos,
    uint8_t * const inOutPix)
{
    double imgX = inNextX?CALC_CARTESIAN_Y(inKlastY, (double)inObj->map->height):inLastX, // (Cartesian Y to cell Y coordinate conversion, if necessary)
        imgY = inObj->ceilingToEye; // Correct value, if ray does not hit anything / is a straight line.

    imgX -= (double)((int)imgX); // Removes integer part.
    if((inNextX && inAddX!=1)||(!inNextX && inAddY!=1))
    {
        imgX = 1.0-imgX;
    }
#ifndef NDEBUG
    if(!(imgX>=0.0 && imgX<1.0))
    {
        Deb_line("imgX = %f", imgX);
    }
#endif //NDEBUG
    assert(imgX>=0.0 && imgX<1.0); // MT_TODO: TEST: Happens sometimes with rotation!

    switch(inObj->hitType[inPos])
    {
        case HitType_none:
            break; // Nothing to do.
        case HitType_ceil:
            imgY -= inCountLen*inObj->ceilingToEye/inObj->e[inPos];
            break;
        case HitType_floor:
            imgY += inCountLen*inObj->floorToEye/inObj->e[inPos];
            break;

        default:
            assert(false);
            break;
    }
#ifndef NDEBUG
    if(!(imgY>=0.0 && imgY<1.0))
    {
        Deb_line("imgY = %f", imgY);
    }
#endif //NDEBUG
    assert(imgY>=0.0 && imgY<1.0); // MT_TODO: TEST: Happens sometimes with rotation!

    int const row = (int)((double)inObj->bmp[inCellType]->d.h*imgY),
        col = (int)((double)inObj->bmp[inCellType]->d.h*imgX);

    unsigned char const * const channelZeroPtr = inObj->bmp[inCellType]->p+3*inObj->bmp[inCellType]->d.w*row+3*col;

    inOutPix[0] = channelZeroPtr[0];
    inOutPix[1] = channelZeroPtr[1];
    inOutPix[2] = channelZeroPtr[2];
}

static inline void fillPixel_floor(struct Mt3d const * const inObj, enum CellType const inCellType, double const inDx, double const inDy, uint8_t * const inOutPix)
{
    double const imgX = inDx-(double)((int)inDx), // Removes integer part.
        imgY = inDy-(double)((int)inDy); // Removes integer part.
    int const row = (int)((double)inObj->bmp[inCellType]->d.h*imgY),
        col = (int)((double)inObj->bmp[inCellType]->d.h*imgX);

    unsigned char const * const channelZeroPtr = inObj->bmp[inCellType]->p+3*inObj->bmp[inCellType]->d.w*row+3*col;

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
            &inOutObj->constants,
            &inOutObj->variables,
            inOutObj->d,
            inOutObj->e,
            inOutObj->eta,
            inOutObj->hitType,
            &inOutObj->floorToEye,
            &inOutObj->ceilingToEye);
    }

    double const mapHeight = (double)inOutObj->map->height,
        kPosY = CALC_CARTESIAN_Y(inOutObj->posY, mapHeight);
    for(int y = 0;y<inOutObj->constants.res.h;++y)
    {
        int const truncPosX = (int)inOutObj->posX,
            truncPosY = (int)inOutObj->posY,
            rowByWidth = y*inOutObj->constants.res.w;
        uint32_t * const rowPix = (uint32_t*)inOutObj->pixels+rowByWidth;

        for(int x = 0;x<inOutObj->constants.res.w;++x)
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
            double const sinZeta = sin(zeta);

            deltaX = cos(zeta); // With parameter v in rotation matrix..
            deltaY = sinZeta; // ..formulas set to 1.0 [see Calc_fillRotated()].
            if(hitsFloorOrCeil)
            {
                deltaX *= inOutObj->d[pos]; // Using distance as parameter..
                deltaY *= inOutObj->d[pos]; // ..v of rotation matrix formula.

                // Get coordinates of cell where the line/"ray" reaches either floor or ceiling:
                //
                dX = deltaX+inOutObj->posX;
                dY = CALC_CARTESIAN_Y(deltaY+kPosY, mapHeight); // Cartesian Y to cell Y coordinate conversion.
                dCellX = (int)dX;
                dCellY = (int)dY;
            }
            //
            // Otherwise: dX, dY, dCellX and dCellY are invalid!

            assert(deltaX!=0.0); // Implement special case!

            uint8_t * const colPix = (uint8_t*)(rowPix+x);

            if(cellX==dCellX && cellY==dCellY) // Line/"ray" hits floor or ceiling in current/player's cell.
            {
                assert(hitsFloorOrCeil);

                countLen = inOutObj->e[pos];
                fillPixel_floor(inOutObj, (enum CellType)inOutObj->map->cells[cellY*inOutObj->map->width+cellX], dX, dY, colPix);
            }
            else
            {
                int const addX = CALC_SIGN_FROM_DOUBLE(deltaX),
                    addY = CALC_SIGN_FROM_DOUBLE(deltaY);
                double const m = deltaY/deltaX, // Slope
                    b = kPosY-m*inOutObj->posX, // Y-intercept
                    tanZeta = tan(zeta),
                    hitXstep = (double)addY/**1.0*//tanZeta, // 1.0 is cell length.
                    hitYstep = (double)addX/**1.0*/*tanZeta; // 1.0 is cell length.

                bool noHit = true;
                int xForHit = cellX+(int)(addX>0),
                    yForHit = (int)kPosY+(int)(addY>0); // (Cartesian Y coordinate)
                double lastX = inOutObj->posX, // Store last reached coordinates for
                    kLastY = kPosY,            // brightness (and other) calculations.
                    hitX = ((double)yForHit-b)/m,
                    hitY = m*(double)xForHit+b;

                do
                {
                    double const dblYForHit = (double)yForHit,
                        dblXForHit = (double)xForHit;

                    //hitX = (dblYForHit-b)/m,
                    //hitY = m*dblXForHit+b;

                    bool const nextY = ( addX==1 && (int)hitX<xForHit ) || ( addX!=1 && hitX>=dblXForHit ),
                        nextX = ( addY==1 && (int)hitY<yForHit ) || ( addY!=1 && hitY>=dblYForHit );

                    assert(nextY||nextX);

                    if(nextY)
                    {
                        // Update last reached coordinates:
                        //
                        lastX = hitX;
                        kLastY = dblYForHit;

                        cellY -= addY;
                        yForHit += addY;

                        hitX += hitXstep;
                    }
                    if(nextX)
                    {
                        // Update last reached coordinates:
                        //
                        kLastY = hitY;
                        lastX = dblXForHit;

                        cellX += addX;
                        xForHit += addX;

                        hitY += hitYstep;
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
                            noHit = false;

                            countLen = fabs((kLastY-kPosY)/sinZeta); // Equivalent to d.
                            if(hitsFloorOrCeil)
                            {
                                countLen *= inOutObj->e[pos]/inOutObj->d[pos]; // Equivalent to e.
                            }

                            fillPixel_block(
                                inOutObj,
                                cellType,
                                countLen,
                                nextX,
                                lastX,
                                kLastY,
                                addX,
                                addY,
                                pos,
                                colPix);
                            break;
                        }

                        case CellType_floor_default: // (falls through).
                        case CellType_floor_exit:
                            if(!noHit) // Line/"ray" hits floor or ceiling.
                            {
                                assert(hitsFloorOrCeil);

                                countLen = inOutObj->e[pos];
                                fillPixel_floor(inOutObj, cellType, dX, dY, colPix);
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
        Bmp_delete(inObj->bmp[i]);
    }

    free(inObj);
}

void Mt3d_setVariables(struct Mt3dVariables const * const inVariables, struct Mt3d * const inOutObj)
{
    assert(inVariables!=NULL);
    assert(inOutObj!=NULL);

    inOutObj->doFill = true; // Triggers late fill() call by Mt3d_draw(), when needed.
    inOutObj->variables = *inVariables;
}

struct Mt3d * Mt3d_create(struct Mt3dParams const * const inParams)
{
    assert(inParams!=NULL);

    struct Mt3d * const retVal = malloc(sizeof *retVal);
    assert(retVal!=NULL);

    size_t const pixelCount = inParams->constants.res.h*inParams->constants.res.w;

    double * const d = malloc(pixelCount*sizeof *d);
    assert(d!=NULL);
    double * const e = malloc(pixelCount*sizeof *e);
    assert(e!=NULL);
    enum HitType * const hitType = malloc(pixelCount*sizeof *hitType);
    assert(hitType!=NULL);

    double * const eta = malloc(pixelCount*sizeof *eta);
    assert(eta!=NULL);

    struct Mt3d buf = (struct Mt3d)
    {
        //.bmp

        .constants = inParams->constants,

        //.variables

        .floorToEye = -1.0, // Invalidates
        .ceilingToEye = -1.0, // Invalidates
        .d = d,
        .e = e,
        .hitType = hitType,
        .eta = eta,

        .doFill = false,
        .updateCount = 0,
        .posX = 0.0,
        .posY = 0.0,
        .gamma = 0.0,
        .map = NULL,
        .pixels = NULL,
    };

    buf.bmp[CellType_floor_default] = Bmp_load("wood-320x320.bmp");
    buf.bmp[CellType_block_default] = Bmp_load("brick-320x320.bmp");
    buf.bmp[CellType_floor_exit] = Bmp_load("gradient-redblue-120x120.bmp");

    memcpy(retVal, &buf, sizeof *retVal);

    Mt3d_setVariables(&inParams->variables, retVal);

    return retVal;
}
