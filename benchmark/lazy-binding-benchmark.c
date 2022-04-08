#include <dlfcn.h>
#include <libxo/xo.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>

#include "benchmark-utils.h"

#if defined(BENCHMARK_DLOPEN)
#define DLOPEN dlopen
#elif defined(BENCHMARK_DLOPEN_SANDBOX)
#define DLOPEN dlopen_sandbox
#else
#define DLOPEN fail, one of the above symbols need to be defined
#endif

#define NO_OUTER_ITERATIONS (1000U)
#define NO_INNER_ITERATIONS (10U)

#define NUM_TEST_LIBRARIES (1U)
static const char *test_library_absolute_paths[] = {
		TEST_LIBRARIES_ROOT "/libhello_world_with_deps_10.so",
		NULL
};

static unsigned long pmc_values[NUM_TEST_LIBRARIES][NUM_COUNTERS][NO_OUTER_ITERATIONS];

static char namebufs[NO_INNER_ITERATIONS][32];

int
main(int argc, char *argv[])
{
	/* Initialise PMC library */
	pmc_init();

	/* Pin benchmark to a single CPU */
	pin_to_cpu();

	/* Initialise names */
	for (unsigned inner_it_no = 0; inner_it_no < NO_INNER_ITERATIONS; ++inner_it_no) {
		sprintf(namebufs[inner_it_no], "hello_world_%d", inner_it_no+1);
	}

	/* Run lazy binding benchmark in a loop on the set of test libraries */
	for (unsigned testlib_no = 0; testlib_no < NUM_TEST_LIBRARIES; ++testlib_no) {

		for (unsigned outer_it_no = 0; outer_it_no < NO_OUTER_ITERATIONS; ++outer_it_no) {

			/* Assert object is closed */
			if (NULL != DLOPEN(test_library_absolute_paths[testlib_no], RTLD_NOLOAD)) {
				fprintf(stderr, "Assert failed, object not closed between iterations\n");
			}

			/* Open object with lazy binding enabled */
			void *handle = DLOPEN(test_library_absolute_paths[testlib_no], RTLD_LAZY);
			if (NULL == handle) {
				xo_err(EX_OSERR, "FAIL: dlopen (%s)", dlerror());
			}

			/* Attach and start counters */
			pmc_setup_run();
			pmc_begin();

			/* Force the bindings by calling the symbols */
			for (unsigned inner_it_no = 0; inner_it_no < NO_INNER_ITERATIONS; ++inner_it_no) {
				const char *(*hello_world)(void) = (const char *(*)(void)) dlsym(handle, namebufs[inner_it_no]);
				(void) hello_world();
			}

			/* Read off counter values */
			for (unsigned counter_index = 0; counter_index < NUM_COUNTERS; ++counter_index) {
				if (pmc_read(pmcids[counter_index], &pmc_values[testlib_no][counter_index][outer_it_no]) < 0) {
					xo_err(EX_OSERR, "FAIL: pmc_read (%s) for %s", strerror(errno), counter_set[counter_index]);
				}
			}

			/* Detach counters */
			pmc_end();
			pmc_teardown_run();

			/* Close the object before the next iteration */
			if (dlclose(handle) < 0) {
				xo_err(EX_OSERR, "FAIL: dlclose (%s)", dlerror());
			}
		}
	}

	/* End of benchmark */
	/* Print results */
	xo_parse_args(argc, argv);
#ifdef BENCHMARK_DLOPEN
	xo_open_container("lazy-binding");
#else
	xo_open_container("lazy-binding-sandbox");
#endif

	xo_emit("{Lwc:Outer iterations}{:outer-iterations/%u}", NO_OUTER_ITERATIONS);
	xo_emit("{Lwc:Inner iterations}{:inner-iterations/%u}", NO_INNER_ITERATIONS);

	xo_open_list("libraries");
	for (unsigned libno = 0; libno < NUM_TEST_LIBRARIES; ++libno) {
		xo_open_instance("libraries");
		xo_emit("{Lwc:Library path}{:library-path/%s}\n", test_library_absolute_paths[libno]);
		for (unsigned counter_index = 0; counter_index < NUM_COUNTERS; ++counter_index) {
			xo_open_list(counter_set[counter_index]);
			for (unsigned i = 0; i < NO_OUTER_ITERATIONS; ++i) {
				const char *const fmt = "{Lwc:%s}{l:%s/%cU}\n";
				char fmt_buf[50];
				sprintf(fmt_buf, fmt, counter_set[counter_index], counter_set[counter_index], '%');
				xo_emit(fmt_buf, pmc_values[libno][counter_index][i]);
			}
			xo_close_list(counter_set[counter_index]);
		}
		xo_close_instance("libraries");
	}
	xo_close_list("libraries");

#ifdef BENCHMARK_DLOPEN
	xo_close_container("lazy-binding");
#else
	xo_close_container("lazy-binding-sandbox");
#endif

	/* Finish writing out data */
	xo_finish();

	return 0;
}
