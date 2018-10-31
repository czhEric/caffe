//For clarity,error checking has been omitted.
#include "./platform.h"
#include <CL/cl.hpp>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;


//create context
cl_context CreateContext()
{
    cl_int errNum;
    cl_uint numPlatForms;
    cl_platform_id firstPlatformId;
    cl_context context = NULL;

    //First, select an OpenCL platform to turn on.
    //For this example, we simply choose the first available platform.
    //Normally, you would query for all available platforms and select the most approprate one.
    errNum = clGetPlatformIDs(1, &firstPlatformId, &numPlatForms);
    cout << errNum << endl;
    cout << numPlatForms << endl;
    if(errNum != CL_SUCCESS || numPlatForms <= 0)
    {
        cerr << "Failed to find any OpenCL platforms." << endl;
        return NULL;
    }

    //Next, create an OpenCL context on the platform.
    //Attempt to create a GPU-based context, and if that fails, try to create a CPU-based context.
    cl_context_properties contextProperties[]=
    {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)firstPlatformId,
        0
    };
    context = clCreateContextFromType(contextProperties,
                                        CL_DEVICE_TYPE_GPU,
                                        NULL, NULL, &errNum);
    if(errNum != CL_SUCCESS)
    {
        cout << "Could not create GPU context, trying CPU..." <<endl;
        context = clCreateContextFromType(contextProperties,
                                            CL_DEVICE_TYPE_CPU,
                                            NULL, NULL, &errNum);
        if(errNum != CL_SUCCESS)
        {
            cerr << "Failed to create an OpenCL GPU or CPU context.";
            return NULL;
        }
    }
    return context;
}

cl_command_queue CreateCommandQueue(cl_context context,
                                    cl_device_id *device)
{
    cl_int errNum;
    cl_device_id *devices;
    cl_command_queue commandQueue = NULL;
    size_t deviceBufferSize = -1;

    //First get the size of the devices buffer
    int deviceIdx = 0; //use the first device
    errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceIdx, NULL, 
                                &deviceBufferSize);
    if(errNum != CL_SUCCESS)
    {
        cerr << "Failed call to clGetContextInfo(..., GL_CONTEXT_DEVICES,...)";
        return NULL;
    }

    if(deviceBufferSize <= 0)
    {
        cerr << "No devices available.";
        return NULL;
    }


    //Allocate memory for the devices buffer
    devices = new cl_device_id[deviceBufferSize / sizeof(cl_device_id)];
    errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBufferSize, 
                                devices, NULL);
    if(errNum != CL_SUCCESS)
    {
        cerr << "Failed to get device IDs";
        return NULL;
    }


    //In this example, we just choose the first available device.
    //In a real program, you would likely use all available devices or 
    //choose the highest performance device based on OpenCL device queries.
    commandQueue =  clCreateCommandQueue(context, devices[0], 0, NULL);
    if(commandQueue == NULL)
    {
        cerr << "Failed to create commandQueue for device 0";
        return NULL;
    }

    *device= devices[0];
    delete[] devices;
    return commandQueue;

}

cl_program CreateProgram(cl_context context, cl_device_id device, const char* fileName)
{
    cl_int errNum;
    cl_program program;

    ifstream kernelFile(fileName, ios::in);
    if(!kernelFile.is_open())
    {
        cerr << "Failed to open file for reading:" << fileName << endl;
        return NULL;
    }
    ostringstream oss;
    oss << kernelFile.rdbuf();

    string srcStdStr = oss.str();
    const char* srcStr = srcStdStr.c_str();
    //create program object
    program = clCreateProgramWithSource(context, 1, (const char**)&srcStr, NULL, NULL);

    if(program == NULL)
    {
        cerr << "Failed to create CL program from source." << endl;
        return NULL;
    }

    //build program
    errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(errNum != CL_SUCCESS)
    {
        //Determine the reason for the error
        char buildLog[16384];

        //if build failed, this function will collect all error message
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);

        cerr << "Error in kernel:" << endl;
        cerr << buildLog;
        clReleaseProgram(program);
        return NULL;
    }
    return program;
}



bool CreateMemObjects(cl_context context, cl_mem memObjects[3], float *a, float* b)
{
    memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                sizeof(float)*ARRAY_SIZE, a, NULL);
    memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                sizeof(float)*ARRAY_SIZE, b, NULL);
    memObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE,
                sizeof(float)*ARRAY_SIZE, NULL, NULL);

    if(memObjects[0] == NULL || memObjects[1] == NULL || memObjects[2] == NULL)
    {
        cerr << "Error creating memory objects." << endl;
        return false;
    }
    return true;
}

int main(int argc, char** argv)
{
    cl_context context = 0;
    cl_command_queue commandQueue = 0;
    cl_program program;
    cl_device_id device = 0;
    cl_kernel kernel = 0;
    cl_mem memObjects[3] = {0,0,0};
    cl_int errNum;

    //Create an OpenCL context on first available platform
    context = CreateContext();
    if(context == NULL)
    {
        cerr << "Failed to create OpenCL context." << endl;
        return 1;
    }

    //Create a command-queue on the first device available on the created context
    commandQueue = CreateCommandQueue(context, &device);
    if(commandQueue == NULL)
    {
        //Cleanup(context, commandQueue, program, kernel, memObjects);
        return -1;
    }

    //Create OpenCL program from helloworld.cl kernel source
    program = CreateProgram(context, device, "demo.cl");
    if(program == NULL)
    {
        //Cleanup(context, commandQueue, program, kernel, memObjects);
        return -1;
    }

    //Create OpenCL kernel
    kernel = clCreateKernel(program, "demo_kernel_", NULL);
    if(kernel == NULL)
    {
        cerr << "Failed to create kernel" << endl;
        //Cleanup(context, commandQueue, program, kernel, memObjects);
        return -1;
    }

    //Create memory objects that will be used as arguments to kernel.
    //First Create host memory arrays that will be used to store the arguments to the kernel
    float result[ARRAY_SIZE];
    float a[ARRAY_SIZE];
    float b[ARRAY_SIZE];
    for(int i = 0; i < ARRAY_SIZE; i++)
    {
        a[i] = (float)i;
        b[i] = (float)i * 2;
    }

    //create memory objects
    if(!CreateMemObjects(context, memObjects, a, b))
    {
        //Cleanup(context, commandQueue, program, kernel, memObjects);
        return -1;
    }

    //Set the kernel arguments(result, a, b)
    errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]); //a
    errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);//b
    errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);//result
    if(errNum != CL_SUCCESS)
    {
        cerr << "Error settint kernel arguments." << endl;
        //Cleanup(context, commandQueue, program, kernel, memObjects);
        return -1;
    }

    size_t globalWorkSize[1] = {ARRAY_SIZE};
    size_t localWorkSize[1] = {1};

    //Queue the kernel up for execution across the array
    cl_event eventPoint;
    errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
             globalWorkSize, localWorkSize, 0,
                NULL, &eventPoint);
    errNum |= clWaitForEvents(1, &eventPoint);
    if(errNum != CL_SUCCESS)
    {
        cerr << "Error queuing kernel for execution." << endl;
        //Cleanup(context, commandQueue, program, kernel, memObjects);
        return -1;
    }
    clReleaseEvent(eventPoint);

    //Read the output buffer back to the Host
    //if blocking_read is CL_TRUE, means that it won't return untill read is finished
    errNum = clEnqueueReadBuffer(commandQueue, memObjects[2], CL_TRUE/*blocking_read*/,
                 0, ARRAY_SIZE * sizeof(float), result, 0, NULL, NULL);
    if(errNum != CL_SUCCESS)
    {
        cerr << "Error reading result buffer." << endl;
        //Cleanup(context, commandQueue, program, kernel, memObjects);
        return -1;
    }

    //Output the result buffer
    for(int i = 0; i < ARRAY_SIZE; i++)
    {
        cout << result[i] << " ";
    }
    cout << endl;
    cout << "Executed program successfully." << endl;
    //Cleanup(context, commandQueue, program, kernel, memObjects);
    return 0;
}



