
// MT, 2016aug23

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "Sys.h"
#include "Bmp.h"

static uint16_t const BITS_PER_PIXEL = 24;
static uint16_t const PLANES = 1;
static uint32_t const COMPRESSION = 0;
static uint32_t const X_PIXEL_PER_METER = 0x130B; // 2835, 72 DPI.
static uint32_t const Y_PIXEL_PER_METER = 0x130B; // 2835, 72 DPI.

#pragma pack(push,1)
struct FileHeader
{
    char signature[2];
    uint32_t fileSize;
    uint32_t reserved;
    uint32_t fileOffsetToPixelArr;
};
struct BitmapInfoHeader
{
    uint32_t dibHeaderSize;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imgSize;
    uint32_t yPixelPerMeter;
    uint32_t xPixelPerMeter;
    uint32_t numColorsPalette;
    uint32_t mostImpColor;
};
struct Bitmap
{
    struct FileHeader fileHeader;
    struct BitmapInfoHeader bmpInfoHeader;
};
#pragma pack(pop)

void Bmp_write(int const inWidth, int const inHeight, unsigned char const * const inPixels, char const * const inFilePath)
{
    assert(!Sys_is_big_endian());
    assert(inWidth>0);
    assert(inHeight>0);
    assert(inPixels!=NULL);
    assert(inFilePath!=NULL);
    
    struct Bitmap * const bmp = calloc(1, sizeof *bmp);
    uint32_t const pixelByteSize = (inWidth*inHeight*BITS_PER_PIXEL)/8;
    
    bmp->fileHeader.signature[0] = 'B';
    bmp->fileHeader.signature[1] = 'M';
    bmp->fileHeader.fileSize = sizeof *bmp+pixelByteSize;
    bmp->fileHeader.fileOffsetToPixelArr = sizeof *bmp;
    
    bmp->bmpInfoHeader.dibHeaderSize =sizeof bmp->bmpInfoHeader;
    bmp->bmpInfoHeader.width = inWidth;
    bmp->bmpInfoHeader.height = -inHeight; // HARD-CODED top to bottom pixel order by negation!
    bmp->bmpInfoHeader.planes = PLANES;
    bmp->bmpInfoHeader.bitsPerPixel = BITS_PER_PIXEL;
    bmp->bmpInfoHeader.compression = COMPRESSION;
    bmp->bmpInfoHeader.imgSize = pixelByteSize;
    bmp->bmpInfoHeader.yPixelPerMeter = Y_PIXEL_PER_METER;
    bmp->bmpInfoHeader.xPixelPerMeter = X_PIXEL_PER_METER;
    bmp->bmpInfoHeader.numColorsPalette = 0;
    bmp->bmpInfoHeader.mostImpColor = 0;
    
    FILE * const fp = fopen(inFilePath,"wb");
    
    fwrite(bmp, 1, sizeof *bmp ,fp);
    fwrite(inPixels, 1, pixelByteSize, fp);
    fclose(fp);
    free(bmp);
}
