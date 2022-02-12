add_compile_definitions(BENCHMARK_DLOPEN=1)
include_directories(../lib/libpmc)
add_executable(dlopen-benchmark dlopen-benchmark.c)