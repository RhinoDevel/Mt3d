
// MT, 2017jan14

#ifndef MT_3D_SINGLETON
#define MT_3D_SINGLETON

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t * Mt3dSingleton_getPixels();

void Mt3dSingleton_input_onKeyPress(char const inChar);
void Mt3dSingleton_input_onKeyRelease(char const inChar);
void Mt3dSingleton_update();
void Mt3dSingleton_draw();
void Mt3dSingleton_init(int const inWidth, int const inHeight, int const inMsPerUpdate, void (* const inToggleFullscreen)(void), void (* const inQuit)(void));
void Mt3dSingleton_deinit();

#ifdef __cplusplus
}
#endif

#endif // MT_3D_SINGLETON
