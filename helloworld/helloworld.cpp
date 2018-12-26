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
int main(int argc, char* argv[])
{
    int stepIdx = 0;
    
    cl_int    status;
    /**Step 1: Getting platforms and choose an available one(first).*/
    cl_platform_id platform;
    getPlatform(platform);

    cout  << "step " << stepIdx << " passed!" << endl;
    stepIdx ++;

    /**Step 2:Query the platform and choose the first GPU device if has one.*/
    cl_device_id *devices=getCl_device_id(platform);

    cout  << "step " << stepIdx << " passed!" << endl;
    stepIdx ++;

    /**Step 3: Create context.*/
    cl_context context = clCreateContext(NULL,1, devices,NULL,NULL,NULL);
    if(context == NULL)
    {
        cerr << "Failed to create OpenCL context." << endl;
        return 1;
    }
    cout  << "step " << stepIdx << " passed!" << endl;
    stepIdx ++;

    /**Step 4: Creating command queue associate with the context.*/
    cl_command_queue commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);
    if(commandQueue == NULL)
    {
        cerr << "Failed to create commandQueue for device 0";
        return -1;
    }
    cout  << "step " << stepIdx << " passed!" << endl;
    stepIdx ++;

    /**Step 5: Create program object */
    const char *filename = "helloworld.cl";
    string sourceStr;
    status = convertToString(filename, sourceStr);
    const char *source = sourceStr.c_str();
    size_t sourceSize[] = {strlen(source)};
    cl_program program = clCreateProgramWithSource(context, 1, &source, sourceSize, NULL);
    if(program == NULL)
    {
        cerr << "Failed to create CL program from source." << endl;
        return -1;
    }
    cout  << "step " << stepIdx << " passed!" << endl;
    stepIdx ++;

    /**Step 6: Build program. */
    status=clBuildProgram(program, 1,devices,NULL,NULL,NULL);
    if(status != CL_SUCCESS)
    {
        //Determine the reason for the error
        char buildLog[16384];

        //if build failed, this function will collect all error message
        clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog,  NULL);

        cerr << "Error in kernel:" << endl;
        cerr << buildLog;
        clReleaseProgram(program);
        return -1;
    }
    cout  << "step " << stepIdx << " passed!" << endl;
    stepIdx ++;

    /**Step 7: Initial input,output for the host and create memory objects for the kernel*/
    const int NUM=512000;
    double* input = new double[NUM];
    for(int i=0;i<NUM;i++)
        input[i]=i;
    double* output = new double[NUM];

    cl_mem inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, (NUM) * sizeof(double),(void *) input, NULL);
    if(inputBuffer == NULL)
    {
        cerr << "inputBuffer is error!" << endl;
    }
    cl_mem outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY , NUM * sizeof(double), NULL, NULL);
    if(outputBuffer == NULL)
    {
        cerr << "outputBuffer is error!" << endl;
    }
    cout  << "step " << stepIdx << " passed!" << endl;
    stepIdx ++;

    /**Step 8: Create kernel object */
    cl_kernel kernel = clCreateKernel(program,"helloworld", NULL);
    if(kernel == NULL)
    {
        cerr << "Failed to create kernel" << endl;
        return -1;
    }
    cout  << "step " << stepIdx << " passed!" << endl;
    stepIdx ++;

    /**Step 9: Sets Kernel arguments.*/
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&inputBuffer);
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&outputBuffer);
    if(status != CL_SUCCESS)
    {
        cerr << "Error settint kernel arguments." << endl;
        return -1;
    }
    cout  << "step " << stepIdx << " passed!" << endl;
    stepIdx ++;

    /**Step 10: Running the kernel.*/
    size_t global_work_size[1] = {NUM};
    

    cl_int err = CL_SUCCESS; 
    cl_event enentPoint;
    status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, global_work_size,
         NULL, 0, NULL, &enentPoint);
    status = clWaitForEvents(1,&enentPoint); ///wait

    if(status != CL_SUCCESS) 
    {
        cout << "Error in clWaitForEvents\n" << endl;
    } 
    clReleaseEvent(enentPoint);
    cout  << "step " << stepIdx << " passed!" << endl;
    stepIdx ++;

    /**Step 11: Read the cout put back to host memory.*/
    status = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0, NUM * sizeof(double), output, 0, NULL, NULL);
    cout<<output[NUM-1]<<endl;
    cout  << "step " << stepIdx << " passed!" << endl;
    stepIdx ++;

    /**Step 12: Clean the resources.*/
    status = clReleaseKernel(kernel);//*Release kernel.
    status = clReleaseProgram(program);    //Release the program object.
    status = clReleaseMemObject(inputBuffer);//Release mem object.
    status = clReleaseMemObject(outputBuffer);
    status = clReleaseCommandQueue(commandQueue);//Release  Command queue.
    status = clReleaseContext(context);//Release context.

    cout  << "step " << stepIdx << " passed!" << endl;
    stepIdx ++;
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
    cout << "end"<<endl;
    return 0;
}



