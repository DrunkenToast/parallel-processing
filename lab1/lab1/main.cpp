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
  cl_int err;
  size_t kernel_size;

  std::string input_str = "";

  for (int i = 1; i < argc; i++) {
    input_str.append(argv[i]);
    input_str.append(" ");
  }

  std::cout << "Input string: " << input_str << std::endl
            << "length: " << sizeof(char) * input_str.length() << std::endl;

  cl_platform_id platform = getPlatformId();
  cl_device_id device = getDeviceId(platform);

  cl_context context = createContext(&device);
  cl_command_queue command_queue = createCommandQueue(context, device);

  // Create input and output buffers
  cl_mem input_mem_obj = clCreateBuffer(
      context, CL_MEM_READ_ONLY, sizeof(char) * input_str.length(), NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__);

  cl_mem output_mem_obj =
      clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                     sizeof(char) * input_str.length(), NULL, &err);
  handleClErr(err, "clCreateBuffer", __LINE__);

  // Write hello world to input buffer
  err = clEnqueueWriteBuffer(command_queue, input_mem_obj, CL_TRUE, 0,
                             sizeof(char) * input_str.length(),
                             input_str.c_str(), 0, NULL, NULL);

  // Read and build kernel
  const char *kernel_src = readKernelSource(KERNEL_FILE, &kernel_size);
  cl_program program =
      clCreateProgramWithSource(context, 1, &kernel_src, &kernel_size, &err);

  err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    getBuildInfo(program, device);
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
  size_t global_work_size = input_str.length();
  size_t local_item_size = 1;
  err = clEnqueueNDRangeKernel(command_queue, kernel, 1, 0, &global_work_size,
                               &local_item_size, 0, NULL, NULL);

  char *OUTPUT = (char *)malloc(sizeof(char) * input_str.length()) + 1;
  err = clEnqueueReadBuffer(command_queue, output_mem_obj, CL_TRUE, 0,
                            sizeof(char) * input_str.length(), OUTPUT, 0, NULL,
                            NULL);
  OUTPUT[input_str.length()] = '\0';
  std::cout << OUTPUT;

  exit(0);
}
