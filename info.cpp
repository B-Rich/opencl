#include <CL/cl.h>

#include "info.h"
#include "util.h"

#include <stdio.h>

void print_buf_info(char const *name, cl_mem buf) {
    size_t actual_size;

    C(clGetMemObjectInfo,
      buf,
      CL_MEM_SIZE,
      sizeof actual_size, &actual_size, NULL);

    printf("Actual size of %s: %zu\n", name, actual_size);
}

void print_kernel_info(cl_kernel kernel, cl_device_id device) {
    size_t   size_t_buf;
    cl_ulong cl_ulong_buf;

    #define PINFO_S(info)                            \
      C(clGetKernelWorkGroupInfo,                    \
        kernel,                                      \
        device,                                      \
        info, sizeof size_t_buf, &size_t_buf, NULL); \
      printf("%s: %zu\n", #info, size_t_buf);

    #define PINFO_U(info)                                      \
      C(clGetKernelWorkGroupInfo,                              \
        kernel,                                                \
        device,                                                \
        info, sizeof cl_ulong_buf, &cl_ulong_buf, NULL);       \
      printf("%s: %lu\n", #info, (unsigned long)cl_ulong_buf);

    PINFO_S(CL_KERNEL_WORK_GROUP_SIZE);
    // The work-group size should preferably be a multiple of this. Corresponds
    // to the wavefront size (number of threads in a scheduling unit) on NVIDIA
    // GPUs.
    PINFO_S(CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE);

    PINFO_U(CL_KERNEL_LOCAL_MEM_SIZE);
    PINFO_U(CL_KERNEL_PRIVATE_MEM_SIZE);

    #undef PINFO_S
}

void print_platform_and_dev_info() {
    cl_uint         n_platforms, n_devices;
    cl_platform_id *platforms;
    cl_device_id   *devices;
    char            strbuf[1024];
    cl_uint         uintbuf;

    C(clGetPlatformIDs, 0, NULL, &n_platforms);
    platforms = alloc<cl_platform_id>(n_platforms);
    C(clGetPlatformIDs, n_platforms, platforms, NULL);
    for (cl_uint p = 0; p < n_platforms; ++p) {
        printf("========== Platform %u ==========\n", (unsigned)p);

        #define PPINFO(info)                    \
          C(clGetPlatformInfo,                  \
            platforms[p],                       \
            info, sizeof strbuf, strbuf, NULL); \
          printf("%s: %s\n", #info, strbuf)

        PPINFO(CL_PLATFORM_NAME);
        PPINFO(CL_PLATFORM_VENDOR);
        PPINFO(CL_PLATFORM_VERSION);
        PPINFO(CL_PLATFORM_PROFILE);
        PPINFO(CL_PLATFORM_EXTENSIONS);

        #undef PPINFO

        C(clGetDeviceIDs, platforms[p], CL_DEVICE_TYPE_ALL, 0, NULL, &n_devices);
        devices = alloc<cl_device_id>(n_devices);
        C(clGetDeviceIDs, platforms[p], CL_DEVICE_TYPE_ALL, n_devices, devices, NULL);
        for (cl_uint d = 0; d < n_devices; ++d) {
            printf("===== Device %u of platform %u =====\n", (unsigned)d, (unsigned)p);

            #define PDINFO_S(info)                  \
              C(clGetDeviceInfo,                    \
                devices[d],                         \
                info, sizeof strbuf, strbuf, NULL); \
              printf("%s: %s\n", #info, strbuf);

            #define PDINFO_U(info)                     \
              C(clGetDeviceInfo,                       \
                devices[d],                            \
                info, sizeof uintbuf, &uintbuf, NULL); \
              printf("%s: %lu\n", #info,               \
                     (unsigned long)uintbuf);

            PDINFO_S(CL_DEVICE_NAME);
            PDINFO_S(CL_DEVICE_VENDOR);
            PDINFO_S(CL_DRIVER_VERSION);
            PDINFO_S(CL_DEVICE_PROFILE);
            PDINFO_S(CL_DEVICE_VERSION);
            PDINFO_S(CL_DEVICE_OPENCL_C_VERSION);
            PDINFO_S(CL_DEVICE_EXTENSIONS);

            PDINFO_U(CL_DEVICE_MAX_COMPUTE_UNITS);
            PDINFO_U(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
            PDINFO_U(CL_DEVICE_MAX_CLOCK_FREQUENCY);

            #undef PDINFO_S
            #undef PDINFO_U
        }
        delete [] devices;
    }
    delete [] platforms;
}
