// Peter Leconte 3AD r0830684

#include "cl_helper.hpp"
#include <CL/cl.h>

char *readKernelSource(const char *kernel, size_t *size) {
  FILE *fp;
  char *source_str;

  fp = fopen(kernel, "rb");

  if (!fp) {
    return nullptr;
  }

  fseek(fp, 0, SEEK_END);
  uint32_t fileLength = ftell(fp);

  rewind(fp);

  source_str = (char *)malloc(fileLength + 1);
  fread(source_str, 1, sizeof(char) * (fileLength + 1), fp);
  source_str[fileLength] = '\0';

  *size = fileLength + 1;

  fclose(fp);

  std::cout << "> Kernel source file read (" << fileLength << " chars)"
            << std::endl;
  return source_str;
}

cl_platform_id getPlatformId() {
  const uint PLATFORM_ATRRIBUTE_COUNT = 4;
  const char *platformAttributeNames[PLATFORM_ATRRIBUTE_COUNT] = {
      "Name", "Vendor", "Version", "Profile" /*, "Extensions"*/};
  const cl_platform_info platformAttributeTypes[PLATFORM_ATRRIBUTE_COUNT] = {
      CL_PLATFORM_NAME, CL_PLATFORM_VENDOR, CL_PLATFORM_VERSION,
      CL_PLATFORM_PROFILE,
      // CL_PLATFORM_EXTENSIONS,
  };
  cl_int err;
  cl_uint numPlatforms;
  cl_platform_id *platforms;

  err = clGetPlatformIDs(0, NULL, &numPlatforms);
  handleClErr(err, "clGetPlatformIDs", __LINE__, __FILE__);

  platforms = (cl_platform_id *)malloc(numPlatforms * sizeof(cl_platform_id));
  err = clGetPlatformIDs(numPlatforms, platforms, NULL);
  handleClErr(err, "clGetPlatformIDs", __LINE__, __FILE__);

  std::cout << "=== Platforms ===" << std::endl;
  for (int i = 0; i < numPlatforms; i++) {
    std::cout << "Platform " << i << std::endl;
    for (int j = 0; j < PLATFORM_ATRRIBUTE_COUNT; j++) {
      size_t infoSize;
      char *info;
      clGetPlatformInfo(platforms[i], platformAttributeTypes[j], 0, NULL,
                        &infoSize);
      info = (char *)malloc(infoSize);

      clGetPlatformInfo(platforms[i], platformAttributeTypes[j], infoSize, info,
                        NULL);

      std::cout << "\t" << platformAttributeNames[j] << ": " << info
                << std::endl;
    }
  }

  cl_platform_id platform = platforms[pickChoice("Pick a platform")];
  free(platforms);
  return platform;
}

cl_device_id getDeviceId(cl_platform_id &platform) {
  const uint DEVICE_ATRRIBUTE_COUNT = 2;
  const char *deviceAttributeNames[DEVICE_ATRRIBUTE_COUNT] = {"Name", "Vendor", /* "Extensions", "Max compute units", "Max clock frequency", "Local memory size", "Global memory size"*/};
  const cl_platform_info deviceAttributeTypes[DEVICE_ATRRIBUTE_COUNT] = {
      CL_DEVICE_NAME, CL_DEVICE_VENDOR,
      // CL_DEVICE_EXTENSIONS,
      // CL_DEVICE_MAX_COMPUTE_UNITS,
      // CL_DEVICE_MAX_CLOCK_FREQUENCY,
      // CL_DEVICE_LOCAL_MEM_SIZE,
      // CL_DEVICE_GLOBAL_MEM_SIZE,
  };
  cl_int err;
  cl_uint numDevices;
  cl_device_id *devices;

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
  handleClErr(err, "clGetDeviceIDs", __LINE__, __FILE__);

  devices = (cl_device_id *)malloc(numDevices * sizeof(cl_platform_id));
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);
  handleClErr(err, "clGetDeviceIDs", __LINE__, __FILE__);

  std::cout << "=== Devices ===" << std::endl;
  for (int i = 0; i < numDevices; i++) {
    std::cout << "Device " << i << std::endl;
    for (int j = 0; j < DEVICE_ATRRIBUTE_COUNT; j++) {
      size_t infoSize;
      char *info;
      cl_int err;
      err = clGetDeviceInfo(devices[i], deviceAttributeTypes[j], 0, NULL,
                            &infoSize);
      handleClErr(err, "clGetDeviceInfo", __LINE__, __FILE__);

      info = (char *)malloc(infoSize);

      err = clGetDeviceInfo(devices[i], deviceAttributeTypes[j], infoSize, info,
                            NULL);
      handleClErr(err, "clGetDeviceInfo", __LINE__, __FILE__);

      std::cout << "\t" << deviceAttributeNames[j] << ": " << info << std::endl;
      free(info);
    }
  }

  cl_device_id device = devices[pickChoice("Pick a device")];
  free(devices);
  return device;
}

uint32_t pickChoice(const char *title, uint32_t _default) {
  uint32_t choice;
  std::cout << std::endl
            << "\033[32m"
            << "> " << title << " [default=" << _default << "]: "
            << "\033[0m";

  if (std::cin.peek() == '\n') {
    choice = _default;
  } else if (!(std::cin >> choice)) {
    std::cout << "Invalid input.\n";
  }

  std::cin.ignore(1);

  std::cout << std::endl;

  return choice;
}

void handleClErr(cl_int errCode, const char *identifier, uint32_t line,
                 const char *file) {
  if (errCode == CL_SUCCESS)
    return;
  std::cout << "Error code [" << errCode << "] with " << identifier << " at "
            << file << ":" << line << std::endl;
  exit(errCode);
}

void getBuildInfo(cl_program &program, cl_device_id &device) {
  size_t buildLogSize;
  char *log;
  cl_int err;

  std::cout << "=== Kernel build info ===" << std::endl;
  err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr,
                              &buildLogSize);
  handleClErr(err, "clGetProgramBuildInfo", __LINE__, __FILE__);

  log = (char *)malloc(buildLogSize);
  clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, buildLogSize,
                        log, nullptr);
  std::cout << "\033[31m" << log << "\033[0m" << std::endl;
  handleClErr(err, "clGetProgramBuildInfo", __LINE__, __FILE__);

  std::cout << "=== End build info ===" << std::endl << std::endl;
  free(log);
}

cl_context createContext(cl_device_id *device) {
  cl_int err;
  cl_context context = clCreateContext(NULL, 1, device, NULL, NULL, &err);
  handleClErr(err, "clCreateContext", __LINE__, __FILE__);
  return context;
}

cl_command_queue createCommandQueue(cl_context &context, cl_device_id &device) {
  cl_int err;
  cl_command_queue_properties properties[]{CL_QUEUE_PROPERTIES,
                                           CL_QUEUE_PROFILING_ENABLE, 0};
  cl_command_queue command_queue =
      clCreateCommandQueueWithProperties(context, device, properties, &err);
  handleClErr(err, "clCreateCommand", __LINE__, __FILE__);

  return command_queue;
}

void printTime(unsigned long elapsed, const char *title) {
    std::cout << title << ": " << elapsed << " ns | "
              << elapsed / (float)1000000 << " ms | "
              << elapsed / (float)1000000000 << " s" << std::endl;
    return;
}
