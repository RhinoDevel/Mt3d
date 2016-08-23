
// MT, 2016aug23

#ifndef MT_BMP
#define MT_BMP

#ifdef __cplusplus
extern "C" {
#endif

void Bmp_write(int const inWidth, int const inHeight, unsigned char const * const inPixels, char const * const inFilePath);

#ifdef __cplusplus
}
#endif

#endif // MT_BMP
