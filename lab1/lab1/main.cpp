#include <cstddef>
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "OpenCL.lib")
#include <stdio.h>
#include <stdlib.h>

#include <CL/cl.h>
#include <cstdint>
#include <iostream>

#include "cl_helper.hpp"
#define KERNEL_FILE "kernel.cl"

int main(void) {
  cl_int err;
  size_t kernel_size;
  const char INPUT[] = "Hello world!\n";

  cl_platform_id platform = getPlatformId();
  cl_device_id device = getDeviceId(platform);

  // Create context
  cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
  handleClErr(err, "clCreateContext", __LINE__);

  // Create command queue
  cl_command_queue command_queue =
      clCreateCommandQueueWithProperties(context, device, NULL, &err);
  handleClErr(err, "clCreateCommand", __LINE__);

  // Create input and output buffers
  cl_mem input_mem_obj =
      clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(INPUT), NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__);

  cl_mem output_mem_obj =
      clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(INPUT), NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__);

  // Write hello world to input buffer
  err = clEnqueueWriteBuffer(command_queue, input_mem_obj, CL_TRUE, 0,
                             sizeof(INPUT), INPUT, 0, NULL, NULL);

  // Read and build kernel
  const char *kernel_src = readKernelSource(KERNEL_FILE, &kernel_size);
  cl_program program =
      clCreateProgramWithSource(context, 1, &kernel_src, &kernel_size, &err);

  err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  handleClErr(err, "clBuildProgram", __LINE__);

  // Create kernel and set parameters
  cl_kernel kernel = clCreateKernel(program, "hello_world", &err);
  handleClErr(err, "clCreateKernel", __LINE__);

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_mem_obj);
  handleClErr(err, "clSetKernelArg", __LINE__);
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_mem_obj);
  handleClErr(err, "clSetKernelArg", __LINE__);

  std::cout << "> Executing..." << std::endl << std::endl;

  /* Global work size is the amount of characters, each worker gets one
   * Because of this the item size is 1 (char) */
  size_t global_work_size = sizeof(INPUT);
  size_t local_item_size = 1;
  err = clEnqueueNDRangeKernel(command_queue, kernel, 1, 0, &global_work_size,
                               &local_item_size, 0, NULL, NULL);

  char *OUTPUT = (char *)malloc(sizeof(INPUT));
  err = clEnqueueReadBuffer(command_queue, output_mem_obj, CL_TRUE, 0,
                            sizeof(INPUT), OUTPUT, 0, NULL, NULL);
  std::cout << OUTPUT;

  exit(0);
}
