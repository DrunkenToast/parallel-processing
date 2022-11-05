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

int main(int argc, char **argv) {
  const cl_int SIZE = 16;
  cl_int err;
  size_t kernel_size;

  cl_float A[SIZE];
  cl_float B[SIZE];
  cl_float C[SIZE];

  srand(time(0));

  std::cout << std::endl;
  for (int i = 0; i < SIZE; i++) {
    A[i] = rand() % 51;
    B[i] = rand() % 51;
    std::cout << A[i] << " \t| \t" << B[i] << std::endl;
  }
  std::cout << std::endl;

  cl_platform_id platform = getPlatformId();
  cl_device_id device = getDeviceId(platform);

  cl_context context = createContext(&device);
  cl_command_queue command_queue = createCommandQueue(context, device);

  // Create input and output buffers
  cl_mem input_a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
                                          sizeof(cl_float) * SIZE, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  cl_mem input_b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
                                          sizeof(cl_float) * SIZE, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  cl_mem output_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                         sizeof(cl_float) * SIZE, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  // Input buffers
  err = clEnqueueWriteBuffer(command_queue, input_a_mem_obj, CL_TRUE, 0,
                             sizeof(cl_float) * SIZE, A, 0, NULL, NULL);
  err = clEnqueueWriteBuffer(command_queue, input_b_mem_obj, CL_TRUE, 0,
                             sizeof(cl_float) * SIZE, B, 0, NULL, NULL);

  // Read and build kernel
  const char *kernel_src = readKernelSource(KERNEL_FILE, &kernel_size);
  cl_program program =
      clCreateProgramWithSource(context, 1, &kernel_src, &kernel_size, &err);

  err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  getBuildInfo(program, device);
  handleClErr(err, "clBuildProgram", __LINE__, __FILE__);

  // Create kernel and set parameters
  // Calculate all averages
  cl_kernel kernel = clCreateKernel(program, "average", &err);
  handleClErr(err, "clCreateKernel", __LINE__, __FILE__);

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_a_mem_obj);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &input_b_mem_obj);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &output_mem_obj);
  handleClErr(err, "clSetKernelArg", __LINE__, __FILE__);

  std::cout << "> Executing..." << std::endl << std::endl;

  size_t global_work_size = SIZE;
  size_t local_item_size = 1;
  err = clEnqueueNDRangeKernel(command_queue, kernel, 1, 0, &global_work_size,
                               &local_item_size, 0, NULL, NULL);
  handleClErr(err, "clEnqueueNDRangeKernel", __LINE__, __FILE__);

  err = clEnqueueReadBuffer(command_queue, output_mem_obj, CL_TRUE, 0,
                            sizeof(cl_float) * SIZE, C, 0, NULL, NULL);
  handleClErr(err, "clEnqueueReadBuffer", __LINE__, __FILE__);

  for (int i = 0; i < SIZE; i++) {
    std::cout << C[i] << std::endl;
  }
  std::cout << std::endl;

  // ### Kernel passes for minimum ############################################

  int resultSize;
  global_work_size = 8;
  local_item_size = 2;

  resultSize = (global_work_size / local_item_size) * 2;

  cl_float *mins = (cl_float *)malloc(sizeof(cl_float) * resultSize);
  cl_float *maxs = (cl_float *)malloc(sizeof(cl_float) * resultSize);

  // Mem
  cl_mem input_mem = clCreateBuffer(context, CL_MEM_READ_ONLY,
                                    sizeof(cl_float) * SIZE, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  cl_mem mins_mem = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                   sizeof(cl_float) * resultSize, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  cl_mem maxs_mem = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                   sizeof(cl_float) * resultSize, NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__, __FILE__);

  // Input buffers
  err = clEnqueueWriteBuffer(command_queue, input_mem, CL_TRUE, 0,
                             sizeof(cl_float) * SIZE, C, 0, NULL, NULL);
  handleClErr(err, "clEnqueueWriteBuffer", __LINE__, __FILE__);

  // create kernel
  kernel = clCreateKernel(program, "minmaxavg", &err);
  handleClErr(err, "clSetKernelArg", __LINE__, __FILE__);

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_mem);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &mins_mem);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &maxs_mem);
  err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &maxs_mem); // change
  err |= clSetKernelArg(kernel, 4, sizeof(cl_int), &SIZE);
  handleClErr(err, "clSetKernelArg", __LINE__, __FILE__);

  // Enqueue 1
  err = clEnqueueNDRangeKernel(command_queue, kernel, 1, 0, &global_work_size,
                               &local_item_size, 0, NULL, NULL);
  handleClErr(err, "clEnqueueNDRangeKernel", __LINE__, __FILE__);

  // Read result!
  err = clEnqueueReadBuffer(command_queue, mins_mem, CL_TRUE, 0,
                            sizeof(cl_float) * resultSize, mins, 0, NULL, NULL);
  handleClErr(err, "clEnqueueReadBuffer", __LINE__, __FILE__);

  err = clEnqueueReadBuffer(command_queue, maxs_mem, CL_TRUE, 0,
                            sizeof(cl_float) * resultSize, maxs, 0, NULL, NULL);
  handleClErr(err, "clEnqueueReadBuffer", __LINE__, __FILE__);

  std::cout << "min: " << mins[0] << std::endl
            << "max: " << maxs[0] << std::endl;

  // free(mins);
  // free(maxs);

  exit(0);
}
