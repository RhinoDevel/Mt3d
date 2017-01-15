
// MT, 2017jan14

#ifndef GUI_SINGLETON_CAIRO
#define GUI_SINGLETON_CAIRO

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void GuiSingleton_cairo_prepareForDirectDraw();
void GuiSingleton_cairo_draw();
    
void GuiSingleton_cairo_init(
    int const inWidth, 
    int const inHeight,
    double const inScaleFactor,
    char const * const inWinTitle,
    unsigned char * const inPixels,
    void (*inKeyPressHandler)(char const),
    void (*inKeyReleaseHandler)(char const),
    void (*inTimerHandler)(void),
    int const inTimerInterval);
void GuiSingleton_cairo_deinit();

#ifdef __cplusplus
}
#endif

#endif // GUI_SINGLETON_CAIRO
