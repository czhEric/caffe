//For clarity,error checking has been omitted.
#include "platform.h"
#include <CL/cl.hpp>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

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



