
// MT, 2016aug22

//NDEBUG

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "Mt3dSingleton.h"
#include "GuiSingleton_cairo.h"

static double const SCALE_FACTOR = 4.0;

static bool onCharPress(char const inChar)
{
    bool isDirty = false;
    
    switch (inChar)
    {
        case 'a':
            isDirty = Mt3dSingleton_ang_left(); // (implicit "cast")
            break;
        case 'd':
            isDirty = Mt3dSingleton_ang_right(); // (implicit "cast")
            break;
        case 'w':
            isDirty = Mt3dSingleton_pos_forward(); // (implicit "cast")
            break;
        case 's':
            isDirty = Mt3dSingleton_pos_backward(); // (implicit "cast")
            break;  
        case 'q':
            isDirty = Mt3dSingleton_pos_left(); // (implicit "cast")
            break;
        case 'e':
            isDirty = Mt3dSingleton_pos_right(); // (implicit "cast")
            break;  
        case 'l':
            isDirty = Mt3dSingleton_pos_up(); // (implicit "cast")
            break;
        case 'k':
            isDirty = Mt3dSingleton_pos_down(); // (implicit "cast")
            break;
        case 'p':
            isDirty = Mt3dSingleton_fov_wider(); // (implicit "cast")
            break;
        case 'o': 
            isDirty = Mt3dSingleton_fov_narrower(); // (implicit "cast")
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
    Mt3dSingleton_init();
    GuiSingleton_cairo_init(Mt3dSingleton_getWidth(), Mt3dSingleton_getHeight(), SCALE_FACTOR, "MT 3D", Mt3dSingleton_getPixels(), onCharPress);
    GuiSingleton_cairo_deinit();
    Mt3dSingleton_deinit();
    return EXIT_SUCCESS;
}
