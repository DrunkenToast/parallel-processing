#include "bmpfuncs.h"
#include <cstddef>
#include <opencv4/opencv2/imgcodecs.hpp>
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "OpenCL.lib")
#include <stdio.h>
#include <stdlib.h>

#include <CL/cl.h>
#include <cstdint>
#include <iostream>

// opencv 4.6.0-5
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

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

  cv::Mat mat = cv::imread("input.bmp", cv::IMREAD_GRAYSCALE);

  // Should always be continuous
  if (!mat.isContinuous()) {
    std::cout << "Data needs to be continuous" << std::endl;
  }

  int width = mat.cols, height = mat.rows;
    std::cout << width << "|" <<height << std::endl;

  const size_t IMGSIZE = sizeof(uchar) * width * height;
  uchar *imgIn = (uchar *)calloc(1, IMGSIZE);
  imgIn = mat.data;
  uchar *imgOut = (uchar *)calloc(1, IMGSIZE);

  // Create input and output buffers
  cl_image_desc desc = basicImage2DDesc(width, height);

  cl_mem imgInBuff;

  cl_image_format clImageFormat;
  clImageFormat.image_channel_order = CL_R;
  clImageFormat.image_channel_data_type = CL_UNSIGNED_INT8;

  // LoadImageFromOpenCV(context, mat, imgInBuff, desc, clImageFormat, err);
  uchar *data = mat.data;

  imgInBuff = clCreateImage(context, CL_MEM_READ_ONLY,
                              &clImageFormat, &desc, NULL, &err);

    size_t origin[3] = { 0, 0, 0 };
    size_t region[3] = { (size_t)width, (size_t)height, 1};

        clEnqueueWriteImage(command_queue, imgInBuff, CL_TRUE, origin, region, NULL, NULL, mat.data, NULL, NULL, NULL);

  handleClErr(err, "img", __LINE__, __FILE__);

  cl_mem imgOutBuff = clCreateImage(context, CL_MEM_WRITE_ONLY, &clImageFormat,
                                    &desc, NULL, &err);

  handleClErr(err, "create img", __LINE__, __FILE__);

  cl_sampler_properties props[7] = {CL_SAMPLER_NORMALIZED_COORDS,
                                    CL_FALSE,
                                    CL_SAMPLER_ADDRESSING_MODE,
                                    CL_ADDRESS_CLAMP_TO_EDGE,
                                    CL_SAMPLER_FILTER_MODE,
                                    CL_FILTER_NEAREST,
                                    0};
  cl_sampler sampler = clCreateSamplerWithProperties(context, props, NULL);

  // Input buffers
  // err = clEnqueueWriteBuffer(command_queue, imgInBuff, CL_TRUE, 0,
  // IMGSIZE,
  //                            imgIn, 0, NULL, NULL);
  // handleClErr(err, "write buffer", __LINE__, __FILE__);

  // Read and build kernel
  const char *kernel_src = readKernelSource(KERNEL_FILE, &kernel_size);
  cl_program program =
      clCreateProgramWithSource(context, 1, &kernel_src, &kernel_size, &err);

  err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  getBuildInfo(program, device);
  handleClErr(err, "clBuildProgram", __LINE__, __FILE__);

  // Create kernel and set parameters
  cl_kernel kernel = clCreateKernel(program, "sobel", &err);
  handleClErr(err, "clCreateKernel", __LINE__, __FILE__);

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &imgInBuff);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &imgOutBuff);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_sampler), &sampler);
  err |= clSetKernelArg(kernel, 3, sizeof(cl_int), &width);
  err |= clSetKernelArg(kernel, 4, sizeof(cl_int), &height);
  handleClErr(err, "clSetKernelArg", __LINE__, __FILE__);

  std::cout << "> Executing..." << std::endl << std::endl;

  size_t global_work_size[2] = {(size_t)width, (size_t)height};
  size_t local_item_size[2] = {10, 10}; // image devisible by 10/10

  cl_event event_timer;
  err = clEnqueueNDRangeKernel(command_queue, kernel, 2, // workdim is 2
                               0, global_work_size, local_item_size, 0, NULL,
                               &event_timer);

  clFinish(command_queue);

  err = clEnqueueReadImage(command_queue, imgOutBuff, CL_TRUE, origin, region, 0, 0, imgOut,
                            0, NULL, NULL);
    std::cout << "here" << std::endl;

  unsigned long starttime, endtime, elapsed;
  clGetEventProfilingInfo(event_timer, CL_PROFILING_COMMAND_START,
                          sizeof(cl_ulong), &starttime, NULL);
  clGetEventProfilingInfo(event_timer, CL_PROFILING_COMMAND_END,
                          sizeof(cl_ulong), &endtime, NULL);

  // storeImage(imgOut, "output.bmp", height, width, "input.bmp");
    // TODO: Left off here! Read data properly now that its an image2d
  mat.data = imgOut;
  cv::imwrite("output.bmp", mat);

  elapsed = endtime - starttime;

  std::cout << "elapsed time: " << elapsed << " ns | " << elapsed / 1000000
            << " ms | " << elapsed / 1000000000 << " s" << std::endl;

  free(imgOut);
  exit(0);
}
