
// MT, 2017mar06

#include "SinSingleton.h"
#include "Calc.h"

#include <stdlib.h>
#include <assert.h>

size_t SinSingleton_len = 0;
uint16_t * SinSingleton_sinLut = NULL;
uint16_t * SinSingleton_asinLut = NULL;

void SinSingleton_init(size_t const inLutLen)
{
    assert(SinSingleton_len==0);
    assert(SinSingleton_sinLut==NULL);
    assert(SinSingleton_asinLut==NULL);
    
    SinSingleton_len = inLutLen;
    
    SinSingleton_sinLut = Calc_createSinLut(SinSingleton_len);
    assert(SinSingleton_sinLut!=NULL);
    
    SinSingleton_asinLut = Calc_createArcSinLut(SinSingleton_len);
    assert(SinSingleton_asinLut!=NULL);
}

void SinSingleton_deinit()
{
    assert(SinSingleton_len>0);
    
    assert(SinSingleton_sinLut!=NULL);
    free(SinSingleton_sinLut);
    SinSingleton_sinLut = NULL;
    
    assert(SinSingleton_asinLut!=NULL);
    free(SinSingleton_asinLut);
    SinSingleton_asinLut = NULL;
    
    SinSingleton_len = 0;
}
