#include <new>

char const *cl_err2str(cl_int e);

void fail(char const *format, ...);
void errno_fail(int errno_val, char const *format, ...);

#define C(fn, ...)                                        \
  do {                                                    \
      cl_int err = (fn)(__VA_ARGS__);                     \
      if (err != CL_SUCCESS)                              \
          fail("%s:%ld: %s failed: %s\n",                 \
               __FILE__, __LINE__, #fn, cl_err2str(err)); \
  } while(0)

#define CE(fn, ...)                                       \
  ({ cl_int err;                                          \
     typeof ((fn)(__VA_ARGS__)) res = (fn)(__VA_ARGS__);  \
     if (err != CL_SUCCESS)                               \
          fail("%s:%ld: %s failed: %s\n",                 \
               __FILE__, __LINE__, #fn, cl_err2str(err)); \
     res; })

char *get_file_buffer(char const *filename, size_t &size_out);

template<typename T, size_t N>
char (&array_len_helper(T (&)[N]))[N];

#define ARRAY_LEN(a) (sizeof array_len_helper(a))

template<typename T>
inline T *alloc(size_t len) {
    T *res = new (std::nothrow) T[len];
    if (!res)
        fail("Memory allocation failed");
    return res;
}
