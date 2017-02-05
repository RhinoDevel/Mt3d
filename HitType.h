
// MT, 2017feb05

#ifndef MT_HITTYPE
#define MT_HITTYPE

#ifdef __cplusplus
extern "C" {
#endif

enum HitType
{
    HitType_none = 0, // Hits neither floor, nor ceiling.
    HitType_floor = 1, // Hits floor.
    HitType_ceil = 2, // Hits ceiling.
};

#ifdef __cplusplus
}
#endif

#endif // MT_HITTYPE
