#nvcc main.cpp -l OpenCL
gcc -o main src/main.c -L/usr/local/cuda/ -l OpenCL -I/usr/local/cuda/include/
