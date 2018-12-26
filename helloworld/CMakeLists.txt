cmake_minimum_required (VERSION 3.5)

set(EXE_NAME test_ocl)

file(GLOB native_srcs "./*.cpp") #当前目录下的所有cpp文件链接均写入native_srcs变量中

include_directories(/usr/local/cuda-8.0/include/ .)
link_directories(/usr/local/cuda-8.0/lib64)
link_libraries(OpenCL)

ADD_DEFINITIONS(
    -O3 -g -W -Wall
)

add_executable(${EXE_NAME} ${native_srcs})
# target_link_libraries(${EXE_NAME} 
#     -lOpenCL
# )




