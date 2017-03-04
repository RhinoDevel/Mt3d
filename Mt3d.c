
// MT, 2016aug22

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#ifndef __STDC_NO_THREADS__
    #include <threads.h>
#endif //__STDC_NO_THREADS__d

#include "Deb.h"
#include "Mt3d.h"
#include "Sys.h"
#include "Calc.h"
#include "Bmp.h"

static double const PLAYER_STEP_LEN = 0.2; // Cell lengths.
static double const PLAYER_ANG_STEP = CALC_TO_RAD(5.0);

struct DrawInput
{
    struct Mt3d * o;
    int firstRow;
    int lastRow;
};

/** Return current second of game time.
 */
static uint64_t getSecond(struct Mt3d const * const inObj)
{
    return (inObj->updateCount*(uint64_t)inObj->constants.msPerUpdate)/1000;
}

static void fill(
    struct Mt3dConstants const * const inC,
    struct Mt3dVariables const * const inV,
    double * const inOutIota,
    double * const inOutEta,
    enum HitType * const inOutHitType)
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
                inOutIota[pos] = 0.0;
            }
            else
            {
                if(yRot<yMiddle)
                {
                    inOutHitType[pos] = HitType_ceil;

                    double const delta = betaTopX-atan((yMiddle-yRot)/aX);
                    assert(delta<betaTopX);

                    inOutIota[pos] = betaTopX-delta;
                    assert(inOutIota[pos]<M_PI_2);
                }
                else
                {
                    inOutHitType[pos] = HitType_floor;

                    double const delta = betaTopX+atan((yRot-yMiddle)/aX);
                    assert(delta>betaTopX);

                    inOutIota[pos] = delta-betaTopX;
                    assert(inOutIota[pos]<M_PI_2);
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
    struct Cell const * const cell = inOutObj->map->cells+(int)y*inOutObj->map->width+(int)x;
    
    if(cell->type==CellType_block_default) // MT_TODO: TEST: Player has no width!
    {
        return false;
    }
    if(cell->height<inOutObj->variables.playerEyeHeight) // MT_TODO: TEST: Player has no head (above the eyes..)!
    {
        return false;
    }
    // MT_TODO: TEST: Player can climb every height!
    
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

static inline void fillPixel(struct Bmp * const inBitmaps[CellType_COUNT], int const inBmpIndex, double const inImgX, double const inImgY, uint8_t * const inOutPix)
{
    assert(inImgX>=0.0 && inImgX<1.0);
    assert(inImgY>=0.0 && inImgY<1.0);

    struct Bmp const * const bmp = inBitmaps[inBmpIndex];
    int const row = (int)((double)bmp->d.h*inImgY), // Truncates
        col = (int)((double)bmp->d.h*inImgX); // Truncates
    unsigned char const * const channelZeroPtr = bmp->p+3*bmp->d.w*row+3*col;

    inOutPix[0] = channelZeroPtr[0];
    inOutPix[1] = channelZeroPtr[1];
    inOutPix[2] = channelZeroPtr[2];
}

static inline void fillPixel_floor(struct Mt3d const * const inObj, enum CellType const inCellType, double const inDx, double const inDy, uint8_t * const inOutPix)
{
    fillPixel(
        inObj->bmp,
        (int)inCellType,
        inDx-(double)((int)inDx), // Removes integer part.
        inDy-(double)((int)inDy), // Removes integer part.
        inOutPix);

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

static void draw(void * inOut)
{
    struct DrawInput const * const input = (struct DrawInput const *)inOut;
    int const truncPosX = (int)input->o->posX,
            truncPosY = (int)input->o->posY;
    struct Cell const * const playerCell = input->o->map->cells+truncPosY*input->o->map->width+truncPosX;
    double const mapHeight = (double)input->o->map->height,
        kPosY = CALC_CARTESIAN_Y(input->o->posY, mapHeight),
        fullEyeHeight = playerCell->floor+input->o->variables.playerEyeHeight;
    
    for(int y = input->firstRow;y<=input->lastRow;++y)
    {
        int const rowByWidth = y*input->o->constants.res.w;
        uint32_t * const rowPix = (uint32_t*)input->o->pixels+rowByWidth;

        for(int x = 0;x<input->o->constants.res.w;++x)
        {
            int const pos = rowByWidth+x;
            uint8_t * const colPix = (uint8_t*)(rowPix+x);
            bool const hitsFloorOrCeil = input->o->hitType[pos]!=HitType_none;
            double const zetaUnchecked = input->o->eta[pos]+input->o->gamma, // (might be out of expected range, but no problem - see usage below)
                deltaX = cos(zetaUnchecked), // With parameter v in both rotation matrix..
                deltaY = sin(zetaUnchecked); // ..formulas set to 1.0 [see Calc_fillRotated()].
            
            assert(deltaX!=0.0); // Implement special case!
            assert(deltaY!=0.0); // Implement special case!
            
            int const addX = CALC_SIGN_FROM_DOUBLE(deltaX),
                addY = CALC_SIGN_FROM_DOUBLE(deltaY);
            double const m = deltaY/deltaX, // Slope. This equals tan(zeta).
                b = kPosY-m*input->o->posX, // Y-intercept
                hitXstep = (double)addY/**1.0*//m, // 1.0 is cell length.
                hitYstep = (double)addX/**1.0*/*m; // 1.0 is cell length.
            
            double countLen = -1.0; // Means unset.
            int cellX = truncPosX,
                cellY = truncPosY,
                xForHit = cellX+(int)(addX>0),
                yForHit = (int)kPosY+(int)(addY>0); // (Cartesian Y coordinate)
            double lastX = input->o->posX, // Store last reached coordinates for
                kLastY = kPosY,            // brightness (and other) calculations.
                hitX = ((double)yForHit-b)/m,
                hitY = m*(double)xForHit+b;
            struct Cell const * cell = input->o->map->cells+cellY*input->o->map->width+cellX;
            bool isBlock = cell->type==CellType_block_default;

            do
            {
                double const dblYForHit = (double)yForHit,
                    dblXForHit = (double)xForHit;
                bool const nextY = ( addX==1 && (int)hitX<xForHit ) || ( addX!=1 && hitX>=dblXForHit ),
                    nextX = ( addY==1 && (int)hitY<yForHit ) || ( addY!=1 && hitY>=dblYForHit );
                
                double hEyeToExit = 0.0, // Side view: To hold horizontal distance from player's eye to where line/"ray" leaves the current cell.
                    vEyeToExit = 0.0;

                assert(!isBlock);
                assert(nextX||nextY);
                
                // Using top view to calculate this (where hEyeToExit is a hypotenuse):
                //
                if(nextX)
                {
                    hEyeToExit = (dblXForHit-input->o->posX)/deltaX; // deltaX equals cos(zeta).
                }
                else
                {
                    assert(nextY);
                    hEyeToExit = (dblYForHit-kPosY)/deltaY;
                }
                hEyeToExit = fabs(hEyeToExit);
                
                // If this is not a straight horizontal line/"ray", check first,
                // if current cell's floor/ceiling will be hit by it:
                //
                if(hitsFloorOrCeil)
                {
                    double vEyeToFloorOrCeil = 0.0; // Side view: To hold vertical distance from player's eye to current cell's floor or ceiling.

                    if(input->o->hitType[pos]==HitType_ceil)
                    {
                        vEyeToFloorOrCeil = cell->floor+cell->height-fullEyeHeight;
                    }
                    else
                    {
                        vEyeToFloorOrCeil = fullEyeHeight-cell->floor;
                    }
                    vEyeToFloorOrCeil = fabs(vEyeToFloorOrCeil);
                    
                    // Side view: Vertical distance from eye to where the line/"ray" is at current cell's border
                    //            (it may be above, below or at floor/ceiling):
                    //
                    vEyeToExit = tan(input->o->iota[pos])*hEyeToExit;
                    
                    assert(vEyeToExit>0.0);
                    
                    if(vEyeToExit>=vEyeToFloorOrCeil)
                    { // => Line/"ray" hits floor/ceiling of current cell.
                        countLen = vEyeToFloorOrCeil/sin(input->o->iota[pos]);
                       
                        double const d = countLen*cos(input->o->iota[pos]),
                            dX = d*deltaX+input->o->posX, // Using distance as parameter v of rotation matrix formula by multiplying deltaX with d.
                            dY = CALC_CARTESIAN_Y(d*deltaY+kPosY, mapHeight); // Cartesian Y to cell Y coordinate conversion. // Using distance as parameter v of rotation matrix formula by multiplying deltaY with d.
                        
                        fillPixel_floor(input->o, cell->type, dX, dY, colPix);
                        break;
                    } // Otherwise: Line/"ray" travels to next cell.
                }

                // As current cell is not a block and floor/ceiling was not hit by line/"ray",
                // advance to next cell reached by line/"ray":
                //
                if(nextY)
                {
                    lastX = hitX; // Update last..
                    kLastY = dblYForHit; // ..reached coordinates.
                    cellY -= addY;
                    assert(cellY>=0 && cellY<input->o->map->height);
                    yForHit += addY;
                    hitX += hitXstep;
                }
                if(nextX)
                { // (OK, if nextY is also true)
                    kLastY = hitY; // Update last..
                    lastX = dblXForHit; // ..reached coordinates.
                    cellX += addX;
                    assert(cellX>=0 && cellX<input->o->map->width);
                    xForHit += addX;
                    hitY += hitYstep;
                }
                cell = input->o->map->cells+cellY*input->o->map->width+cellX;
                isBlock = cell->type==CellType_block_default;
                
                // Will the NEXT cell's border get hit?
                //
                {
                    double const heightForHit = fullEyeHeight+(input->o->hitType[pos]==HitType_floor?-vEyeToExit:vEyeToExit);
                    bool floorHit = false;

                    if(isBlock || (floorHit = heightForHit<cell->floor) || heightForHit>=cell->floor+cell->height)
                    { // Yes, it is getting hit!
                        // **************************
                        // *** FILL PIXEL "BLOCK" *** Start
                        // **************************

                        static double const IMG_HEIGHT = 1.0, // In cell lengths.
                            IMG_WIDTH = 1.0; // In cell lengths.
                        double imgX = nextX?CALC_CARTESIAN_Y(kLastY, mapHeight):lastX, // (Cartesian Y to cell Y coordinate conversion, if necessary)
                            imgY = isBlock
                                 ? heightForHit // Block [always start counting whole images at 0 (bottom)].
                                 : floorHit
                                    ?cell->floor-heightForHit // Floor hit. Start counting whole images at cell floor (top).
                                    :heightForHit-cell->floor-cell->height; // Ceiling hit. Start counting whole images at cell ceiling (bottom).

                        imgX -= (double)(int)imgX; // Removes integer part.
                        assert(imgX>=0.0 && imgX<IMG_WIDTH);
                        if((nextX && addX!=1)||(!nextX && addY!=1))
                        {
                            imgX = IMG_WIDTH-imgX;
                        }

                        imgY = imgY-(double)((int)imgY/IMG_HEIGHT); // Removes count of whole images fitting in and sets imgY to fraction.
                        assert(imgY>=0.0 && imgY<IMG_HEIGHT);
                        if(!floorHit)
                        {
                            imgY = IMG_HEIGHT-imgY; // Correct for block or ceiling hit.
                        }  

                        fillPixel(input->o->bmp, (int)cell->type, imgX, imgY, colPix);

                        // **************************
                        // *** FILL PIXEL "BLOCK" *** End
                        // **************************

                        countLen = hitsFloorOrCeil?vEyeToExit/sin(input->o->iota[pos]):hEyeToExit;
                        break;
                    }
                }
            }while(true);
            assert(countLen>=0.0);

            // ***********************
            // *** Set brightness: ***
            // ***********************
            
            double const brightness = (input->o->map->maxVisible-fmin(countLen, input->o->map->maxVisible))/input->o->map->maxVisible; // countLen 0 = 1.0, countLen maxVisible = 0.0;
            int const sub = (int)(input->o->map->maxDarkness*255.0*(1.0-brightness)+0.5), // Rounds
                red = (int)colPix[2]-sub,
                green = (int)colPix[1]-sub,
                blue = (int)colPix[0]-sub;

            colPix[2] = (uint8_t)((int)(red>0)*red);//red>0?(uint8_t)red:0;
            colPix[1] = (uint8_t)((int)(green>0)*green);//green>0?(uint8_t)green:0;
            colPix[0] = (uint8_t)((int)(blue>0)*blue);//blue>0?(uint8_t)blue:0;
        }
    }
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
            inOutObj->iota,
            inOutObj->eta,
            inOutObj->hitType);
    }

#ifdef __STDC_NO_THREADS__
    {
        struct DrawInput input = {
            .o = inOutObj,
            .firstRow = 0,
            .lastRow = inOutObj->constants.res.h-1
        };

        draw(&input);
    }
#else //__STDC_NO_THREADS__
    {
        assert(false); // MT_TODO: TEST: Set .firstRow & .lastRow values correctly, below and make sure that thread count makes sense!
        
        thrd_t threadId[4];
        int i = 0;
        struct DrawInput input[4];

        for(i = 0;i<4;++i)
        {   
            input[i] = {
                .o = inOutObj,
                .firstRow = 0,
                .lastRow = inOutObj->constants.res.h-1
            };
            
            thrd_create(threadId+i, threadFunction, input+i);
        }

        for(int i = 0;i<4;++i)
        {
            thrd_join(threadId[i], NULL);
        }
    }
#endif //__STDC_NO_THREADS__
}

void Mt3d_delete(struct Mt3d * const inObj)
{
    assert(inObj!=NULL);

    assert(inObj->map==NULL); // No ownership of map.
    assert(inObj->pixels==NULL); // No ownership of pixels.

    assert(inObj->iota!=NULL);
    free((double*)inObj->iota);

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

    if(!inOutObj->doFill) // Could be forced from outside of function.
    {
        inOutObj->doFill = true;
        
        do
        {
            if(inOutObj->variables.alpha!=inVariables->alpha)
            {
                break; // Trigger fill.
            }
            if(inOutObj->variables.beta!=inVariables->beta)
            {
                break; // Trigger fill.
            }
            if(inOutObj->variables.theta!=inVariables->theta)
            {
                break; // Trigger fill.
            }
            
            //inOutObj->variables.h
            
            inOutObj->doFill = false;
        }while(false);
    }
    
    inOutObj->variables = *inVariables;
}

struct Mt3d * Mt3d_create(struct Mt3dParams const * const inParams)
{
    assert(inParams!=NULL);

    struct Mt3d * const retVal = malloc(sizeof *retVal);
    assert(retVal!=NULL);

    size_t const pixelCount = inParams->constants.res.h*inParams->constants.res.w;

    double * const iota = malloc(pixelCount*sizeof *iota);
    assert(iota!=NULL);
    enum HitType * const hitType = malloc(pixelCount*sizeof *hitType);
    assert(hitType!=NULL);

    double * const eta = malloc(pixelCount*sizeof *eta);
    assert(eta!=NULL);

    struct Mt3d buf = (struct Mt3d)
    {
        //.bmp

        .constants = inParams->constants,

        //.variables

        .iota = iota,
        .hitType = hitType,
        .eta = eta,

        .doFill = true, // => Forces fill on next render run [see Mt3d_setVariables()].
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
