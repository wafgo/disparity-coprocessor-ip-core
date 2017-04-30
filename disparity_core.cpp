/*
 * disparity_core.cpp
 *
 *  Created on: 17.06.2016
 *      Author: Wadim Mueller
 */
#include <string.h>
#include <stdint.h>
#include "core.h"
#include <stdlib.h>
#include <math.h>
#include <limits.h>

static uint32_t get_correlation(uint8_t* ref, uint8_t * cmp) {
#pragma HLS INLINE
	uint32_t res = 0;
	int tmpr, tmpg, tmpb;
	int rr, gr, br, rc, gc, bc;

	for (int i = 0; i < (WINDOW_WIDTH * WINDOW_HEIGHT); i++) {
#pragma HLS UNROLL
		int array_idx = i * 3;
		rr = ref[array_idx];
		gr = ref[array_idx + 1];
		br = ref[array_idx + 2];

		rc = cmp[array_idx];
		gc = cmp[array_idx + 1];
		bc = cmp[array_idx + 2];

		tmpr = abs(rr - rc);
		tmpg = abs(gr - gc);
		tmpb = abs(br - bc);
		res += tmpr + tmpg + tmpb;
	}
	return res;
}

static void __memcpy(uint8_t* to, uint8_t* from, uint32_t count) {
#pragma HLS INLINE
	int loc_cnt = count >> 2;
	uint32_t* lto = (uint32_t*) to;
	uint32_t* lfrom = (uint32_t*) from;

	while (loc_cnt--) {
		*lto = *lfrom;
		lto++;
		lfrom++;
	}
}

int disparity_pixel_coprocessor(volatile unsigned int* addrLeft, volatile unsigned int* addrRight,
		volatile unsigned int *addrOutput, unsigned char bpp, unsigned int xDim, unsigned int yDim, volatile int* current_row
#ifdef WITH_MAX_DISPARITY
		, unsigned int maxDisparity
#endif
#ifdef WITH_MATCH_THRESHOLD
		, unsigned int match_threshold
#endif
) {
#pragma HLS PIPELINE

#pragma HLS INTERFACE s_axilite port=current_row bundle=CTRL_BUS
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE s_axilite port=yDim bundle=CTRL_BUS
#pragma HLS INTERFACE s_axilite port=xDim bundle=CTRL_BUS
#pragma HLS INTERFACE s_axilite port=bpp bundle=CTRL_BUS
#ifdef WITH_MATCH_THRESHOLD
#pragma HLS INTERFACE s_axilite port=match_threshold bundle=CTRL_BUS
#endif
#ifdef WITH_MAX_DISPARITY
#pragma HLS INTERFACE s_axilite port=maxDisparity bundle=CTRL_BUS
#endif
#pragma HLS INTERFACE m_axi depth=10000 port=addrOutput offset=slave bundle=MASTER_BUS
#pragma HLS INTERFACE m_axi depth=10000 port=addrRight offset=slave bundle=MASTER_BUS
#pragma HLS INTERFACE m_axi depth=10000 port=addrLeft offset=slave bundle=MASTER_BUS

	int bytes_per_pixel = bpp >> 3;
	int current_right_addr_idx = 0;
	int current_left_addr_idx = 0;
	int right_buffer_idx = 0;
	int left_buffer_idx = 0;
	int bytes_per_window_line = WINDOW_WIDTH * bytes_per_pixel;
	int bytes_per_image_line = xDim * bytes_per_pixel;
	uint32_t min = UINT_MAX;
	uint32_t res;
	uint32_t disparity;
	int i;
	int oline_idx = 0;

	static uint8_t right_buffer[BRAM_BUF_SIZE];
	static uint8_t left_buffer[BRAM_BUF_SIZE];
	static uint32_t output_buffer[LINE_LENGTH_MAX];
	static uint32_t lrBufTmp[LINE_LENGTH_MAX*WINDOW_HEIGHT];
	static uint32_t llBufTmp[LINE_LENGTH_MAX*WINDOW_HEIGHT];
	static uint8_t rBufTmp[LINE_LENGTH_MAX*WINDOW_HEIGHT*4];
	static uint8_t lBufTmp[LINE_LENGTH_MAX*WINDOW_HEIGHT*4];

	for (int row = 0; row < (yDim - WINDOW_HEIGHT); ++row) {
		uint32_t c_row_idx = (row * bytes_per_image_line)>>2;
		uint32_t c_row_idx_out = row * xDim;

		*current_row = row;

		memcpy((uint32_t*)lrBufTmp, (uint32_t*)&addrRight[c_row_idx], bytes_per_image_line*WINDOW_HEIGHT);
		memcpy((uint32_t*)llBufTmp, (uint32_t*)&addrLeft[c_row_idx], bytes_per_image_line*WINDOW_HEIGHT);

		__memcpy(rBufTmp, (uint8_t*)lrBufTmp, bytes_per_image_line*WINDOW_HEIGHT);
		__memcpy(lBufTmp, (uint8_t*)llBufTmp, bytes_per_image_line*WINDOW_HEIGHT);

		for (int col_left = 0; col_left < (xDim - WINDOW_WIDTH); ++col_left) {

			current_left_addr_idx = (col_left * 3);

			for (i = 0; i < WINDOW_HEIGHT; ++i) {
				__memcpy((uint8_t*) &left_buffer[left_buffer_idx], (uint8_t*) &lBufTmp[current_left_addr_idx],
						bytes_per_window_line);
				left_buffer_idx += bytes_per_window_line;
				current_left_addr_idx += bytes_per_image_line;
			}
			left_buffer_idx = 0;
#ifdef WITH_MAX_DISPARITY
			int pix_cnt = 0;
#endif
			for (int col_right = col_left; col_right < (xDim - WINDOW_WIDTH); ++col_right) {
				current_right_addr_idx = (col_right * 3);

				for (i = 0; i < WINDOW_HEIGHT; ++i) {
					__memcpy((uint8_t*) &right_buffer[right_buffer_idx], (uint8_t*) &rBufTmp[current_right_addr_idx],
							bytes_per_window_line);
					right_buffer_idx += bytes_per_window_line;
					current_right_addr_idx += bytes_per_image_line;
				}
#ifdef WITH_MAX_DISPARITY
				pix_cnt++;
#endif
				res = get_correlation((uint8_t*) left_buffer, (uint8_t*) right_buffer);
				right_buffer_idx = 0;

				if (res < min) {
					min = res;
					disparity = col_right - col_left;
#ifdef WITH_MATCH_THRESHOLD
					if (min < match_threshold)
						break;
#endif
				}
#ifdef WITH_MAX_DISPARITY
				if (pix_cnt > maxDisparity)
					break;
#endif

			}

			output_buffer[oline_idx] = disparity;
			oline_idx++;
			min = UINT_MAX;
			disparity = 0;
		}
		memcpy((uint32_t*)&addrOutput[c_row_idx_out], (uint32_t*)&output_buffer[0], oline_idx<<2);
		oline_idx = 0;
	}
	return 0;
}
