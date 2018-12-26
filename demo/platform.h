#ifndef _PLATFORM_H_
#define _PLATFORM_H_
#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

#define ARRAY_SIZE 100

cl_context CreateContext();

cl_command_queue CreateCommandQueue(cl_context context,
                                    cl_device_id *device);

cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName);

bool CreateMemObjects(cl_context context, cl_mem memObjects[3], float *a, float* b);
                                    

#endif



