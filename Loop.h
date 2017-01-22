
// MT, 2017jan14

#ifndef MT_3D_LOOP
#define MT_3D_LOOP

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Loop
{
    double const msPerUpdate;
    void (* const update)(void);
    void (* const render)(double const);
    
    uint64_t frameCountStartTime;
    int frameCount;
    uint64_t previous;
    double lag;
};

void Loop_run(struct Loop * const inOutObj);
struct Loop * Loop_create(double const inMsPerUpdate, void (* const inUpdate)(void), void (* const inRender)(double const));
void Loop_delete(struct Loop * inObj);
struct Loop * Loop_create();

#ifdef __cplusplus
}
#endif

#endif // MT_3D_LOOP
