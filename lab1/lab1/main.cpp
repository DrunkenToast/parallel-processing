#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "OpenCL.lib")
#include <stdio.h>
#include <stdlib.h>
 
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#include <cstdint>
#include <iostream>
 
#define MAX_SOURCE_SIZE (0x100000)
 
#define PLATFORM_ATRRIBUTE_COUNT 4
const char* platformAttributeNames[PLATFORM_ATRRIBUTE_COUNT] = { "Name", "Vendor", "Version", "Profile" /*, "Extensions"*/};
const cl_platform_info platformAttributeTypes[PLATFORM_ATRRIBUTE_COUNT] = {
	CL_PLATFORM_NAME,
	CL_PLATFORM_VENDOR,
	CL_PLATFORM_VERSION,
	CL_PLATFORM_PROFILE,
	//CL_PLATFORM_EXTENSIONS,
};

#define DEVICE_ATRRIBUTE_COUNT 7
const char* deviceAttributeNames[DEVICE_ATRRIBUTE_COUNT] = { "Name", "Vendor", "Extensions", "Max compute units", "Max clock frequency", "Local memory size", "Global memory size" /*, "Extensions"*/};
const cl_platform_info deviceAttributeTypes[DEVICE_ATRRIBUTE_COUNT] = {
    CL_DEVICE_NAME,
    CL_DEVICE_VENDOR,
    CL_DEVICE_EXTENSIONS,
    CL_DEVICE_MAX_COMPUTE_UNITS,
    CL_DEVICE_MAX_CLOCK_FREQUENCY,
    CL_DEVICE_LOCAL_MEM_SIZE,
    CL_DEVICE_GLOBAL_MEM_SIZE,
};

void handleClErr(cl_int errCode, const char* identifier, uint32_t line);
char* readKernelSource(const char* Kernel);
uint32_t pickChoice(const char* title, uint32_t _default = 0);

int main(void) {
	cl_int err;
	cl_uint numPlatforms = 0;
	cl_uint numDevices = 0;
	cl_platform_id* platforms;
	cl_device_id* devices;
	uint32_t platformChoice = 0;
	uint32_t deviceChoice = 0;
	char platformName[150];
	//oclC deviceContructs;

    err = clGetPlatformIDs(0, NULL, &numPlatforms);
    handleClErr(err, "clGetPlatformIDs", __LINE__);

	platforms = (cl_platform_id*)malloc(numPlatforms * sizeof(cl_platform_id));
    err = clGetPlatformIDs(numPlatforms, platforms, NULL);
    handleClErr(err, "clGetPlatformIDs", __LINE__);

    for (int i = 0; i < numPlatforms; i++) {
        std::cout << "Platform " << i << std::endl;
        for (int j = 0; j < PLATFORM_ATRRIBUTE_COUNT; j++) {
			// get platform attribute value size
			size_t infoSize;
            char* info;
			clGetPlatformInfo(platforms[i], platformAttributeTypes[j], 0, NULL, &infoSize);
			info = (char*)malloc(infoSize);

			// get platform attribute value
			clGetPlatformInfo(platforms[i], platformAttributeTypes[j], infoSize, info, NULL);

			//printf("  %d.%d %-11s: %s\n", i + 1, j + 1, attributeNames[j], info);
            std::cout << "\t" << platformAttributeNames[j] << ": " << info << std::endl;
		}
    }

    platformChoice = pickChoice("Pick a platform");

    err = clGetDeviceIDs(platforms[platformChoice], CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
    handleClErr(err, "clGetDeviceIDs", __LINE__);

	devices = (cl_device_id*)malloc(numDevices * sizeof(cl_platform_id));
    err = clGetDeviceIDs(platforms[platformChoice], CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);
    handleClErr(err, "clGetDeviceIDs", __LINE__);

    for (int i = 0; i < numDevices; i++) {
        std::cout << "Device " << i << std::endl;
        for (int j = 0; j < DEVICE_ATRRIBUTE_COUNT; j++) {
			// get platform attribute value size
			size_t infoSize;
            char* info;
			clGetDeviceInfo(devices[i], deviceAttributeTypes[j], 0, NULL, &infoSize);
			info = (char*)malloc(infoSize);

			// get platform attribute value
			clGetDeviceInfo(devices[i], deviceAttributeTypes[j], infoSize, info, NULL);

			//printf("  %d.%d %-11s: %s\n", i + 1, j + 1, attributeNames[j], info);
            std::cout << "\t" << deviceAttributeNames[j] << ": " << info << std::endl;
		}
    }

    //deviceChoice = pickChoice("Pick a device");

    free(devices);
    free(platforms);
    exit(0);
}

char* readKernelSource(const char* Kernel) {
    FILE* fp;
    char* source_str;
    
    fp = fopen(Kernel, "rb");

    if (!fp)
    {
        return nullptr;
    }

    fseek(fp, 0, SEEK_END);
    uint32_t fileLength = ftell(fp);
    
    source_str = (char*)malloc(fileLength);
    fread(source_str, 1, sizeof(char) * (fileLength + 1), fp);

    fclose(fp);

    std::cout << "Kernel loaded" << std::endl;
    return source_str;
}

uint32_t pickChoice(const char* title, uint32_t _default)
{
    uint32_t choice;
    std::cout << title << "[default=" << _default << "]: ";

	if (std::cin.peek() == '\n') { //check if next character is newline
        choice = _default; //and assign the default
    } else if (!(std::cin >> choice)) { //be sure to handle invalid input
        std::cout << "Invalid input.\n";
    }

	return choice;
}

void handleClErr(cl_int errCode, const char* identifier, uint32_t line) {
    if (errCode == CL_SUCCESS) return;
    std::cout << "Error code " << errCode << " with " << identifier << " at line " << line << std::endl;
    exit(1);
}