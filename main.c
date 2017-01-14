
// MT, 2016aug22

//NDEBUG

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "Sys.h"
#include "Deb.h"
#include "Mt3dSingleton.h"
#include "GuiSingleton_cairo.h"

static int const WIDTH = 320;
static int const HEIGHT = 240;
static double const SCALE_FACTOR = 2.0;

static bool ang_left = false;
static bool ang_right = false;
static bool pos_forward = false;
static bool pos_backward = false;
static bool pos_left = false;
static bool pos_right = false;
static bool pos_up = false;
static bool pos_down = false;
static bool fov_wider = false;
static bool fov_narrower = false;
static int const GAME_LOOP_INTERVAL = 40; // 1000/25 ms.
//
static void update()
{
    if(pos_left!=pos_right)
    {
        if(pos_left)
        {
            Mt3dSingleton_pos_left();
        }
        else
        {
            Mt3dSingleton_pos_right();
        }
    }
    
    if(pos_forward!=pos_backward)
    {
        if(pos_forward)
        {
            Mt3dSingleton_pos_forward();
        }
        else
        {
            Mt3dSingleton_pos_backward();
        }
    }
   
    if(ang_left!=ang_right)
    {
        if(ang_left)
        {
            Mt3dSingleton_ang_left();
        }
        else
        {
            Mt3dSingleton_ang_right();
        }
    }
    
    if(pos_up!=pos_down)
    {
        if(pos_up)
        {
            Mt3dSingleton_pos_up();
        }
        else
        {
            Mt3dSingleton_pos_down();
        }
    }
    
    if(fov_wider!=fov_narrower)
    {
        if(fov_wider)
        {
            Mt3dSingleton_fov_wider();
        }
        else
        {
            Mt3dSingleton_fov_narrower();
        }
    }
}
//
void render(double const inLag)
{
    Mt3dSingleton_draw();
    GuiSingleton_cairo_draw();
}
//
/**
 * - See: http://gameprogrammingpatterns.com/game-loop.html
 */
static void onGameLoop()
{
    static double const MS_PER_UPDATE = 40.0/*GAME_LOOP_INTERVAL*/;//16.0; // 1000/60 ms.
    
    static uint64_t previous = 0;
    static double lag = 0.0;

    if(previous==0)
    {
        previous = Sys_get_posix_clock_time_ms();
    }
    
    uint64_t const current = Sys_get_posix_clock_time_ms(),
        elapsed = current - previous;
    
    previous = current;
    lag += (double)elapsed;

    //processInput(); // Already done by key events.

    while (lag >= MS_PER_UPDATE)
    {
        update();
        lag -= MS_PER_UPDATE;
    }

    render(lag/MS_PER_UPDATE);
}

static void charToFunc(char const inChar, bool const inVal)
{
    switch (inChar)
    {
        case 'a':
            ang_left = inVal;
            break;
        case 'd':
            ang_right = inVal;
            break;
        case 'w':
            pos_forward = inVal;
            break;
        case 's':
            pos_backward = inVal;
            break;  
        case 'q':
            pos_left = inVal;
            break;
        case 'e':
            pos_right = inVal;
            break;  
        case 'l':
            pos_up = inVal;
            break;
        case 'k':
            pos_down = inVal;
            break;
        case 'p':
            fov_wider = inVal;
            break;
        case 'o': 
            fov_narrower = inVal;
            break;
            
        default:
            break;
    }
}

/** GUI singleton will call this function as callback,
 *  if a key got pressed. The key will be represented by given character.
 */
static void onCharPress(char const inChar)
{
    charToFunc(inChar, true);
}

/** GUI singleton will call this function as callback,
 *  if a key got released. The key will be represented by given character.
 */
static void onCharRelease(char const inChar)
{
    charToFunc(inChar, false);
}

int main(int argc, char *argv[])
{
    Mt3dSingleton_init(WIDTH, HEIGHT); // Initializes 3D controller singleton.
    
    // Initialize and give control to GUI singleton (easily replaceable by some other GUI singleton):
    //
    GuiSingleton_cairo_init(WIDTH, HEIGHT, SCALE_FACTOR, "MT 3D", Mt3dSingleton_getPixels(), onCharPress, onCharRelease, onGameLoop, GAME_LOOP_INTERVAL);
    
    // -> GUI has control, here (GUI main loop is running). <-
    
    GuiSingleton_cairo_deinit(); // Exited from GUI main loop, deinitialize GUI singleton.
    
    Mt3dSingleton_deinit(); // Clean-up 3D singleton.
    
    return EXIT_SUCCESS;
}
