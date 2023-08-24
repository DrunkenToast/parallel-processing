/*
*	Parallel Processing Lab - Exam 2021
*	Source code for structural matching.
*
*	Optimize the find_pattern_offsets function with OpenCL.
*	Keep the reference code, so you can check whether the optimized code still works correct or not and to see how much faster the code becomes.
*
*	Use of former projects is allowed, as well as documentation about OpenCL (presentations, API and C spec are included in the project directory).
*	Use of the internet is prohibited! Being caught, means an automatic 0 for the entire course!
*
*	After the exam, make sure the entire solution is handed in.
*
*	Lars Struyf.
*/
#include "Header.h"

int main(void)
{
	for (int i = 0; i < 10; i++)
	{
		structMatchReference();
	}

	return 0;
}

int structMatchReference(void)
{
	const uint32_t temp_count = 5;

	for (uint32_t i = 0; i < DIM_W * DIM_H; i++)
		pixels[i] = 0;

	uint32_t width, height;
	st_coord min_x, min_y;


#ifdef _WIN32
	LARGE_INTEGER clock_begin;
	LARGE_INTEGER clock_end;
	LARGE_INTEGER frequency;
#elif defined __unix__
	timeval clock_begin;
	timeval clock_end;
	timezone tz;
	time_t timediff_sec;
	suseconds_t timediff_usec;
#endif

	// find bounding box
	pattern_dims(temp_count, temp_pnts, &min_x, &min_y, &width, &height);

	// make absolute
	//printf("w %d, h %d\n", width, height);
	//printf("Fixing pattern to make all positions relative to top left of template bounding box(x, y) > 0\n");
	for (uint32_t i = 0; i < temp_count; i++)
	{
		temp_pnts[i].x_coord -= min_x;
		temp_pnts[i].y_coord -= min_y;

		// generate absolute offset list
		offsets[i] = temp_pnts[i].x_coord + (temp_pnts[i].y_coord * DIM_W);
	}

	// inject test patterns
	st_position pos;
	pos.x_coord = 3;
	pos.y_coord = 4;

	inject_pattern(pos, temp_count, temp_pnts);

	pos.x_coord = 1234;
	pos.y_coord = 1234;

	inject_pattern(pos, temp_count, temp_pnts);

	uint32_t sa_width = DIM_W - width + 1;
	uint32_t sa_height = DIM_H - height + 1;

	// REAL kernel find it!
	//printf("Scanning pattern %d %d\n", sa_width, sa_height);

#ifdef _WIN32
	QueryPerformanceCounter(&clock_begin);
#elif defined __unix__
	gettimeofday(&clock_begin, &tz);
#endif

	//find_pattern(sa_width, sa_height, temp_count, temp_pnts, result);
	find_pattern_offsets(sa_width, sa_height, temp_count, offsets, result);

#ifdef _WIN32
	QueryPerformanceCounter(&clock_end);
	QueryPerformanceFrequency(&frequency);
#elif defined __unix__
	gettimeofday(&clock_end, &tz);
#endif

	// evaluate results
	for (uint32_t h = 0; h < sa_width; h++)
	{
		for (uint32_t w = 0; w < sa_height; w++)
		{
			if (result[h * sa_width + w] == temp_count * marker_value)
			{
				printf("Found pattern @ w %d, h %d\n", w, h);
			}
		}
	}

#ifdef _WIN32
	printf("Structural Match Reference:\nElapsed time: %lf ms\n\n", (double)(clock_end.QuadPart - clock_begin.QuadPart) / frequency.QuadPart * 1000);
#elif defined __unix__
	timediff_sec = (clock_end.tv_usec - clock_begin.tv_usec) > 0 ? (clock_end.tv_sec - clock_begin.tv_sec) : (clock_end.tv_sec - clock_begin.tv_sec) - 1;
	timediff_usec = (clock_end.tv_usec - clock_begin.tv_usec) > 0 ? (clock_end.tv_usec - clock_begin.tv_usec) : (clock_begin.tv_usec - clock_end.tv_usec);
	printf("Structural Match Reference:\nElapsed time: %lf ms\n\n", (double)timediff_usec / 1000);
#endif

	return 0;
}

void inject_pattern(st_position pos, uint32_t count, const st_position * pnts)
{
	for (uint32_t i = 0; i < count; i++)
	{
		uint32_t abspos = DIM_W * (pnts[i].y_coord + pos.y_coord) + pos.x_coord + pnts[i].x_coord;
		pixels[abspos] = marker_value;
	}
}

void pattern_dims(uint32_t count, st_position * pnts, int32_t *min_X, int32_t *min_Y, uint32_t *width, uint32_t *height)
{
#ifdef _WIN32
	int32_t min_x = INT32_MAX, min_y = INT32_MAX;
	int32_t max_x = INT32_MIN, max_y = INT32_MIN;
#elif defined __unix__
	int32_t min_x = INT_MAX, min_y = INT_MAX;
	int32_t max_x = INT_MIN, max_y = INT_MIN;
#endif

	for (uint32_t i = 0; i < count; i++)
	{
		if (pnts[i].x_coord < min_x)
			min_x = pnts[i].x_coord;
		if (pnts[i].y_coord < min_y)
			min_y = pnts[i].y_coord;

		if (pnts[i].x_coord > max_x)
			max_x = pnts[i].x_coord;
		if (pnts[i].y_coord > max_y)
			max_y = pnts[i].y_coord;
	}
	*width = max_x - min_x + 1;
	*height = max_y - min_y + 1;
	*min_X = min_x;
	*min_Y = min_y;
}

// The kernel to optimize
void find_pattern_offsets(uint32_t search_width, uint32_t search_height, uint32_t count, const uint32_t  * offsets, int32_t * res)
{
	for (uint32_t h = 0; h < search_height; h++)
	{
		for (uint32_t w = 0; w < search_width; w++)
		{
			int32_t base_idx = h * DIM_W + w; // current base position to put the pattern

			uint32_t sum = 0;
			for (uint32_t i = 0; i < count; i++)
			{
				int32_t abspos = base_idx + offsets[i];
				sum += pixels[abspos];
			}
			*res++ = sum;
			//printf("%d/%d: %d\n",  w, h, sum);
		}
	}
}
