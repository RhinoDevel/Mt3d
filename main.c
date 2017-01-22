
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
static double const GAME_LOOP_INTERVAL = 40.0;
static double const MS_PER_UPDATE = 40.0;/*GAME_LOOP_INTERVAL*/;//16.0; // 1000/60 ms.

void render(double const inLag)
{
    GuiSingleton_cairo_prepareForDirectDraw();
    Mt3dSingleton_draw();
    GuiSingleton_cairo_draw();
}
//
/**
 * - See: http://gameprogrammingpatterns.com/game-loop.html
 */
static void onGameLoop()
{   
    static uint64_t frameCountStartTime = 0;
    static int frameCount = 0;
    static uint64_t previous = 0;
    static double lag = 0.0;
    
    uint64_t const current = Sys_get_posix_clock_time_ms();
    
    if(frameCountStartTime==0)
    {
        frameCountStartTime = current;
        frameCount = 0;
    }
    
    if(previous==0)
    {
        previous = current;
    }
    
    uint64_t const elapsed = current - previous;
    
    previous = current;
    lag += (double)elapsed;

    //processInput(); // Already done by key events.

    while (lag >= MS_PER_UPDATE)
    {
        Mt3dSingleton_update();
        lag -= MS_PER_UPDATE;
    }

    render(lag/MS_PER_UPDATE);
    ++frameCount;

    uint64_t const frameCountElapsed = Sys_get_posix_clock_time_ms()-frameCountStartTime;
    
    if(frameCountElapsed>=1000)
    {
        Sys_log_line(true, true, "FPS: %f.", (1000.0*(double)frameCount)/(double)frameCountElapsed);
        frameCountStartTime = 0; // (frameCount gets reset above during next run)
    }
}

int main(int argc, char *argv[])
{   
    Mt3dSingleton_init(WIDTH, HEIGHT, (int)MS_PER_UPDATE); // Initializes 3D controller singleton. // Truncates
    
    // Initialize and give control to GUI singleton (easily replaceable by some other GUI singleton):
    //
    GuiSingleton_cairo_init(WIDTH, HEIGHT, SCALE_FACTOR, "MT 3D", Mt3dSingleton_getPixels(), Mt3dSingleton_input_onKeyPress, Mt3dSingleton_input_onKeyRelease, onGameLoop, GAME_LOOP_INTERVAL);
    
    // -> GUI has control, here (GUI main loop is running). <-
    
    GuiSingleton_cairo_deinit(); // Exited from GUI main loop, deinitialize GUI singleton.
    
    Mt3dSingleton_deinit(); // Clean-up 3D singleton.
    
    return EXIT_SUCCESS;
}
