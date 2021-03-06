
// MT, 2017jan14

#include <stdlib.h>
#include <assert.h>

#include "Calc.h"
#include "Mt3dParams.h"
#include "Mt3d.h"
#include "Mt3dInput.h"
#include "MapSample.h"
#include "Mt3dSingleton.h"
#include "SinSingleton.h"
#include "Deb.h"

static double const ALPHA = CALC_TO_RAD(60.0);
static double const ALPHA_MIN = CALC_TO_RAD(20.0);
static double const ALPHA_MAX = CALC_TO_RAD(160.0);
static double const ALPHA_STEP = CALC_TO_RAD(5.0);
static double const THETA_STEP = CALC_TO_RAD(5.0);
static double const H_STEP = 0.1; // In cell lengths.

static struct Mt3d * o = NULL;
static struct Mt3dInput * input = NULL;
static int width = 0;
static int height = 0;
static void (*toggleFullscreen)(void);
static void (*quit)(void);

static double getBeta(double const inAlpha)
{
    assert(width>0);
    assert(height>0);
    
    double const opposite = (double)height*LUT_SIN(inAlpha/2.0),
        hypotenuse = sqrt(opposite*opposite+width*width);
    
    return 2.0*LUT_ASIN(opposite/hypotenuse)/*atan((double)height*LUT_SIN(inAlpha/2.0)/(double)width)*/;
}

static bool pos_up()
{
    assert(o!=NULL);

    double const H_MAX = o->map->cells[(int)o->posY*o->map->width+(int)o->posX].height; // Also see Mt3d.posStep().
    
    assert(o->variables.playerEyeHeight<=H_MAX);
    
    double h = 0.0;

    if(o->variables.playerEyeHeight==H_MAX)
    {
        return false;
    }

    h = o->variables.playerEyeHeight+H_STEP;
    if(h>H_MAX)
    {
        h = H_MAX;
    }

    {
        struct Mt3dVariables v = (struct Mt3dVariables)
        {
            .alpha = o->variables.alpha,
            .beta = o->variables.beta,
            .theta = o->variables.theta,
            .playerEyeHeight = h
        };
        Mt3d_setVariables(&v, o);
    }
    return true;
}
static bool pos_down()
{
    assert(o!=NULL);

    static double const H_MIN = 0.0; // Also see Mt3d.posStep().
    
    double h = 0.0;

    if(o->variables.playerEyeHeight==H_MIN)
    {
        return false;
    }

    h = o->variables.playerEyeHeight-H_STEP;
    if(h<H_MIN)
    {
        h = H_MIN;
    }

    {
        struct Mt3dVariables v = (struct Mt3dVariables)
        {
            .alpha = o->variables.alpha,
            .beta = o->variables.beta,
            .theta = o->variables.theta,
            .playerEyeHeight = h
        };
        Mt3d_setVariables(&v, o);
    }
    return true;
}
static bool fov_wider()
{
    double alpha = 0.0;

    assert(o!=NULL);

    if(o->variables.alpha==ALPHA_MAX)
    {
        return false;
    }

    alpha = o->variables.alpha+ALPHA_STEP;
    if(alpha>ALPHA_MAX)
    {
        alpha = ALPHA_MAX;
    }

    {
        struct Mt3dVariables v = (struct Mt3dVariables)
        {
            .alpha = alpha,
            .beta = getBeta(alpha),
            .theta = o->variables.theta,
            .playerEyeHeight = o->variables.playerEyeHeight
        };
        Mt3d_setVariables(&v, o);
    }
    return true;
}
static bool fov_narrower()
{
    double alpha = 0.0;

    assert(o!=NULL);

    if(o->variables.alpha==ALPHA_MIN)
    {
        return false;
    }

    alpha = o->variables.alpha-ALPHA_STEP;
    if(alpha<ALPHA_MIN)
    {
        alpha = ALPHA_MIN;
    }

    {
        struct Mt3dVariables v = (struct Mt3dVariables)
        {
            .alpha = alpha,
            .beta = getBeta(alpha),
            .theta = o->variables.theta,
            .playerEyeHeight = o->variables.playerEyeHeight
        };
        Mt3d_setVariables(&v, o);
    }
    return true;
}
static bool rot_z_ccw()
{
    assert(o!=NULL);

    double theta = o->variables.theta;

    theta += THETA_STEP;
    if(theta>=Calc_PiMul2)
    {
        theta = 0.0;
    }

    {
        struct Mt3dVariables v = (struct Mt3dVariables)
        {
            .alpha = o->variables.alpha,
            .beta = o->variables.beta,
            .theta = theta,
            .playerEyeHeight = o->variables.playerEyeHeight
        };
        Mt3d_setVariables(&v, o);
    }
    return true;
}
static bool rot_z_cw()
{
    assert(o!=NULL);

    double theta = o->variables.theta;

    theta -= THETA_STEP;
    if(theta<0.0)
    {
        theta = Calc_PiMul2-THETA_STEP;
    }

    {
        struct Mt3dVariables v = (struct Mt3dVariables)
        {
            .alpha = o->variables.alpha,
            .beta = o->variables.beta,
            .theta = theta,
            .playerEyeHeight = o->variables.playerEyeHeight
        };
        Mt3d_setVariables(&v, o);
    }
    return true;
}

static void applyInput()
{
    assert(input!=NULL);
    assert(o!=NULL);
    assert(toggleFullscreen!=NULL);
    assert(quit!=NULL);

    if(input->quit)
    {
        quit();
        return;
    }

    if(input->toggleFullscreen)
    {
        toggleFullscreen();
        Mt3dInput_setFlagByChar('f', !input->toggleFullscreen, input); // (return value ignored) // MT_TODO: TEST: STUPID HARD-CODED WORKAROUND - IMPLEMENT CORRECTLY!
    }

    if(input->pos_left!=input->pos_right)
    {
        Mt3d_pos_leftOrRight(o, input->pos_left); // (return value ignored)
    }

    if(input->pos_forward!=input->pos_backward)
    {
        Mt3d_pos_forwardOrBackward(o, input->pos_forward); // (return value ignored)
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

    if(input->rot_z_ccw!=input->rot_z_cw)
    {
        if(input->rot_z_ccw)
        {
            rot_z_ccw(); // (return value ignored)
        }
        else
        {
            rot_z_cw(); // (return value ignored)
        }
    }
}

int Mt3dSingleton_getWidth()
{
    assert(o!=NULL);
    return o->constants.res.w;
}
int Mt3dSingleton_getHeight()
{
    assert(o!=NULL);
    return o->constants.res.h;
}
uint32_t * Mt3dSingleton_getPixels()
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

void Mt3dSingleton_init(int const inWidth, int const inHeight, int const inMsPerUpdate, void (* const inToggleFullscreen)(void), void (* const inQuit)(void))
{
    assert(width==0);
    assert(height==0);
    assert(o==NULL);
    assert(input==NULL);
    assert(toggleFullscreen==NULL);
    assert(quit==NULL);

    assert(inWidth>0);
    assert(inHeight>0);

    input = Mt3dInput_create();
    toggleFullscreen = inToggleFullscreen;
    quit = inQuit;

    width = inWidth;
    height = inHeight;

    {
        struct Mt3dParams * const params = malloc(sizeof *params);
        assert(params!=NULL);

        params->constants.msPerUpdate = inMsPerUpdate;
        params->constants.res =
            (struct Dim)
            {
                .w = width,
                .h = height
            };

        params->variables.alpha = ALPHA;
        params->variables.beta = getBeta(ALPHA);
        params->variables.theta = 0.0;
        params->variables.playerEyeHeight = 0.4; // MT_TODO: TEST: This must be read from Map object to-be-created, below!

        o = Mt3d_create(params);
        free(params);
    }

    o->map = MapSample_create();
    assert(sizeof *o->pixels==4);
    o->pixels = malloc(width*height*sizeof *o->pixels);
    assert(o->pixels!=NULL);
    o->posX = o->map->posX;
    o->posY = o->map->posY;
    o->gamma = o->map->gamma;

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
    assert(toggleFullscreen!=NULL);
    assert(quit!=NULL);

    Map_delete(o->map);
    o->map = NULL;
    free(o->pixels);
    o->pixels = NULL;
    Mt3d_delete(o);
    o = NULL;
    Mt3dInput_delete(input);
    input = NULL;
    toggleFullscreen = NULL;
    quit = NULL;
}
