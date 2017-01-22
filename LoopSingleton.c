
// MT, 2017jan22

#include <assert.h>
#include <stdlib.h>

#include "LoopSingleton.h"
#include "Loop.h"

static struct Loop * o = NULL;

void LoopSingleton_run()
{
    Loop_run(o);
}

void LoopSingleton_init(double const inMsPerUpdate, void (* const inUpdate)(void), void (* const inRender)(double const))
{
    assert(o==NULL);

    o = Loop_create(inMsPerUpdate, inUpdate, inRender);
}
void LoopSingleton_deinit()
{
    assert(o!=NULL);
    
    Loop_delete(o);
    o = NULL;
}
