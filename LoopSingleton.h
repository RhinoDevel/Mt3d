
// MT, 2017jan22

#ifndef LOOP_SINGLETON
#define LOOP_SINGLETON

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void LoopSingleton_run();
void LoopSingleton_init(double const inMsPerUpdate, void (* const inUpdate)(void), void (* const inRender)(double const));
void LoopSingleton_deinit();

#ifdef __cplusplus
}
#endif

#endif // LOOP_SINGLETON
