#ifndef BITMAPIMAGE_H_
#define BITMAPIMAGE_H_

#include <stdint.h>

typedef struct
{
      int8_t bfType[2];     // the header field used to identify the BMP & DIB file is 0x42 0x4D in hexadecimal, same as BM in ASCII. The following entries are possible:
                           // BM � Windows 3.1x, 95, NT, ... etc.; and it is not mandatory unless file size is greater or equal to SIGNATURE
                           // BA � OS/2 struct Bitmap Array
                           // CI � OS/2 struct Color Icon
                           // CP � OS/2 constant Color Pointer
                           // IC � OS/2 struct Icon
                           // PT � OS/2 Pointer
      uint32_t bfSize;     // the size of the BMP file in bytes (unsafe)
      uint32_t bfReserved; // reserved; actual value depends on the application that creates the image
      uint32_t bfOffBits;  // the offset, i.e. starting address, of the byte where the bitmap image data (pixel array) can be found.
} __attribute__((packed)) BITMAP_FILE_HEADER;

// Struct defines the information header of a bitmap file
// The order of the values represent the first bytes inside a bitmap file
typedef struct
{
	uint32_t biSize;                          /* the size of this header (40 bytes)     */
	int32_t  biWidth, biHeight;                /* the bitmap width, height in pixels (signed integer). */
	uint16_t biPlanes;                        /* the number of color planes being used. Must be set to 1.    */
	uint16_t biBitCount;                      /* the number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16, 24 and 32.            */
	uint32_t biCompression;                   /* the compression method being used. See the next table for a list of possible values.          */
	uint32_t biSizeImage;                     /* the image size. This is the size of the raw bitmap data (see below), and should not be confused with the file size.      */
	int32_t  biXPelsPerMeter, biYPelsPerMeter; /* the horizontal, vertical resolution of the image. (pixel per meter, signed integer)          */
	uint32_t biClrUsed;                       /* the number of colors in the color palette, or 0 to default to 2n.        */
	uint32_t biClrImportant;                  /* the number of important colors used, or 0 when every color is important; generally ignored.         */
      //unsigned char palette[1024];            /* Storage for indexed color palate, size 2^biBitCount */
} __attribute__((packed)) BITMAP_INFO_HEADER;

// Possible bitmap data compression methods
typedef enum {
  BI_RGB = 0,
  BI_RLE8,
  BI_RLE4,
  BI_BITFIELDS, //Huffman 1D
  BI_JPEG,      //RLE-24
  BI_PNG
} BITMAP_COMPRESSION;

#endif /* BITMAPIMAGE_H_ */
