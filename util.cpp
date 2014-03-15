#include <CL/cl.h>

#include "util.h"

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

char const *cl_err2str(cl_int e) {
    #define E(c) case c: return #c;
    switch (e) {
    E(CL_SUCCESS)
    E(CL_DEVICE_NOT_FOUND)
    E(CL_DEVICE_NOT_AVAILABLE)
    E(CL_COMPILER_NOT_AVAILABLE)
    E(CL_MEM_OBJECT_ALLOCATION_FAILURE)
    E(CL_OUT_OF_RESOURCES)
    E(CL_OUT_OF_HOST_MEMORY)
    E(CL_PROFILING_INFO_NOT_AVAILABLE)
    E(CL_MEM_COPY_OVERLAP)
    E(CL_IMAGE_FORMAT_MISMATCH)
    E(CL_IMAGE_FORMAT_NOT_SUPPORTED)
    E(CL_BUILD_PROGRAM_FAILURE)
    E(CL_MAP_FAILURE)
    E(CL_MISALIGNED_SUB_BUFFER_OFFSET)
    E(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST)
    E(CL_COMPILE_PROGRAM_FAILURE)
    E(CL_LINKER_NOT_AVAILABLE)
    E(CL_LINK_PROGRAM_FAILURE)
    E(CL_DEVICE_PARTITION_FAILED)
    E(CL_KERNEL_ARG_INFO_NOT_AVAILABLE)

    E(CL_INVALID_VALUE)
    E(CL_INVALID_DEVICE_TYPE)
    E(CL_INVALID_PLATFORM)
    E(CL_INVALID_DEVICE)
    E(CL_INVALID_CONTEXT)
    E(CL_INVALID_QUEUE_PROPERTIES)
    E(CL_INVALID_COMMAND_QUEUE)
    E(CL_INVALID_HOST_PTR)
    E(CL_INVALID_MEM_OBJECT)
    E(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)
    E(CL_INVALID_IMAGE_SIZE)
    E(CL_INVALID_SAMPLER)
    E(CL_INVALID_BINARY)
    E(CL_INVALID_BUILD_OPTIONS)
    E(CL_INVALID_PROGRAM)
    E(CL_INVALID_PROGRAM_EXECUTABLE)
    E(CL_INVALID_KERNEL_NAME)
    E(CL_INVALID_KERNEL_DEFINITION)
    E(CL_INVALID_KERNEL)
    E(CL_INVALID_ARG_INDEX)
    E(CL_INVALID_ARG_VALUE)
    E(CL_INVALID_ARG_SIZE)
    E(CL_INVALID_KERNEL_ARGS)
    E(CL_INVALID_WORK_DIMENSION)
    E(CL_INVALID_WORK_GROUP_SIZE)
    E(CL_INVALID_WORK_ITEM_SIZE)
    E(CL_INVALID_GLOBAL_OFFSET)
    E(CL_INVALID_EVENT_WAIT_LIST)
    E(CL_INVALID_EVENT)
    E(CL_INVALID_OPERATION)
    E(CL_INVALID_GL_OBJECT)
    E(CL_INVALID_BUFFER_SIZE)
    E(CL_INVALID_MIP_LEVEL)
    E(CL_INVALID_GLOBAL_WORK_SIZE)
    E(CL_INVALID_PROPERTY)
    E(CL_INVALID_IMAGE_DESCRIPTOR)
    E(CL_INVALID_COMPILER_OPTIONS)
    E(CL_INVALID_LINKER_OPTIONS)
    E(CL_INVALID_DEVICE_PARTITION_COUNT)

    default: return "Unknown error";
    }
    #undef E
}

static void fail_helper(bool include_errno, char const *format, va_list args)
  __attribute__((format(printf, 2, 0), noreturn));

static void fail_helper(bool include_errno, char const *format, va_list args) {
    vfprintf(stderr, format, args);
    if (include_errno) {
        char const*const err_str = strerror(errno);
        fprintf(stderr, ": %s (errno = %d)", err_str ? err_str : "unknown error", errno);
    }
    putc('\n', stderr);
    exit(EXIT_FAILURE);
}

void fail(char const *format, ...) {
    va_list args;
    va_start(args, format);
    fail_helper(false, format, args);
}

void errno_fail(char const *format, ...) {
    va_list args;
    va_start(args, format);
    fail_helper(true, format, args);
}

char *get_file_buffer(char const *filename, size_t &size_out) {
    FILE *file;
    char *file_buf;
    long file_size;

    if (!(file = fopen(filename, "rb")))
        errno_fail("failed to open '%s'", filename);
    if (fseek(file, 0, SEEK_END) == -1)
        errno_fail("failed to seek to end of '%s'", filename);
    if ((file_size = ftell(file)) == -1)
        errno_fail("failed to get size of '%s'", filename);
    if (fseek(file, 0, SEEK_SET) == -1)
        errno_fail("failed to seek to beginning of '%s'", filename);

    file_buf = alloc<char>(file_size);
    size_t const fread_res = fread(file_buf, 1, file_size, file);
    if ((unsigned long long)fread_res < (unsigned long long)file_size)
        errno_fail("Unknown error opening '%s'", filename);
    if (fclose(file) == EOF)
        errno_fail("failed to close '%s'", filename);

    size_out = file_size;
    return file_buf;
}
