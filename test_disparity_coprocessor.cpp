#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "core.h"
#include <time.h>

int disparity_pixel_coprocessor(volatile unsigned int* addrLeft, volatile unsigned int* addrRight,
		volatile unsigned int *addrOutput, unsigned char bpp, unsigned int xDim, unsigned int yDim, volatile int* current_row
#ifdef WITH_MAX_DISPARITY
		, unsigned int maxDisparity
#endif
#ifdef WITH_MATCH_THRESHOLD
		, unsigned int match_threshold
#endif
);

#define MEM_SIZE (TEST_IMG_X * TEST_IMG_Y * TEST_BYTES_PP)


#define LEFT_FILE_NAME "../../../../pics/left.bmp"
#define RIGHT_FILE_NAME "../../../../pics/right.bmp"
#define REF_FILE_NAME "../../../../pics/ref.dat"

static int crow;
static BITMAP_INFO_HEADER rightHeaderInfo;
static BITMAP_INFO_HEADER leftHeaderInfo;
static BITMAP_FILE_HEADER rightFileHeader;
static BITMAP_FILE_HEADER leftFileHeader;
static int rightPadding, leftPadding, leftHeaderSize, rightHeaderSize;

static bool loadBMPHeader(uint8_t* dta, BITMAP_INFO_HEADER* infoHeader,
		BITMAP_FILE_HEADER* bitmapHeader, int* padding, int* headerSize) {
	bool l_Created = false;

	int l_HeaderSize = sizeof(BITMAP_FILE_HEADER);
	memcpy(bitmapHeader, dta, l_HeaderSize);

	int l_HeaderInfoSize = sizeof(BITMAP_INFO_HEADER);
	memcpy(infoHeader, dta + l_HeaderSize, l_HeaderInfoSize);

	*padding = 0;
	int l_Pitch = infoHeader->biWidth * 3;
	if (l_Pitch % 4 != 0) {
		*padding = (4 - (infoHeader->biWidth % 4));
	}

	*headerSize = l_HeaderSize + l_HeaderInfoSize;
	l_Created = true;

	return l_Created;
}

bool is_within_delta(uint32_t expected, uint32_t actual) {

	int32_t s_expected = (int32_t) expected;
	uint32_t expected_max = expected + TEST_DELTA_MAX;
	uint32_t expected_min = max((s_expected - TEST_DELTA_MAX), 0);

	if ((actual > expected_max) || (actual < expected_min)) {
		return false;
	}

	return true;
}

int main() {
	int i,j;

	struct stat str, stl;
	uint8_t* leftData;
	uint8_t* rightData;

	FILE* leftImag = fopen(LEFT_FILE_NAME, "r");
	FILE* rightImag = fopen(RIGHT_FILE_NAME, "r");
	FILE* refDisp = fopen(REF_FILE_NAME, "r");

	stat(LEFT_FILE_NAME, &stl);
	stat(RIGHT_FILE_NAME, &str);

	if (leftImag == NULL) {
		printf("Error opening left file\n");
		return 1;
	}

	if (rightImag == NULL) {
		printf("Error opening right file\n");
		return 2;
	}

	if (refDisp == NULL) {
		printf("Error could not open ref.dat \n");
		return 2;
	}

	leftData = (uint8_t*) malloc(stl.st_size);
	rightData = (uint8_t*) malloc(str.st_size);

	if (leftData == NULL || rightData == NULL) {
		printf("Could not allocate memory\n");
		return 3;
	}

	if (stl.st_size != fread(leftData, 1, stl.st_size, leftImag)) {
		printf("could not read left image\n");
		return 4;
	}
	if (str.st_size != fread(rightData, 1, str.st_size, rightImag)) {
		printf("could not read right image\n");
		return 5;
	}

	loadBMPHeader(leftData, &leftHeaderInfo, &leftFileHeader, &leftPadding, &leftHeaderSize);
	loadBMPHeader(rightData, &rightHeaderInfo, &rightFileHeader, &rightPadding, &rightHeaderSize);

	printf(
			"INFO: left image %i height %i width %i compression \r\n",
			leftHeaderInfo.biHeight, leftHeaderInfo.biWidth,
			leftHeaderInfo.biCompression);
	printf("INFO: right image %i height %i width\r\n",
			rightHeaderInfo.biHeight, rightHeaderInfo.biWidth);

	uint8_t* rawDataR = rightData + rightFileHeader.bfOffBits;
	uint8_t* rawDataL = leftData + leftFileHeader.bfOffBits;

	uint32_t* rawDataO = (uint32_t*) malloc(leftHeaderInfo.biHeight*leftHeaderInfo.biWidth*4);

	memset(rawDataO, 0, leftHeaderInfo.biHeight*leftHeaderInfo.biWidth*4);
	if (rawDataO == NULL) {
		printf("could not alloc output buffer\n");
		return 6;
	}

	double time1=0.0, tstart;
	tstart = clock();

	disparity_pixel_coprocessor((uint32_t*)rawDataL, (uint32_t*)rawDataR, (uint32_t*)rawDataO, TEST_BPP, rightHeaderInfo.biWidth, rightHeaderInfo.biHeight, &crow
#ifdef WITH_MAX_DISPARITY
								, 25
#endif
#ifdef WITH_MATCH_THRESHOLD
								, 10000
#endif
);
	time1 += clock() - tstart;

	time1 = time1/CLOCKS_PER_SEC;

	printf("execution time: %lf sec\n", time1);
	int cnt = 0;
	for (i = 0; i < rightHeaderInfo.biHeight; ++i) {
	  for (j = 0; j < rightHeaderInfo.biWidth; ++j) {
		  	  	  uint32_t expected;
				  fread(&expected, sizeof(expected), 1, refDisp);
				  if ( !is_within_delta(expected, rawDataO[cnt]) ) {
					  printf("error detected at row %d column %d. expected: %d actual: %d\n", i,j, expected, rawDataO[cnt]);
					  return -1;
				  }
				cnt++;
			}
	  printf("\n");
	}
err_out:
	fclose(refDisp);
	free(leftData);
	free(rightData);
	free(rawDataO);

	return 0;
}
