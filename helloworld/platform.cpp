#include "platform.h"
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




