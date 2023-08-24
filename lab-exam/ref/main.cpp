#include <CL/cl_platform.h>
#include <cstddef>
#include <cstdlib>
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "OpenCL.lib")
#include <stdio.h>
#include <stdlib.h>

#include <CL/cl.h>
#include <cstdint>
#include <iostream>

#include "cl_helper.hpp"
#include <string.h>
#define KERNEL_FILE "kernel.cl"

// TODO: Additional exercise speed mem
// TODO: Coalescent access
//
void randomizeBodies(cl_float4 *data, int n) {
  for (int i = 0; i < n; i++) {
    data[i].x = 2.0f * (rand() / (float)RAND_MAX) - 1.0f;
    data[i].y = 2.0f * (rand() / (float)RAND_MAX) - 1.0f;
    data[i].z = 2.0f * (rand() / (float)RAND_MAX) - 1.0f;
  }
}

#define LOCAL_SIZE 250

int main(int argc, char **argv) {
  cl_int err;
  size_t kernel_size;

  const int nBodies = 300000;
  const float dt = 0.01f;
  const int nIters = 10;

  const size_t dataSize = sizeof(cl_float4) * nBodies;

  cl_float4 *pos = (cl_float4 *)malloc(dataSize);
  cl_float4 *vel = (cl_float4 *)malloc(dataSize);

  srand(1);

  randomizeBodies(pos, nBodies);
  randomizeBodies(vel, nBodies);

  cl_platform_id platform = getPlatformId();
  cl_device_id device = getDeviceId(platform);

  cl_context context = createContext(&device);
  cl_command_queue command_queue = createCommandQueue(context, device);

  // Create input and output buffers
  cl_mem input_a_mem_obj =
      clCreateBuffer(context, CL_MEM_READ_WRITE, dataSize, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  cl_mem input_b_mem_obj =
      clCreateBuffer(context, CL_MEM_READ_WRITE, dataSize, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  // Input buffers
  err = clEnqueueWriteBuffer(command_queue, input_a_mem_obj, CL_TRUE, 0,
                             dataSize, pos, 0, NULL, NULL);
  err = clEnqueueWriteBuffer(command_queue, input_b_mem_obj, CL_TRUE, 0,
                             dataSize, vel, 0, NULL, NULL);

  // Read and build kernel
  const char *kernel_src = readKernelSource(KERNEL_FILE, &kernel_size);
  cl_program program =
      clCreateProgramWithSource(context, 1, &kernel_src, &kernel_size, &err);

  err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  getBuildInfo(program, device);
  handleClErr(err, "clBuildProgram", __LINE__, __FILE__);

  // Create kernel and set parameters
  cl_kernel kernel = clCreateKernel(program, "nbody", &err);
  handleClErr(err, "clCreateKernel", __LINE__, __FILE__);

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_a_mem_obj);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &input_b_mem_obj);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_float4) * LOCAL_SIZE, NULL);
  err |= clSetKernelArg(kernel, 3, sizeof(float), &dt);
  err |= clSetKernelArg(kernel, 4, sizeof(int), &nBodies);
  // err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_mem_obj);
  // err |= clSetKernelArg(kernel, 3, sizeof(cl_int), &A_WIDTH);
  // err |= clSetKernelArg(kernel, 4, sizeof(cl_int), &A_HEIGHT);
  // err |= clSetKernelArg(kernel, 5, sizeof(cl_int), &B_WIDTH);
  // err |= clSetKernelArg(kernel, 6, sizeof(cl_int), &B_HEIGHT);
  handleClErr(err, "clSetKernelArg", __LINE__, __FILE__);

  std::cout << "> Executing..." << std::endl << std::endl;

  size_t global_work_size = nBodies;
  size_t local_work_size = LOCAL_SIZE;

  cl_event event_timer;

  int tests = 10;
  unsigned long total = 0;

  for (int i = 0; i < tests; i++) {

    err = clEnqueueNDRangeKernel(command_queue, kernel, 1, 0, &global_work_size,
                                 &local_work_size, 0, NULL, &event_timer);

    // clFinish(command_queue);

    err = clEnqueueReadBuffer(command_queue, input_a_mem_obj, CL_TRUE, 0,
                              dataSize, pos, 0, NULL, NULL);
    err = clEnqueueReadBuffer(command_queue, input_b_mem_obj, CL_TRUE, 0,
                              dataSize, vel, 0, NULL, NULL);

    unsigned long starttime, endtime;

    clGetEventProfilingInfo(event_timer, CL_PROFILING_COMMAND_START,
                            sizeof(cl_ulong), &starttime, NULL);
    clGetEventProfilingInfo(event_timer, CL_PROFILING_COMMAND_END,
                            sizeof(cl_ulong), &endtime, NULL);
    ulong elapsed = endtime - starttime;
    if (i > 0) { // skip first cold run
      total += elapsed;
      printTime(elapsed);
    } else {
      printTime(elapsed, "Cold run time");
    }
  }
  printTime(total / 9, "Average time");

  for (int i = 0; i < 10; i++) {
    std::cout << pos[i * 100].x << " " << pos[i * 100].y << " "
              << pos[i * 100].z << std::endl;
  }

  free(pos);
  free(vel);
  return 0;
}
