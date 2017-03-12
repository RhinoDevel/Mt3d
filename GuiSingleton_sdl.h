
// MT, 2017mar12

#ifndef GUI_SINGLETON_SDL
#define GUI_SINGLETON_SDL

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void GuiSingleton_sdl_prepareForDirectDraw();
void GuiSingleton_sdl_draw();
void GuiSingleton_sdl_toggleFullscreen();
void GuiSingleton_sdl_quit();
    
void GuiSingleton_sdl_init(
    int const inWidth, 
    int const inHeight,
    double const inScaleFactor,
    char const * const inWinTitle,
    uint32_t * const inPixels,
    void (*inKeyPressHandler)(char const),
    void (*inKeyReleaseHandler)(char const),
    void (*inTimerHandler)(void),
    int const inTimerInterval);
void GuiSingleton_sdl_deinit();

#ifdef __cplusplus
}
#endif

#endif // GUI_SINGLETON_SDL
