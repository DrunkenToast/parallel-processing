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

#define SIZE 50

int main(int argc, char **argv) {
  cl_int err;
  size_t kernel_size;
  int INPUT_A[SIZE];
  int INPUT_B[SIZE];
  int OUTPUT[SIZE];

  srand(time(0));

  for (int i = 0; i < SIZE; i++) {
    INPUT_A[i] = rand() % 10 + 1;
    INPUT_B[i] = rand() % 10 + 1;
  }

  cl_platform_id platform = getPlatformId();
  cl_device_id device = getDeviceId(platform);

  cl_context context = createContext(&device);
  cl_command_queue command_queue = createCommandQueue(context, device);

  // Create input and output buffers
  cl_mem input_a_mem_obj =
      clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * SIZE, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  cl_mem input_b_mem_obj =
      clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * SIZE, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  cl_mem output_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                         sizeof(int) * SIZE, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  // Input buffers
  err = clEnqueueWriteBuffer(command_queue, input_a_mem_obj, CL_TRUE, 0,
                             sizeof(int) * SIZE, INPUT_A, 0, NULL, NULL);
  err = clEnqueueWriteBuffer(command_queue, input_b_mem_obj, CL_TRUE, 0,
                             sizeof(int) * SIZE, INPUT_B, 0, NULL, NULL);

  // Read and build kernel
  const char *kernel_src = readKernelSource(KERNEL_FILE, &kernel_size);
  cl_program program =
      clCreateProgramWithSource(context, 1, &kernel_src, &kernel_size, &err);

  err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  getBuildInfo(program, device);
  handleClErr(err, "clBuildProgram", __LINE__, __FILE__);

  // Create kernel and set parameters
  cl_kernel kernel = clCreateKernel(program, "vector_mult", &err);
  handleClErr(err, "clCreateKernel", __LINE__, __FILE__);

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_a_mem_obj);
  handleClErr(err, "clSetKernelArg", __LINE__, __FILE__);
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &input_b_mem_obj);
  handleClErr(err, "clSetKernelArg", __LINE__, __FILE__);

  err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_mem_obj);
  handleClErr(err, "clSetKernelArg", __LINE__, __FILE__);

  std::cout << "> Executing..." << std::endl << std::endl;

  //
  size_t global_work_size = SIZE;
  size_t local_item_size = 1;
  err = clEnqueueNDRangeKernel(command_queue, kernel, 1, 0, &global_work_size,
                               &local_item_size, 0, NULL, NULL);

  err = clEnqueueReadBuffer(command_queue, output_mem_obj, CL_TRUE, 0,
                            sizeof(int) * SIZE, OUTPUT, 0, NULL, NULL);

  for (int i = 0; i < SIZE; i++) {
    std::cout << INPUT_A[i] << " * " << INPUT_B[i] << "\t = " << OUTPUT[i] << std::endl;
  }
  exit(0);
}
