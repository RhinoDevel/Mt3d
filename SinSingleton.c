
// MT, 2017mar06

#include "SinSingleton.h"
#include "Calc.h"

#include <stdlib.h>
#include <assert.h>

static uint16_t * lut = NULL;

uint16_t* SinSingleton_getLut()
{
    return lut; // Caller does NOT take ownership!
}

void SinSingleton_init(size_t const inLutLen)
{
    assert(lut==NULL);
    
    lut = Calc_createSinLut(inLutLen);
    assert(lut!=NULL);
}

void SinSingleton_deinit()
{
    assert(lut!=NULL);
    free(lut);
    lut = NULL;
}
