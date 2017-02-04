
// MT, 2016aug23

#ifndef MT_CELLTYPE
#define MT_CELLTYPE

#ifdef __cplusplus
extern "C" {
#endif

enum CellType
{
    CellType_floor_default = 0,
    CellType_block_default = 1,
    CellType_floor_exit = 2,
    CellType_COUNT = 3
};
    
#ifdef __cplusplus
}
#endif

#endif // MT_CELLTYPE
