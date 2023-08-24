/*
 *	Parallel Processing Lab - Exam 2021
 *	Source code for structural matching.
 *
 *	Optimize the find_pattern_offsets function with OpenCL.
 *	Keep the reference code, so you can check whether the optimized code
 *still works correct or not and to see how much faster the code becomes.
 *
 *	Use of former projects is allowed, as well as documentation about OpenCL
 *(presentations, API and C spec are included in the project directory). Use of
 *the internet is prohibited! Being caught, means an automatic 0 for the entire
 *course!
 *
 *	After the exam, make sure the entire solution is handed in.
 *
 *	Lars Struyf.
 */

/*
*   Modified and solved by 
*   Peter Leconte 3AD r0830684
*
*   README.md contains more details about my findings and reasoning.
*
*   Make sure USE_OPENCL is defined to use my OpenCL code.
*   Reference code runs when undefined
*
*   OpenCL 3.0
*/
#define USE_OPENCL

#include "Header.h"
#include "cl_helper.hpp"
#include <cstdint>

#define KERNEL_FILE "kernel.cl"

int main(void) {
#ifdef USE_OPENCL
  cl_platform_id platform = getPlatformId();
  cl_device_id device = getDeviceId(platform);

  cl_context context = createContext(&device);
#endif

  for (int i = 0; i < 10; i++) {
#ifdef USE_OPENCL
    structMatchOpenCl(device, context);
#else
    structMatchReference();
#endif
  }

  return 0;
}

int structMatchReference(void) {
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
  struct timezone tz;
  time_t timediff_sec;
  suseconds_t timediff_usec;
#endif

  // find bounding box
  pattern_dims(temp_count, temp_pnts, &min_x, &min_y, &width, &height);

  // make absolute
  // printf("w %d, h %d\n", width, height);
  // printf("Fixing pattern to make all positions relative to top left of
  // template bounding box(x, y) > 0\n");
  for (uint32_t i = 0; i < temp_count; i++) {
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
  // printf("Scanning pattern %d %d\n", sa_width, sa_height);

#ifdef _WIN32
  QueryPerformanceCounter(&clock_begin);
#elif defined __unix__
  gettimeofday(&clock_begin, &tz);
#endif

  // find_pattern(sa_width, sa_height, temp_count, temp_pnts, result);
  find_pattern_offsets(sa_width, sa_height, temp_count, offsets, result);

#ifdef _WIN32
  QueryPerformanceCounter(&clock_end);
  QueryPerformanceFrequency(&frequency);
#elif defined __unix__
  gettimeofday(&clock_end, &tz);
#endif

  // evaluate results
  for (uint32_t h = 0; h < sa_width; h++) {
    for (uint32_t w = 0; w < sa_height; w++) {
      if (result[h * sa_width + w] == temp_count * marker_value) {
        printf("Found pattern @ w %d, h %d\n", w, h);
      }
    }
  }

#ifdef _WIN32
  printf("Structural Match Reference:\nElapsed time: %lf ms\n\n",
         (double)(clock_end.QuadPart - clock_begin.QuadPart) /
             frequency.QuadPart * 1000);
#elif defined __unix__
  timediff_sec = (clock_end.tv_usec - clock_begin.tv_usec) > 0
                     ? (clock_end.tv_sec - clock_begin.tv_sec)
                     : (clock_end.tv_sec - clock_begin.tv_sec) - 1;
  timediff_usec = (clock_end.tv_usec - clock_begin.tv_usec) > 0
                      ? (clock_end.tv_usec - clock_begin.tv_usec)
                      : (clock_begin.tv_usec - clock_end.tv_usec);
  printf("Structural Match Reference:\nElapsed time: %lf ms\n\n",
         (double)timediff_usec / 1000);
#endif

  return 0;
}

void inject_pattern(st_position pos, uint32_t count, const st_position *pnts) {
  for (uint32_t i = 0; i < count; i++) {
    uint32_t abspos =
        DIM_W * (pnts[i].y_coord + pos.y_coord) + pos.x_coord + pnts[i].x_coord;
    pixels[abspos] = marker_value;
  }
}

int structMatchOpenCl(cl_device_id &device, cl_context &context) {

  // init from ref
  const uint32_t temp_count = 5;

  for (uint32_t i = 0; i < DIM_W * DIM_H; i++)
    pixels[i] = 0;

  uint32_t width, height;
  st_coord min_x, min_y;

  // find bounding box
  pattern_dims(temp_count, temp_pnts, &min_x, &min_y, &width, &height);

  // make absolute
  // printf("w %d, h %d\n", width, height);
  // printf("Fixing pattern to make all positions relative to top left of
  // template bounding box(x, y) > 0\n");
  for (uint32_t i = 0; i < temp_count; i++) {
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

  // end of init
  std::cout << sa_width << " x " << sa_height << std::endl;

  cl_int err;
  size_t kernel_size;

  cl_command_queue command_queue = createCommandQueue(context, device);

  size_t pixels_size =
      sizeof(uint8_t) * DIM_H * DIM_W; // uint8 size == uchar size
  size_t offset_size = sizeof(uint32_t) * temp_count;
  size_t result_size = sizeof(int32_t) * DIM_H * DIM_W;

  // Create input and output buffers
  cl_mem input_pixels_mem_obj =
      clCreateBuffer(context, CL_MEM_READ_ONLY, pixels_size, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  cl_mem input_offsets_mem_obj =
      clCreateBuffer(context, CL_MEM_READ_ONLY, offset_size, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  cl_mem input_result_mem_obj =
      clCreateBuffer(context, CL_MEM_WRITE_ONLY, result_size, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  // Input buffers
  err = clEnqueueWriteBuffer(command_queue, input_pixels_mem_obj, CL_TRUE, 0,
                             pixels_size, pixels, 0, NULL, NULL);
  err = clEnqueueWriteBuffer(command_queue, input_offsets_mem_obj, CL_TRUE, 0,
                             offset_size, offsets, 0, NULL, NULL);
  err = clEnqueueWriteBuffer(command_queue, input_result_mem_obj, CL_TRUE, 0,
                             result_size, result, 0, NULL, NULL);

  // Read and build kernel
  const char *kernel_src = readKernelSource(KERNEL_FILE, &kernel_size);
  cl_program program =
      clCreateProgramWithSource(context, 1, &kernel_src, &kernel_size, &err);

  err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  getBuildInfo(program, device);
  handleClErr(err, "clBuildProgram", __LINE__, __FILE__);

  // Create kernel and set parameters
  cl_kernel kernel = clCreateKernel(program, "findPatternOffsets", &err);
  handleClErr(err, "clCreateKernel", __LINE__, __FILE__);

  int dim_width = DIM_W;

  err = clSetKernelArg(kernel, 0, sizeof(uint32_t), &sa_width);
  err |= clSetKernelArg(kernel, 1, sizeof(uint32_t), &dim_width);
  err |= clSetKernelArg(kernel, 2, sizeof(uint32_t), &temp_count);
  err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &input_pixels_mem_obj);
  err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &input_offsets_mem_obj);
  err |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &input_result_mem_obj);
  err |= clSetKernelArg(kernel, 6, sizeof(offset_size), NULL);
  // err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_mem_obj);
  // err |= clSetKernelArg(kernel, 3, sizeof(cl_int), &A_WIDTH);
  // err |= clSetKernelArg(kernel, 4, sizeof(cl_int), &A_HEIGHT);
  // err |= clSetKernelArg(kernel, 5, sizeof(cl_int), &B_WIDTH);
  // err |= clSetKernelArg(kernel, 6, sizeof(cl_int), &B_HEIGHT);
  handleClErr(err, "clSetKernelArg", __LINE__, __FILE__);

  std::cout << "> Executing..." << std::endl << std::endl;

  size_t global_work_size[2] = {(size_t)sa_width, (size_t)sa_height};
  size_t local_work_size[2] = {(size_t)5, (size_t)1}; // TODO

  cl_event event_timer;

  err = clEnqueueNDRangeKernel(command_queue, kernel, 2, 0, global_work_size,
                               NULL, 0, NULL, &event_timer);
  handleClErr(err, "Enqueue kernel", __LINE__, __FILE__);

  // clFinish(command_queue);

  // err = clEnqueueReadBuffer(command_queue, input_offsets_mem_obj, CL_TRUE, 0,
  //                           offset_size, offsets, 0, NULL, NULL);
  err = clEnqueueReadBuffer(command_queue, input_result_mem_obj, CL_TRUE, 0,
                            result_size, result, 0, NULL, NULL);

  unsigned long starttime, endtime;

  clGetEventProfilingInfo(event_timer, CL_PROFILING_COMMAND_START,
                          sizeof(cl_ulong), &starttime, NULL);
  clGetEventProfilingInfo(event_timer, CL_PROFILING_COMMAND_END,
                          sizeof(cl_ulong), &endtime, NULL);
  ulong elapsed = endtime - starttime;
  printTime(elapsed);

  // End of ref
  // evaluate results
  std::cout << "Evaluating results:" << std::endl;

  for (uint32_t h = 0; h < sa_height; h++) {
    for (uint32_t w = 0; w < sa_width; w++) {
      if (result[h * sa_width + w] == temp_count * marker_value) {
        printf("Found pattern @ w %d, h %d\n", w, h);
      }
    }
  }

  // TODO: Free!

  return 0;
}

void pattern_dims(uint32_t count, st_position *pnts, int32_t *min_X,
                  int32_t *min_Y, uint32_t *width, uint32_t *height) {
#ifdef _WIN32
  int32_t min_x = INT32_MAX, min_y = INT32_MAX;
  int32_t max_x = INT32_MIN, max_y = INT32_MIN;
#elif defined __unix__
  int32_t min_x = INT_MAX, min_y = INT_MAX;
  int32_t max_x = INT_MIN, max_y = INT_MIN;
#endif

  for (uint32_t i = 0; i < count; i++) {
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
void find_pattern_offsets(uint32_t search_width, uint32_t search_height,
                          uint32_t count, const uint32_t *offsets,
                          int32_t *res) {
  for (uint32_t h = 0; h < search_height; h++) {
    for (uint32_t w = 0; w < search_width; w++) {
      int32_t base_idx =
          h * DIM_W + w; // current base position to put the pattern

      uint32_t sum = 0;
      for (uint32_t i = 0; i < count; i++) {
        int32_t abspos = base_idx + offsets[i];
        sum += pixels[abspos];
      }
      *res++ = sum;
      // printf("%d/%d: %d\n",  w, h, sum);
    }
  }
}
