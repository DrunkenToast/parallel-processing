/*
*   Modified and solved by 
*   Peter Leconte 3AD r0830684
*/

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <climits>

#ifdef _WIN32
#include <Windows.h>
#elif defined __unix__
#include <sys/time.h>
#endif


#include <CL/cl.h>

#define DIM_W 5000
#define DIM_H 5000

typedef int32_t st_coord;

typedef struct
{
	st_coord x_coord; /*!< in subpixels */
	st_coord y_coord; /*!< in subpixels */
}
st_position;

// cross shape
st_position temp_pnts[5] = { {0,5}, {0,-5}, {0,0}, {5, 0}, {-5, 0} };
uint32_t offsets[5];

const uint8_t marker_value = 100;

// x86 Arrays
uint8_t pixels[DIM_W * DIM_H];
int32_t result[DIM_W * DIM_H]; /* max result dim */

// x86 functions
int structMatchReference();
int structMatchOpenCl(cl_device_id &device, cl_context &context);
void inject_pattern(st_position pos, uint32_t count, const st_position * pnts);
void pattern_dims(uint32_t count, st_position * pnts, int32_t *min_X, int32_t *min_Y, uint32_t *width, uint32_t *height);
void find_pattern(uint32_t search_width, uint32_t search_height, uint32_t count, const st_position * pnts, int32_t * res);
void find_pattern_offsets(uint32_t search_width, uint32_t search_height, uint32_t count, const uint32_t  * offsets, int32_t * res);
