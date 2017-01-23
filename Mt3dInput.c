
// MT, 2017jan22

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "Mt3dInput.h"


bool Mt3dInput_setFlagByChar(char const inChar, bool const inVal, struct Mt3dInput * const inOutObj)
{
    assert(inOutObj!=NULL);
    
    bool retVal = true; // TRUE by default.
    
    switch (inChar)
    {
        case 'x':
            inOutObj->quit = inVal;
            break;
        case 'a':
            inOutObj->ang_left = inVal;
            break;
        case 'd':
            inOutObj->ang_right = inVal;
            break;
        case 'w':
            inOutObj->pos_forward = inVal;
            break;
        case 's':
            inOutObj->pos_backward = inVal;
            break;  
        case 'q':
            inOutObj->pos_left = inVal;
            break;
        case 'e':
            inOutObj->pos_right = inVal;
            break;  
        case 'l':
            inOutObj->pos_up = inVal;
            break;
        case 'k':
            inOutObj->pos_down = inVal;
            break;
        case 'p':
            inOutObj->fov_wider = inVal;
            break;
        case 'o': 
            inOutObj->fov_narrower = inVal;
            break;
            
        default:
            retVal = false;
            break;
    }
    
    return retVal;
}

void Mt3dInput_delete(struct Mt3dInput * const inObj)
{
    assert(inObj!=NULL); 
    free(inObj);
}

struct Mt3dInput * Mt3dInput_create()
{
    struct Mt3dInput * const retVal = malloc(sizeof *retVal);
    assert(retVal!=NULL);
    
    struct Mt3dInput const buf = (struct Mt3dInput)
    {
        .quit = false,
        .ang_left = false,
        .ang_right= false,
        .pos_forward= false,
        .pos_backward= false,
        .pos_left= false,
        .pos_right= false,
        .pos_up= false,
        .pos_down= false,
        .fov_wider= false,
        .fov_narrower = false
    };

    memcpy(retVal, &buf, sizeof *retVal);
    
    return retVal;
}
