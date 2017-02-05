
// MT, 2017jan22

#ifndef MT_3D_INPUT
#define MT_3D_INPUT

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Mt3dInput
{
    // *** FLAGS SIGNALIZING PRESSED KEYS ***

    bool quit;
    bool toggleFullscreen;
    bool ang_left;
    bool ang_right;
    bool pos_forward;
    bool pos_backward;
    bool pos_left;
    bool pos_right;
    bool pos_up;
    bool pos_down;
    bool fov_wider;
    bool fov_narrower;
    bool rot_z_ccw;
    bool rot_z_cw;
};

/** Try to find and set a flag for given character to given value in given object.
 *  Return, if a flag was found and set or not.
 */
bool Mt3dInput_setFlagByChar(char const inChar, bool const inVal, struct Mt3dInput * const inOutObj);
void Mt3dInput_delete(struct Mt3dInput * inObj);
struct Mt3dInput * Mt3dInput_create();

#ifdef __cplusplus
}
#endif

#endif // MT_3D_INPUT
