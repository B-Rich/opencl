#include "cl_util.h"

kernel void test_kernel(global float *in, global float *out) {
    size_t const i = get_global_id(0);
    out[i] = 2*in[i];
}
