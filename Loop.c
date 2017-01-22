
// MT, 2017jan22

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "Sys.h"
#include "Loop.h"

/**
 * - See: http://gameprogrammingpatterns.com/game-loop.html
 */
void Loop_run(struct Loop * const inOutObj)
{   
    assert(inOutObj!=NULL);
    
    uint64_t const current = Sys_get_posix_clock_time_ms();
    
    if(inOutObj->frameCountStartTime==0)
    {
        inOutObj->frameCountStartTime = current;
        inOutObj->frameCount = 0;
    }
    
    if(inOutObj->previous==0)
    {
        inOutObj->previous = current;
    }
    
    uint64_t const elapsed = current - inOutObj->previous;
    
    inOutObj->previous = current;
    inOutObj->lag += (double)elapsed;

    //processInput();

    while (inOutObj->lag >= inOutObj->msPerUpdate)
    {
        inOutObj->update();
        inOutObj->lag -= inOutObj->msPerUpdate;
    }

    inOutObj->render(inOutObj->lag/inOutObj->msPerUpdate);
    ++inOutObj->frameCount;

    uint64_t const frameCountElapsed = Sys_get_posix_clock_time_ms()-inOutObj->frameCountStartTime;
    
    if(frameCountElapsed>=1000)
    {
        Sys_log_line(true, true, "FPS: %f.", (1000.0*(double)inOutObj->frameCount)/(double)frameCountElapsed);
        inOutObj->frameCountStartTime = 0; // (frameCount gets reset above during next run)
    }
}

void Loop_delete(struct Loop * const inObj)
{
    assert(inObj!=NULL); 
    free(inObj);
}

struct Loop * Loop_create(double const inMsPerUpdate, void (* const inUpdate)(void), void (* const inRender)(double const))
{
    struct Loop * const retVal = malloc(sizeof *retVal);
    assert(retVal!=NULL);
    
    struct Loop const buf = (struct Loop)
    {
        .msPerUpdate = inMsPerUpdate,
        .update = inUpdate,
        .render = inRender,
       
        .frameCountStartTime = 0,
        .frameCount = 0,
        .previous = 0,
        .lag = 0.0
    };

    memcpy(retVal, &buf, sizeof *retVal);
    
    return retVal;
}
