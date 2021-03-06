
// MT, 2016aug22

//NDEBUG

#include <stdlib.h>
#include <stdio.h>

#include "Mt3dSingleton.h"

//#include "GuiSingleton_cairo.h"
#include "GuiSingleton_sdl.h"

#include "LoopSingleton.h"
#include "SinSingleton.h"
#include "Calc.h"

static int const WIDTH = 320;//384;
static int const HEIGHT = 200;//216;
static double const SCALE_FACTOR = 2.0;
static double const GAME_LOOP_INTERVAL = 40.0;
static double const MS_PER_UPDATE = 40.0;/*GAME_LOOP_INTERVAL*/;//16.0; // 1000/60 ms.
static size_t const LUT_LEN = 10000;

static void render(double const inLag)
{
    GuiSingleton_sdl_prepareForDirectDraw();
    Mt3dSingleton_draw();
    GuiSingleton_sdl_draw();
}

int main(int argc, char *argv[])
{
    SinSingleton_init(LUT_LEN);
    
    Mt3dSingleton_init(WIDTH, HEIGHT, (int)MS_PER_UPDATE, GuiSingleton_sdl_toggleFullscreen, GuiSingleton_sdl_quit); // Initializes 3D controller singleton. // Truncates
    LoopSingleton_init(MS_PER_UPDATE, Mt3dSingleton_update, render);

    // Initialize and give control to GUI singleton (easily replaceable by some other GUI singleton):
    //
    GuiSingleton_sdl_init(//GuiSingleton_cairo_init()
        WIDTH,
        HEIGHT,
        SCALE_FACTOR,
        "MT 3D",
        Mt3dSingleton_getPixels(),
        Mt3dSingleton_input_onKeyPress,
        Mt3dSingleton_input_onKeyRelease,
        LoopSingleton_run,
        GAME_LOOP_INTERVAL);

    // -> GUI has control, here (GUI main loop is running). <-

    GuiSingleton_sdl_deinit();//GuiSingleton_cairo_deinit() // Exited from GUI main loop, deinitialize GUI singleton.
    LoopSingleton_deinit();
    Mt3dSingleton_deinit(); // Clean-up 3D singleton.
    SinSingleton_deinit();
    return EXIT_SUCCESS;
}
