
// MT, 2017mar06

#ifndef SIN_SINGLETON
#define SIN_SINGLETON

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LUT_SIN(x) Calc_sin(SinSingleton_sinLut, SinSingleton_len, (x))
#define LUT_ASIN(x) Calc_asin(SinSingleton_asinLut, SinSingleton_len, (x))
    
extern size_t SinSingleton_len;
extern uint16_t * SinSingleton_sinLut;
extern uint16_t * SinSingleton_asinLut;

void SinSingleton_init(size_t const inLutLen);
void SinSingleton_deinit();

#ifdef __cplusplus
}
#endif

#endif // SIN_SINGLETON
