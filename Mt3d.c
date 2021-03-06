
// MT, 2016aug22

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#ifndef __STDC_NO_THREADS__
    #include <threads.h>
#endif //__STDC_NO_THREADS__d

#include "Deb.h"
#include "Mt3d.h"
#include "Sys.h"
#include "Calc.h"
#include "Bmp.h"
#include "SinSingleton.h"

static double const PLAYER_STEP_LEN = 0.2; // Cell lengths.
static double const PLAYER_ANG_STEP = CALC_TO_RAD(5.0);

struct DrawInput
{
    struct Mt3d * o;
    int firstRow;
    int lastRow;
};

///** Return current second of game time.
// */
//static uint64_t getSecond(struct Mt3d const * const inObj)
//{
//    return (inObj->updateCount*(uint64_t)inObj->constants.msPerUpdate)/1000;
//}

static void fill(
    struct Mt3dConstants const * const inC,
    struct Mt3dVariables const * const inV,
    double * const inOutIota,
    double * const inOutEta)
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
        sXmiddle = yMiddle/LUT_SIN(inV->beta/2.0),
        sYmiddle = xMiddle/LUT_SIN(inV->alpha/2.0),
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

                betaTopX = LUT_ASIN(yMiddle/sX);
                assert(betaTopX>0.0 && betaTopX<M_PI_2);
                aX = yMiddle/Calc_tan(SinSingleton_sinLut, SinSingleton_len, betaTopX);
            }

            double const absOppositeY = fabs(yMiddle-yRot),
                aXsqr = aX*aX;
            
            if(absOppositeY==0.0)
            {
                inOutIota[pos] = 0.0; // HitType_none
            }
            else
            {
                double const hypotenuseY = sqrt(absOppositeY*absOppositeY+aXsqr),
                    partDelta = LUT_ASIN(absOppositeY/hypotenuseY);
                
                if(yRot<yMiddle)
                {
                    double const delta = betaTopX-partDelta/*atan((yMiddle-yRot)/aX)*/;
                    assert(delta<betaTopX);
                    
                    inOutIota[pos] = betaTopX-delta; // HitType_ceil
                    assert(inOutIota[pos]>0.0 && inOutIota[pos]<M_PI_2);
                }
                else
                {
                    double const delta = betaTopX+partDelta/*atan((yRot-yMiddle)/aX)*/;
                    assert(delta>betaTopX);

                    inOutIota[pos] = delta-betaTopX;
                    assert(inOutIota[pos]>0.0 && inOutIota[pos]<M_PI_2);
                    
                    inOutIota[pos] = -inOutIota[pos]; // HitType_floor
                }
            }

            {
                double const diff = yMiddle-yRot,
                    sY = sqrt(diff*diff+sYmiddleSqr),
                    alphaLeftY = LUT_ASIN(xMiddle/sY), // To hold alphaX/2.
                    absOppositeX = fabs(xMiddle-xRot),
                    hypotenuseX = sqrt(absOppositeX*absOppositeX+aXsqr),
                    partEpsilon = LUT_ASIN(absOppositeX/hypotenuseX);
                    
                double epsilon = 0.0;

                if(xRot<=xMiddle) // atan(0) equals 0 [asin(0) also], so it is OK, if equal.
                {
                    epsilon = alphaLeftY-partEpsilon/*atan((xMiddle-xRot)/aX)*/;
                }
                else
                {
                    epsilon = alphaLeftY+partEpsilon/*atan((xRot-xMiddle)/aX)*/;
                }

                inOutEta[pos] = alphaLeftY-epsilon; // Eta is a pre-calculated angle to be used with player view angle, later.
                //inOutEta[pos] = CALC_ANGLE_TO_POS(inOutEta[pos]); // Not necessary, here.
            }
        }
    }
}

static bool posStep(struct Mt3d * const inOutObj, double const inIota) // Iota: Complete angle in wanted direction (0 rad <= a < 2*PI rad).
{
    double const x = inOutObj->posX+PLAYER_STEP_LEN*Calc_cos(SinSingleton_sinLut, SinSingleton_len, inIota),
        y = inOutObj->posY-PLAYER_STEP_LEN*LUT_SIN(inIota); // Subtraction, because cell coordinate system starts on top, Cartesian coordinate system at bottom.
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
        uint32_t * const rowPix = input->o->pixels+rowByWidth;

        for(int x = 0;x<input->o->constants.res.w;++x)
        {
            int const pos = rowByWidth+x;
            uint32_t * const colPix = rowPix+x;
            enum HitType const hitType = input->o->iota[pos]==0.0?HitType_none:input->o->iota[pos]>0.0?HitType_ceil:HitType_floor;
            bool const hitsFloorOrCeil = hitType!=HitType_none;
            double const absIota = fabs(input->o->iota[pos]),
                zetaUnchecked = input->o->eta[pos]+input->o->gamma, // (might be out of expected range, but no problem - see usage below)
                deltaX = Calc_cos(SinSingleton_sinLut, SinSingleton_len, zetaUnchecked), // With parameter v in both rotation matrix..
                deltaY = LUT_SIN(zetaUnchecked); // ..formulas set to 1.0 [see Calc_fillRotated()].
            
            assert(deltaX!=0.0); // Implement special case!
            assert(deltaY!=0.0); // Implement special case!
            
            int const addX = CALC_SIGN_FROM_DOUBLE(deltaX),
                addY = CALC_SIGN_FROM_DOUBLE(deltaY);
            double const m = deltaY/deltaX, // Slope. This equals tan(zeta).
                b = kPosY-m*input->o->posX, // Y-intercept
                hitXstep = (double)addY/**1.0*//m, // 1.0 is cell length.
                hitYstep = (double)addX/**1.0*/*m; // 1.0 is cell length.
            
            double countLen = -1.0, // Means unset.
                relBmpX = -1.0,
                relBmpY = -1.0;
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

                    if(hitType==HitType_ceil)
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
                    vEyeToExit = Calc_tan(SinSingleton_sinLut, SinSingleton_len, absIota)*hEyeToExit;
                    assert(vEyeToExit>0.0);
                    
                    if(vEyeToExit>=vEyeToFloorOrCeil)
                    { // => Line/"ray" hits floor/ceiling of current cell.
                        countLen = vEyeToFloorOrCeil/LUT_SIN(absIota);
                        assert(countLen>=0.0);
                       
                        double const d = countLen*Calc_cos(SinSingleton_sinLut, SinSingleton_len, absIota),
                            dX = d*deltaX+input->o->posX, // Using distance as parameter v of rotation matrix formula by multiplying deltaX with d.
                            dY = CALC_CARTESIAN_Y(d*deltaY+kPosY, mapHeight); // Cartesian Y to cell Y coordinate conversion. // Using distance as parameter v of rotation matrix formula by multiplying deltaY with d.

                        relBmpX = dX-(double)((int)dX); // Removes integer part.
                        relBmpY = dY-(double)((int)dY); // Removes integer part.
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
                    double const heightForHit = fullEyeHeight+(hitType==HitType_floor?-vEyeToExit:vEyeToExit);
                    bool floorHit = false;

                    if(isBlock || (floorHit = heightForHit<cell->floor) || cell->floor+cell->height<heightForHit)
                    { // Yes, it is getting hit!
                        countLen = hitsFloorOrCeil?vEyeToExit/LUT_SIN(absIota):hEyeToExit;
                        assert(countLen>=0.0);
                        
                        // **************************
                        // *** FILL PIXEL "BLOCK" *** Start
                        // **************************

                        static double const IMG_HEIGHT = 1.0, // In cell lengths.
                            IMG_WIDTH = 1.0; // In cell lengths.
                        
                        relBmpX = nextX?CALC_CARTESIAN_Y(kLastY, mapHeight):lastX; // (Cartesian Y to cell Y coordinate conversion, if necessary)
                        relBmpY = isBlock
                             ? heightForHit // Block [always start counting whole images at 0 (bottom)].
                             : floorHit
                               ? cell->floor-heightForHit // Floor hit. Start counting whole images at cell floor (top).
                               : heightForHit-cell->floor-cell->height; // Ceiling hit. Start counting whole images at cell ceiling (bottom).

                        relBmpX -= (double)(int)relBmpX; // Removes integer part.
                        assert(relBmpX>=0.0 && relBmpX<IMG_WIDTH);
                        if((nextX && addX!=1)||(!nextX && addY!=1))
                        {
                            relBmpX = IMG_WIDTH-relBmpX;
                        }

                        relBmpY = relBmpY-(double)((int)relBmpY/IMG_HEIGHT); // Removes count of whole images fitting in and sets relBmpY to fraction.
                        assert(relBmpY>=0.0 && relBmpY<IMG_HEIGHT);
                        if(!floorHit)
                        {
                            relBmpY = IMG_HEIGHT-relBmpY; // Correct for block or ceiling hit.
                        }  

                        // **************************
                        // *** FILL PIXEL "BLOCK" *** End
                        // **************************
                        
                        break;
                    }
                }
            }while(true);
            
            assert(countLen>=0.0);
            
            // ******************
            // *** FILL PIXEL *** Start
            // ******************

            assert(relBmpX>=0.0 && relBmpX<1.0);
            assert(relBmpY>=0.0 && relBmpY<1.0);

            struct Bmp const * const bmp = input->o->bmp[(int)cell->type];
            uint8_t const * const bmpChannel = bmp->p
                                          + 3*bmp->d.w*(int)((double)bmp->d.h*relBmpY) // Hard-coded for 24 bits per pixel. Truncates.
                                          + 3*(int)((double)bmp->d.h*relBmpX); // Hard-coded for 24 bits per pixel. Truncates.

            assert(!Sys_is_big_endian());
            *colPix = 0xFF000000ul+((uint32_t)bmpChannel[2]<<16)+((uint32_t)bmpChannel[1]<<8)+(uint32_t)bmpChannel[0];

            // ******************
            // *** FILL PIXEL *** End
            // ******************

            // ***********************
            // *** Set brightness: ***
            // ***********************
            
            uint8_t * const channel = (uint8_t*)colPix;
            double const brightness = (input->o->map->maxVisible-fmin(countLen, input->o->map->maxVisible))/input->o->map->maxVisible; // countLen 0 = 1.0, countLen maxVisible = 0.0;
            int const sub = (int)(input->o->map->maxDarkness*255.0*(1.0-brightness)+0.5), // Rounds
                red = (int)channel[2]-sub,
                green = (int)channel[1]-sub,
                blue = (int)channel[0]-sub;

            channel[2] = (uint8_t)((int)(red>0)*red);//red>0?(uint8_t)red:0;
            channel[1] = (uint8_t)((int)(green>0)*green);//green>0?(uint8_t)green:0;
            channel[0] = (uint8_t)((int)(blue>0)*blue);//blue>0?(uint8_t)blue:0;
        }
    }
    
    // Debugging: Get and show distances (it's funny!):
    //
//    # double * const distances = malloc(input->o->constants.res.h*input->o->constants.res.w*sizeof *distances);
//    # double maxDistance = 0.0,
//    #     minDistance = DBL_MAX;
//
//    # distances[pos] = countLen;
//
//    for(int d = 0;d<input->o->constants.res.h*input->o->constants.res.w;++d)
//    {
//        if(distances[d]>maxDistance)
//        {
//            maxDistance = distances[d];
//        }
//        if(distances[d]<minDistance)
//        {
//            minDistance = distances[d];
//        }
//    }
//    maxDistance -= minDistance;
//    for(int y = input->firstRow;y<=input->lastRow;++y)
//    {
//        int const rowByWidth = y*input->o->constants.res.w;
//        uint32_t * const rowPix = input->o->pixels+rowByWidth;
//
//        for(int x = 0;x<input->o->constants.res.w;++x)
//        {   
//            int const pos = rowByWidth+x;
//            uint32_t * const colPix = rowPix+x;
//            
//            uint8_t const val = /*255.0-*/(uint8_t)(255.0*(distances[pos]-minDistance)/maxDistance);
//            
//            *colPix = 0xFF000000ul+((uint32_t)val<<16)+((uint32_t)val<<8)+(uint32_t)val;
//        }
//    }
//    free(distances);
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
            inOutObj->eta);
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
    
    double * const eta = malloc(pixelCount*sizeof *eta);
    assert(eta!=NULL);

    struct Mt3d buf = (struct Mt3d)
    {
        //.bmp

        .constants = inParams->constants,

        //.variables

        .iota = iota,
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
