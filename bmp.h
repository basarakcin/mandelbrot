// Include Guards
// (alternative #pragma once)
#ifndef BMP_H
#define BMP_H
#include <stdint.h>
// #pragma pack
// https://docs.microsoft.com/en-us/cpp/preprocessor/pack?view=vs-2019
#pragma pack(push, 1)
// BITMAPFILEHEADER
// https://docs.microsoft.com/en-us/windows/desktop/api/Wingdi/ns-wingdi-tagbitmapfileheader
typedef struct
{
    uint16_t bfType;      // "Magic Number": 0x4d42 = "BM"
    uint32_t bfSize;      // Dateigröße (byte)
    uint16_t bfReserved1; // Nicht belegt; muss 0 sein
    uint16_t bfReserved2; // Nicht belegt; muss 0 sein
    uint32_t bfOffBits;   // Offset von Bilddata = 54
} BITMAPFILEHEADER;

// BITMAPINFOHEADER
// https://docs.microsoft.com/en-us/previous-versions//dd183376(v=vs.85)
typedef struct
{
    uint32_t biSize;         // Größe des DIB Headers (byte)
    int32_t biWidth;         // Bildbreite
    int32_t biHeight;        // Bildhöhe
    uint16_t biPlanes;       // Anzahl der Farbebene
    uint16_t biBitCount;     // Bits pro Pixel
    uint32_t biCompression;  // Komprimierungsart
    uint32_t biSizeImage;    // Bildgröße (byte) = 40
    int32_t biXPelsPerMeter; // 0
    int32_t biYPelsPerMeter; // 0
    uint32_t biClrUsed;      // 0
    uint32_t biClrImportant; // 0
} BITMAPINFOHEADER;

typedef struct
{
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
} RGBTRIPLET;

#pragma pack(pop)
#endif  // !BMP_H