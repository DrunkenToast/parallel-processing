#include <cstddef>
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

int main(int argc, char **argv) {
  cl_int err;
  size_t kernel_size;

  const cl_int A_WIDTH = 6000;
  const cl_int A_HEIGHT = 150;

  const cl_int B_WIDTH = 400;
  const cl_int B_HEIGHT = 6000;

  const cl_int C_WIDTH = B_WIDTH;
  const cl_int C_HEIGHT = A_HEIGHT;

  if (A_WIDTH != B_HEIGHT) {
    std::cout << "Matrix sizes wrong";
    exit(1);
  }

  cl_int A[A_HEIGHT][A_WIDTH];
  cl_int B[B_HEIGHT][B_WIDTH];

  cl_int C[C_HEIGHT][C_WIDTH] = {0};

  const size_t A_SIZE = sizeof(cl_int) * A_WIDTH * A_HEIGHT;
  const size_t B_SIZE = sizeof(cl_int) * B_WIDTH * B_HEIGHT;
  const size_t C_SIZE = sizeof(cl_int) * C_WIDTH * C_HEIGHT;

  srand(time(0));

  std::cout << std::endl;
  for (int j = 0; j < A_HEIGHT; j++) {
    for (int i = 0; i < A_WIDTH; i++) {
      A[j][i] = rand() % 5 + 1;
      std::cout << A[j][i] << "\t";
    }
    std::cout << std::endl;
  }

  std::cout << std::endl;
  for (int j = 0; j < B_HEIGHT; j++) {
    for (int i = 0; i < B_WIDTH; i++) {
      B[j][i] = rand() % 5 + 1;
      std::cout << B[j][i] << "\t";
    }
    std::cout << std::endl;
  }

  cl_platform_id platform = getPlatformId();
  cl_device_id device = getDeviceId(platform);

  cl_context context = createContext(&device);
  cl_command_queue command_queue = createCommandQueue(context, device);

  // Create input and output buffers
  cl_mem input_a_mem_obj =
      clCreateBuffer(context, CL_MEM_READ_ONLY, A_SIZE, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  cl_mem input_b_mem_obj =
      clCreateBuffer(context, CL_MEM_READ_ONLY, B_SIZE, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  cl_mem output_mem_obj =
      clCreateBuffer(context, CL_MEM_WRITE_ONLY, C_SIZE, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  // Input buffers
  err = clEnqueueWriteBuffer(command_queue, input_a_mem_obj, CL_TRUE, 0, A_SIZE,
                             A, 0, NULL, NULL);
  err = clEnqueueWriteBuffer(command_queue, input_b_mem_obj, CL_TRUE, 0, B_SIZE,
                             B, 0, NULL, NULL);

  // Read and build kernel
  const char *kernel_src = readKernelSource(KERNEL_FILE, &kernel_size);
  cl_program program =
      clCreateProgramWithSource(context, 1, &kernel_src, &kernel_size, &err);

  err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  getBuildInfo(program, device);
  handleClErr(err, "clBuildProgram", __LINE__, __FILE__);

  // Create kernel and set parameters
  cl_kernel kernel = clCreateKernel(program, "matrix", &err);
  handleClErr(err, "clCreateKernel", __LINE__, __FILE__);

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_a_mem_obj);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &input_b_mem_obj);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_mem_obj);
  err |= clSetKernelArg(kernel, 3, sizeof(cl_int), &A_WIDTH);
  err |= clSetKernelArg(kernel, 4, sizeof(cl_int), &B_WIDTH);
  handleClErr(err, "clSetKernelArg", __LINE__, __FILE__);

  std::cout << "> Executing..." << std::endl << std::endl;

  size_t global_work_size[2] = {C_WIDTH, C_HEIGHT};
  size_t local_item_size[2] = {1, 1};

  cl_event event_timer;

  err = clEnqueueNDRangeKernel(command_queue, kernel, 2, // workdim is 2
                               0, global_work_size, local_item_size, 0, NULL,
                               &event_timer);

  err = clEnqueueReadBuffer(command_queue, output_mem_obj, CL_TRUE, 0, C_SIZE,
                            C, 0, NULL, NULL);

  unsigned long starttime, endtime, elapsed;
  clGetEventProfilingInfo(event_timer, CL_PROFILING_COMMAND_START,
                          sizeof(cl_ulong), &starttime, NULL);
  clGetEventProfilingInfo(event_timer, CL_PROFILING_COMMAND_END,
                          sizeof(cl_ulong), &endtime, NULL);
  for (int i = 0; i < C_HEIGHT; i++) {
    for (int j = 0; j < C_WIDTH; j++) {
      std::cout << C[i][j] << "\t";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;

    elapsed = endtime - starttime;

  std::cout << "elapsed time: " << elapsed << std::endl;

  exit(0);
}
