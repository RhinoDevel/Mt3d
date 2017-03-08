
// MT, 2017mar06

#include "SinSingleton.h"
#include "Calc.h"

#include <stdlib.h>
#include <assert.h>

static uint16_t * lut = NULL;
static uint16_t * asinLut = NULL;

uint16_t* SinSingleton_getLut()
{
    return lut; // Caller does NOT take ownership!
}

uint16_t* SinSingleton_getAsinLut()
{
    return asinLut; // Caller does NOT take ownership!
}

void SinSingleton_init(size_t const inLutLen)
{
    assert(lut==NULL);
    assert(asinLut==NULL);
    
    lut = Calc_createSinLut(inLutLen);
    assert(lut!=NULL);
    
    asinLut = Calc_createArcSinLut(inLutLen);
    assert(asinLut!=NULL);
}

void SinSingleton_deinit()
{
    assert(lut!=NULL);
    free(lut);
    lut = NULL;
    
    assert(asinLut!=NULL);
    free(asinLut);
    asinLut = NULL;
}
