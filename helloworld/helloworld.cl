__kernel void helloworld(__global double* in, __global double* out)
{
    int num = get_global_id(0);
	out[num] = in[num] / 2.4 *(in[num]/6) ;
}

__kernel void hello_kernel(__global const float *a,
                            __global const float *b,
                            __global const float *result)
{
    int gid = get_global_id(0);
    result[gid] = a[gid] + b[gid];
}