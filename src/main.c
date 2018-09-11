#include <stdio.h>
#include <stdlib.h>
 
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
int main()
{
    cl_int status=0;
    size_t deviceListSize;
    cl_uint numPlatforms;
    cl_platform_id platfomr=NULL;
    status=clGetPlatformIDs(0,NULL,&numPlatforms);
    printf("status=%d\n",status);
    return 0;  
}





