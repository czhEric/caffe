//For clarity,error checking has been omitted.
#include <CL/cl.h>
#include "tool.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
using namespace std;
#if 0
//sample 1
int main(int argc, char* argv[])
{
    cl_int    status;
    /**Step 1: Getting platforms and choose an available one(first).*/
    cl_platform_id platform;
    getPlatform(platform);

    /**Step 2:Query the platform and choose the first GPU device if has one.*/
    cl_device_id *devices=getCl_device_id(platform);

    /**Step 3: Create context.*/
    cl_context context = clCreateContext(NULL,1, devices,NULL,NULL,NULL);

    /**Step 4: Creating command queue associate with the context.*/
    cl_command_queue commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);

    /**Step 5: Create program object */
    const char *filename = "helloworld.cl";
    string sourceStr;
    status = convertToString(filename, sourceStr);
    const char *source  = sourceStr.c_str();
    size_t sourceSize[] = {strlen(source)};
    cl_program program = clCreateProgramWithSource(context, 1, &source, sourceSize, NULL);

    /**Step 6: Build program. */
    status=clBuildProgram(program, 1,devices,NULL,NULL,NULL);

    /**Step 7: Initial input,output for the host and create memory objects for the kernel*/
    const int NUM=512000;
    double* input = new double[NUM];
    for(int i=0;i<NUM;i++)
        input[i]=i;
    double* output = new double[NUM];

    cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, (NUM) * sizeof(double),(void *) input, NULL);
    cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY , NUM * sizeof(double), NULL, NULL);

    /**Step 8: Create kernel object */
    cl_kernel kernel = clCreateKernel(program,"helloworld", NULL);

    /**Step 9: Sets Kernel arguments.*/
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&inputBuffer);
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&outputBuffer);

    /**Step 10: Running the kernel.*/
    size_t global_work_size[1] = {NUM};
    cl_event enentPoint;
    status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, &enentPoint);
    clWaitForEvents(1,&enentPoint); ///wait
    clReleaseEvent(enentPoint);

    /**Step 11: Read the cout put back to host memory.*/
    status = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0, NUM * sizeof(double), output, 0, NULL, NULL);
    cout<<output[NUM-1]<<endl;

    /**Step 12: Clean the resources.*/
    status = clReleaseKernel(kernel);//*Release kernel.
    status = clReleaseProgram(program);    //Release the program object.
    status = clReleaseMemObject(inputBuffer);//Release mem object.
    status = clReleaseMemObject(outputBuffer);
    status = clReleaseCommandQueue(commandQueue);//Release  Command queue.
    status = clReleaseContext(context);//Release context.

    if (output != NULL)
    {
        free(output);
        output = NULL;
    }

    if (devices != NULL)
    {
        free(devices);
        devices = NULL;
    }
    return 0;
}
#else
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
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return -1;
    }

    //Create OpenCL program from helloworld.cl kernel source
    program = CreateProgram(context, device, "helloworld.cl");
    if(program == NULL)
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return -1;
    }

    //Create OpenCL kernel
    kernel = clCreateKernel(program, "hello_kernel", NULL);
    if(kernel == NULL)
    {
        cerr << "Failed to create kernel" << endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return -1;
    }

    //Create memory objects that will be used as arguments to kernel.
    //First Create host memory arrays that will be used to store the arguments to the kernel
    #define ARRAY_SIZE 100
    float result[ARRAY_SIZE];
    float a[ARRAY_SIZE];
    float b[ARRAY_SIZE];
    for(int i = 0; i < ARRAY_SIZE; i++)
    {
        a[i] = i;
        b[i] = i * 2;
    }

    if(!CreateMemObjects(context, memObjects, a, b))
    {
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return -1;
    }

    //Set the kernel arguments(result, a, b)
    errNum = clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]); //a
    errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);//b
    errNum |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);//result
    if(errNum != CL_SUCCESS)
    {
        cerr << "Error settint kernel arguments." << endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return -1;
    }

    size_t globalWorkSize[1] = {ARRAY_SIZE};
    size_t localWorkSize[1] = {1};

    //Queue the kernal up for execution across the array
    errNum = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL,
             globalWorkSize, localWorkSize, 0,
                NULL, NULL);
    if(errNum != CL_SUCCESS)
    {
        cerr << "Error queuing kernel for execution." << endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return -1;
    }

    //Read the output buffer back to the Host
    errNum = clEnqueueReadBuffer(commandQueue, memObjects[2], CL_TRUE/*blocking_read*/,
                 0, ARRAY_SIZE * sizeof(float), result, 0, NULL, NULL);
    if(errNum != CL_SUCCESS)
    {
        cerr << "Error reading result buffer." << endl;
        Cleanup(context, commandQueue, program, kernel, memObjects);
        return -1;
    }

    //Output the result buffer
    for(int i = 0; i < ARRAY_SIZE; i++)
    {
        cout << result[i] << " ";
    }
    cout << endl;
    cout << "Executed program successfully." << endl;
    Cleanup(context, commandQueue, program, kernel, memObjects);
    return 0;
}
#endif



