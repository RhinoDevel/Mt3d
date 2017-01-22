
// MT, 2017jan14

#include <stdlib.h>
#include <assert.h>

#include "Calc.h"
#include "Mt3d.h"
#include "Mt3dInput.h"
#include "MapSample.h"
#include "Mt3dSingleton.h"

static double const ALPHA = CALC_TO_RAD(45.0);
static double const ALPHA_MIN = CALC_TO_RAD(20.0);
static double const ALPHA_MAX = CALC_TO_RAD(160.0);
static double const ALPHA_STEP = CALC_TO_RAD(5.0);
static double const H = 0.3; // As part of room height (e.g. 0.5 = 50% of room height).
static double const H_MIN = 0.1;
static double const H_MAX = 0.9;
static double const H_STEP = 0.1;

static struct Mt3d * o = NULL;
static struct Mt3dInput * input = NULL;
static int width = 0;
static int height = 0;

static double getBeta(double const inAlpha)
{
    assert(width>0);
    assert(height>0);
    
    return 2.0*atan((double)height*tan(inAlpha/2.0)/(double)width);
}

static bool pos_up()
{
    assert(o!=NULL);
    
    double h = 0.0;
    
    if(o->h==H_MAX)
    {
        return false;
    }
    
    h = o->h+H_STEP;
    if(h>H_MAX)
    {
        h = H_MAX;
    }
    Mt3d_setValues(o->alpha, o->beta, h, o);
    return true;
}
static bool pos_down()
{
    assert(o!=NULL);
    
    double h = 0.0;
    
    if(o->h==H_MIN)
    {
        return false;
    }
    
    h = o->h-H_STEP;
    if(h<H_MIN)
    {
        h = H_MIN;
    }
    Mt3d_setValues(o->alpha, o->beta, h, o);
    return true;
}
static bool fov_wider()
{
    double alpha = 0.0;
    
    assert(o!=NULL);
    
    if(o->alpha==ALPHA_MAX)
    {
        return false;
    }
    
    alpha = o->alpha+ALPHA_STEP;      
    if(alpha>ALPHA_MAX)
    {
        alpha = ALPHA_MAX;
    }
    Mt3d_setValues(alpha, getBeta(alpha), o->h, o);
    return true;
}
static bool fov_narrower()
{
    double alpha = 0.0;
    
    assert(o!=NULL);
    
    if(o->alpha==ALPHA_MIN)
    {
        return false;
    }
    
    alpha = o->alpha-ALPHA_STEP;      
    if(alpha<ALPHA_MIN)
    {
        alpha = ALPHA_MIN;
    }
    Mt3d_setValues(alpha, getBeta(alpha), o->h, o);
    return true;
}

static void applyInput()
{
    assert(input!=NULL);
    assert(o!=NULL);
    
    if(input->pos_left!=input->pos_right)
    {
        Mt3d_pos_leftOrRight(o, input->pos_left); // (return value ignored)
    }
    
    if(input->pos_forward!=input->pos_backward)
    {
        Mt3d_pos_forwardOrBackward(o,  input->pos_forward); // (return value ignored)
    }
   
    if(input->ang_left!=input->ang_right)
    {
        Mt3d_ang_leftOrRight(o, input->ang_left); // (return value ignored)
    }
    
    if(input->pos_up!=input->pos_down)
    {
        if(input->pos_up)
        {
            pos_up(); // (return value ignored)
        }
        else
        {
            pos_down(); // (return value ignored)
        }
    }
    
    if(input->fov_wider!=input->fov_narrower)
    {
        if(input->fov_wider)
        {
            fov_wider(); // (return value ignored)
        }
        else
        {
            fov_narrower(); // (return value ignored)
        }
    }
}

int Mt3dSingleton_getWidth()
{
    assert(o!=NULL);
    return o->width;
}
int Mt3dSingleton_getHeight()
{
    assert(o!=NULL);
    return o->height;
}
unsigned char * Mt3dSingleton_getPixels()
{
    assert(o!=NULL);
    assert(o->pixels!=NULL);
    return o->pixels;
}

void Mt3dSingleton_input_onKeyPress(char const inChar)
{
    Mt3dInput_setFlagByChar(inChar, true, input); // (return value ignored)
}
void Mt3dSingleton_input_onKeyRelease(char const inChar)
{    
    Mt3dInput_setFlagByChar(inChar, false, input); // (return value ignored)
}

void Mt3dSingleton_update()
{
    assert(o!=NULL);
    applyInput();
    Mt3d_update(o);
}

void Mt3dSingleton_draw()
{
    assert(o!=NULL);
    Mt3d_draw(o);
}

void Mt3dSingleton_init(int const inWidth, int const inHeight, int const inMsPerUpdate)
{
    assert(width==0);
    assert(height==0);
    assert(o==NULL);
    assert(input==NULL);
    
    assert(inWidth>0);
    assert(inHeight>0);
    
    input = Mt3dInput_create();
    
    width = inWidth;
    height = inHeight;
    o = Mt3d_create(inWidth, height, ALPHA, getBeta(ALPHA), H, inMsPerUpdate);
    
    o->map = MapSample_create();
    assert(sizeof *o->pixels==1);
    o->pixels = malloc(width*height*4*sizeof *o->pixels);
    assert(o->pixels!=NULL);
    o->posX = o->map->posX;
    o->posY = o->map->posY;
    o->gamma = o->map->gamma;

//    for(int row = 0, col = 0;row<height;++row)
//    {
//        uint32_t * const rowPix = ((uint32_t*)o->pixels)+row*width;
//
//        for(col = 0;col<width;++col)
//        {
//            uint8_t * const colPix = (uint8_t*)(rowPix+col);
//
//            colPix[0] = 0xFF; // Blue
//            colPix[1] = 0x0; // Green
//            colPix[2] = 0xFF; // Red
//            //colPix[3] = 0xFF; // (unused)
//        }
//    }
    
    Mt3dSingleton_draw();
}
void Mt3dSingleton_deinit()
{
    assert(width>0);
    assert(height>0);
    assert(o!=NULL);
    assert(o->map!=NULL);
    assert(o->pixels!=NULL);
    assert(input!=NULL);
    
    Map_delete(o->map);
    o->map = NULL;
    free(o->pixels);
    o->pixels = NULL;
    Mt3d_delete(o);
    o = NULL;
    Mt3dInput_delete(input);
    input = NULL;
}
