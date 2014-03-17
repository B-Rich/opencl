#include <CL/cl.h>

#include "info.h"
#include "util.h"

#include <stdio.h>

#define USE_MAP_BUFFER

static cl_context       ctx;
static cl_platform_id   platform;
static cl_device_id     device;
static cl_mem           in_buf, out_buf;
static cl_program       prg;
static cl_kernel        kernel;
static cl_command_queue cmd_queue;

float const test_data[] = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 };

static void CL_CALLBACK err_callback(
  char const *errinfo,
  void const *private_info,
  size_t cb,
  void *user_data) {
    fprintf(stderr, "OpenCL error callback: %s\n", errinfo);
}

static void CL_CALLBACK destructor_callback(cl_mem memobj, void *user_data) {
    printf("Object destroyed: %s\n", (char*)user_data);
}

static void create_ctx() {
    cl_uint n_platforms, n_devices;

    // Use the first GPU device of the first platform

    C(clGetPlatformIDs, 1, &platform, &n_platforms);
    if (n_platforms == 0)
        fail("n_platforms == 0");

    C(clGetDeviceIDs, platform, CL_DEVICE_TYPE_GPU, 1, &device, &n_devices);

    cl_context_properties const ctx_props[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
        0, 0 };
    ctx = CE(clCreateContext, ctx_props, 1, &device, err_callback, NULL, &err);
}

static void create_buffers() {
    in_buf = CE(clCreateBuffer,
      ctx,
      CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
      sizeof test_data, (float*)test_data,
      &err);

    out_buf = CE(clCreateBuffer,
      ctx,
      CL_MEM_WRITE_ONLY,
      sizeof test_data, NULL,
      &err);

    print_buf_info("in_buf", in_buf);
    print_buf_info("out_buf", out_buf);

    C(clSetMemObjectDestructorCallback,
      in_buf,
      destructor_callback, (void*)"in_buf");

    C(clSetMemObjectDestructorCallback,
      out_buf,
      destructor_callback, (void*)"out_buf");
}

static void create_cmd_queue() {
    cmd_queue = CE(clCreateCommandQueue,
      ctx, device,
      0,
      &err);
}

static void compile_program(char const *filename) {
    static char build_log[2048];
    size_t      filesize;
    char const *source = get_file_buffer(filename, filesize);
    cl_int      res;

    prg = CE(clCreateProgramWithSource, ctx, 1, &source, &filesize, &err);

    res = clBuildProgram(
      prg,
      1, &device,  // num_devices, device_list
      "-Werror",   // options
      NULL, NULL); // pfn_notify, user_data

    C(clGetProgramBuildInfo,
      prg,
      device,
      CL_PROGRAM_BUILD_LOG,
      ARRAY_LEN(build_log),
      build_log,
      NULL);

    printf("Build log:\n%s\n", build_log);

    if (res != CL_SUCCESS) {
        if (res == CL_BUILD_PROGRAM_FAILURE)
            fail("Compilation error");
        else
            fail("Compilation failed: %s\n", cl_err2str(res));
    }

    kernel = CE(clCreateKernel, prg, "test_kernel", &err);

    print_kernel_info(kernel, device);

    delete [] source;
}

static void bind_arguments() {
    C(clSetKernelArg, kernel, 0, sizeof in_buf, &in_buf);
    C(clSetKernelArg, kernel, 1, sizeof out_buf, &out_buf);
}

static void clean_up() {
    C(clReleaseContext, ctx);
    C(clReleaseMemObject, in_buf);
    C(clReleaseMemObject, out_buf);
    C(clReleaseProgram, prg);
    C(clReleaseKernel, kernel);
    C(clReleaseCommandQueue, cmd_queue);
}

static void run() {
    size_t const work_size = ARRAY_LEN(test_data);
    float res[ARRAY_LEN(test_data)];

    C(clEnqueueNDRangeKernel,
      cmd_queue,
      kernel,
      1,                // work_dim
      NULL, &work_size, // global_work_offset, global_work_size
      NULL,             // local_work_size - size of work-group
      0,                // num_events_in_wait_list
      NULL,             // event_wait_list
      NULL);            // event

    float *res_ptr;

#ifdef USE_MAP_BUFFER

    res_ptr = (float*)CE(clEnqueueMapBuffer,
      cmd_queue,
      out_buf,
      CL_TRUE,
      CL_MAP_READ,
      0, sizeof test_data,
      0, NULL,
      NULL,
      &err);

#else

    C(clEnqueueReadBuffer,
      cmd_queue,
      out_buf,
      CL_TRUE,             // blocking_read
      0, sizeof test_data, // offset, cb
      res,                 // ptr
      0, NULL,             // num_events_in_wait_list, event_wait_list
      NULL);               // event

    res_ptr = res;

#endif // USE_MAP_BUFFER

    for (size_t i = 0; i < ARRAY_LEN(res); ++i)
        printf("%f ", res_ptr[i]);
    putchar('\n');

#ifdef USE_MAP_BUFFER

    C(clEnqueueUnmapMemObject,
      cmd_queue,
      out_buf,
      res_ptr,
      0, NULL,
      NULL);

#endif
}

int main(int argc, char *argv[]) {
    char const *filename = argc > 1 ? argv[1] : "test.cl";

    print_platform_and_dev_info();

    create_ctx();
    create_buffers();
    create_cmd_queue();

    compile_program(filename);
    bind_arguments();
    run();

    clean_up();

    puts("Shut down cleanly");
}
