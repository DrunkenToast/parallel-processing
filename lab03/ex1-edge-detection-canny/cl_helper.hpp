#pragma once
#include <CL/cl.h>
#include <iostream>
#include <opencv4/opencv2/core/mat.hpp>
#include <stdio.h>
#include <stdlib.h>

char *readKernelSource(const char *kernel, size_t *size);
uint32_t pickChoice(const char *title, uint32_t _default = 0);
cl_platform_id getPlatformId();
cl_device_id getDeviceId(cl_platform_id &platform);
void handleClErr(cl_int errCode, const char *identifier, uint32_t line,
                 const char *file);
void getBuildInfo(cl_program &program, cl_device_id &device);
cl_context createContext(cl_device_id *device);
cl_command_queue createCommandQueue(cl_context &context, cl_device_id &device);
cl_image_desc basicImage2DDesc(size_t width, size_t height);
