#include <CL/cl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

void handleClErr(cl_int errCode, const char *identifier, uint32_t line);
char *readKernelSource(const char *kernel, size_t *size);
uint32_t pickChoice(const char *title, uint32_t _default = 0);
cl_platform_id getPlatformId();
cl_device_id getDeviceId(cl_platform_id &platform);
