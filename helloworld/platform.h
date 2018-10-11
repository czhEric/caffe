#ifndef _PLATFORM_H_
#define _PLATFORM_H_
#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
cl_context CreateContext();
cl_command_queue CreateCommandQueue(cl_context context,
                                    cl_device_id *device);
                                    

#endif



