#include "bmpfuncs.h"
#include <cstddef>
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "OpenCL.lib")
#include <stdio.h>
#include <stdlib.h>

#include <CL/cl.h>
#include <cstdint>
#include <iostream>

#include "bmpfuncs.h"
#include "cl_helper.hpp"
#include <math.h>
#define KERNEL_FILE "kernel.cl"

int main(int argc, char **argv) {
  cl_int err;
  size_t kernel_size;

  cl_platform_id platform = getPlatformId();
  cl_device_id device = getDeviceId(platform);

  cl_context context = createContext(&device);
  cl_command_queue command_queue = createCommandQueue(context, device);

  const float angle = 63 * (M_PI/180);
  const float sinTheta = sin(angle);
  const float cosTheta = cos(angle);

  const int originX = 300, originY = 200;

  int width, height;
  float *imgIn = readImage("input.bmp", &width, &height);
  const size_t IMGSIZE = sizeof(float) * width * height;

  float *imgOut = (float *)malloc(IMGSIZE);

  // Create input and output buffers
  cl_mem imgInBuff =
      clCreateBuffer(context, CL_MEM_READ_ONLY, IMGSIZE, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  cl_mem imgOutBuff =
      clCreateBuffer(context, CL_MEM_WRITE_ONLY, IMGSIZE, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  // Input buffers
  err = clEnqueueWriteBuffer(command_queue, imgInBuff, CL_TRUE, 0, IMGSIZE,
                             imgIn, 0, NULL, NULL);

  // Read and build kernel
  const char *kernel_src = readKernelSource(KERNEL_FILE, &kernel_size);
  cl_program program =
      clCreateProgramWithSource(context, 1, &kernel_src, &kernel_size, &err);

  err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  getBuildInfo(program, device);
  handleClErr(err, "clBuildProgram", __LINE__, __FILE__);

  // Create kernel and set parameters
  cl_kernel kernel = clCreateKernel(program, "imgcpy", &err);
  handleClErr(err, "clCreateKernel", __LINE__, __FILE__);

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &imgInBuff);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &imgOutBuff);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_int), &width);
  err |= clSetKernelArg(kernel, 3, sizeof(cl_int), &height);

  err |= clSetKernelArg(kernel, 4, sizeof(float), &sinTheta);
  err |= clSetKernelArg(kernel, 5, sizeof(float), &cosTheta);

  err |= clSetKernelArg(kernel, 6, sizeof(cl_int), &originX);
  err |= clSetKernelArg(kernel, 7, sizeof(cl_int), &originY);
  handleClErr(err, "clSetKernelArg", __LINE__, __FILE__);

  std::cout << "> Executing..." << std::endl << std::endl;

  size_t global_work_size[2] = {(size_t)width, (size_t)height};
  size_t local_item_size[2] = {10, 10}; // image devisible by 10/10

  cl_event event_timer;

  err = clEnqueueNDRangeKernel(command_queue, kernel, 2, // workdim is 2
                               0, global_work_size, local_item_size, 0, NULL,
                               &event_timer);

  err = clEnqueueReadBuffer(command_queue, imgOutBuff, CL_TRUE, 0, IMGSIZE,
                            imgOut, 0, NULL, NULL);

  unsigned long starttime, endtime, elapsed;
  clGetEventProfilingInfo(event_timer, CL_PROFILING_COMMAND_START,
                          sizeof(cl_ulong), &starttime, NULL);
  clGetEventProfilingInfo(event_timer, CL_PROFILING_COMMAND_END,
                          sizeof(cl_ulong), &endtime, NULL);

  storeImage(imgOut, "output.bmp", height, width, "input.bmp");

  elapsed = endtime - starttime;

  std::cout << "elapsed time: " << elapsed << " ns | " << elapsed / 1000000
            << " ms | " << elapsed / 1000000000 << " s" << std::endl;

  exit(0);
}
