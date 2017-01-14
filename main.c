
// MT, 2016aug22

//NDEBUG

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "Deb.h"
#include "Mt3dSingleton.h"
#include "GuiSingleton_cairo.h"

static int const WIDTH = 320;
static int const HEIGHT = 200;
static double const SCALE_FACTOR = 4.0;

static void onGameLoop()
{
    static int c = 0;
    
    Deb_line("TIMER EVENT NR. %d.", ++c);
}

/** GUI singleton will call this function as callback,
 *  if a key got pressed. The key will be represented by given character.
 * 
 *  This callback triggers 3D library update, if necessary
 *  and returns true, if GUI singleton must redraw.
 */
static bool onCharPress(char const inChar)
{
    bool isDirty = false;
    
    switch (inChar)
    {
        case 'a':
            isDirty = Mt3dSingleton_ang_left();
            break;
        case 'd':
            isDirty = Mt3dSingleton_ang_right();
            break;
        case 'w':
            isDirty = Mt3dSingleton_pos_forward();
            break;
        case 's':
            isDirty = Mt3dSingleton_pos_backward();
            break;  
        case 'q':
            isDirty = Mt3dSingleton_pos_left();
            break;
        case 'e':
            isDirty = Mt3dSingleton_pos_right();
            break;  
        case 'l':
            isDirty = Mt3dSingleton_pos_up();
            break;
        case 'k':
            isDirty = Mt3dSingleton_pos_down();
            break;
        case 'p':
            isDirty = Mt3dSingleton_fov_wider();
            break;
        case 'o': 
            isDirty = Mt3dSingleton_fov_narrower();
            break;
            
        default:
            break;
    }
    if(isDirty)
    {
        Mt3dSingleton_draw();
        return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    Mt3dSingleton_init(WIDTH, HEIGHT); // Initializes 3D controller singleton.
    
    // Initialize and give control to GUI singleton (easily replaceable by some other GUI singleton):
    //
    GuiSingleton_cairo_init(WIDTH, HEIGHT, SCALE_FACTOR, "MT 3D", Mt3dSingleton_getPixels(), onCharPress, onGameLoop);
    
    // -> GUI has control, here (GUI main loop is running). <-
    
    GuiSingleton_cairo_deinit(); // Exited from GUI main loop, deinitialize GUI singleton.
    
    Mt3dSingleton_deinit(); // Clean-up 3D singleton.
    
    return EXIT_SUCCESS;
}
