
// MT, 2016aug22

//NDEBUG

#include <stdlib.h>

#include "Mt3d.h"

static int const WIDTH = 300;
static int const HEIGHT = 200;
static int const ALPHA = 90;
static int const BETA = 60;
static double const H = 0.5;

int main()
{
    struct Mt3d * const o = Mt3d_create(WIDTH, HEIGHT, ALPHA, BETA, H);
    
    Mt3d_delete(o);
    return EXIT_SUCCESS;
}
