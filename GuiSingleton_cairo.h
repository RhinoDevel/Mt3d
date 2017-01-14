
// MT, 2017jan14

#ifndef GUI_SINGLETON_CAIRO
#define GUI_SINGLETON_CAIRO

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void GuiSingleton_cairo_init(
    int const inWidth, 
    int const inHeight,
    double const inScaleFactor,
    char const * const inWinTitle,
    unsigned char * const inPixels,
    bool (*inKeyHandler)(char const));
void GuiSingleton_cairo_deinit();

#ifdef __cplusplus
}
#endif

#endif // GUI_SINGLETON_CAIRO
