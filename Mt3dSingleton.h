
// MT, 2017jan14

#ifndef MT_3D_SINGLETON
#define MT_3D_SINGLETON

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool Mt3dSingleton_ang_left();
bool Mt3dSingleton_ang_right();
bool Mt3dSingleton_pos_left();
bool Mt3dSingleton_pos_right();
bool Mt3dSingleton_pos_forward();
bool Mt3dSingleton_pos_backward();
bool Mt3dSingleton_pos_up();
bool Mt3dSingleton_pos_down();
bool Mt3dSingleton_fov_wider();
bool Mt3dSingleton_fov_narrower();

int Mt3dSingleton_getWidth();
int Mt3dSingleton_getHeight();

unsigned char * Mt3dSingleton_getPixels();

void Mt3dSingleton_draw();
void Mt3dSingleton_init();
void Mt3dSingleton_deinit();

#ifdef __cplusplus
}
#endif

#endif // MT_3D_SINGLETON
