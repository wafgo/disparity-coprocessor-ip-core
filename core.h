#ifndef MY_CORE_HEADER_H
#define MY_CORE_HEADER_H

#include "bitmap.h"

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })


#define CONST_IMAGE_X_DIM	384
#define CONST_IMAGE_Y_DIM	288

#define ARRAY_SIZE(_X_) (sizeof(_X_)/sizeof(_X_[0]))
#define WINDOW_WIDTH	8
#define WINDOW_HEIGHT	8

#define BRAM_BUF_SIZE (WINDOW_HEIGHT * WINDOW_WIDTH * 3)

#define WITH_MAX_DISPARITY	TRUE
//#define WITH_MATCH_THRESHOLD TRUE

#define LINE_LENGTH_MAX		500
/* TEST BENCH */
#define TEST_IMG_X 			200
#define TEST_IMG_Y 			100
#define TEST_BPP			24
#define TEST_BYTES_PP		3
#define TEST_DISPARITY		3
#define TEST_DELTA_MAX		2

struct __attribute__((packed)) rgb {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

#endif /* MY_CORE_HEADER_H */
