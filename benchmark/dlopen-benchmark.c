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

#ifndef DLOPEN_MODE
#define DLOPEN_MODE RTLD_NOW
#endif

#define NO_OUTER_ITERATIONS (1000U)
#define NO_INNER_ITERATIONS (100U)

#define NUM_TEST_LIBRARIES (3U)
static const char *test_library_absolute_paths[] = {
	TEST_LIBRARIES_ROOT "/libhello_world.so",
	TEST_LIBRARIES_ROOT "/libz_nofio.so",
	TEST_LIBRARIES_ROOT "/libpng.so",
	NULL
};

static unsigned long pmc_values[NUM_TEST_LIBRARIES][NUM_COUNTERS][NO_OUTER_ITERATIONS];


int
main(int argc, char *argv[])
{
	/* Initialise PMC library */
	pmc_init();

	/* Pin benchmark to a single CPU */
	pin_to_cpu();

	/* Run dlopen-dlclose benchmark in a loop on the set of test libraries */
	for (unsigned testlib_no = 0; testlib_no < NUM_TEST_LIBRARIES; ++testlib_no) {

		for (unsigned outer_it_no = 0; outer_it_no < NO_OUTER_ITERATIONS; ++outer_it_no) {

			/* Assert object is closed */
			if (NULL != DLOPEN(test_library_absolute_paths[testlib_no], RTLD_NOLOAD)) {
				fprintf(stderr, "Assert failed, object not closed between iterations\n");
			}

			/* Attach and start counters */
			pmc_setup_run();
			pmc_begin();

			for (unsigned inner_it_no = 0; inner_it_no < NO_INNER_ITERATIONS; ++inner_it_no) {
				void *handle = DLOPEN(test_library_absolute_paths[testlib_no], DLOPEN_MODE);

				if (NULL == handle) {
					xo_err(EX_OSERR, "FAIL: dlopen (%s)", dlerror());
				}

				if (dlclose(handle) < 0) {
					xo_err(EX_OSERR, "FAIL: dlclose (%s)", dlerror());
				}
			}

			for (unsigned counter_index = 0; counter_index < NUM_COUNTERS; ++counter_index) {
				if (pmc_read(pmcids[counter_index], &pmc_values[testlib_no][counter_index][outer_it_no]) < 0) {
					xo_err(EX_OSERR, "FAIL: pmc_read (%s) for %s", strerror(errno), counter_set[counter_index]);
				}
			}

			/* Detach counters */
			pmc_end();
			pmc_teardown_run();
		}
	}

	/* End of benchmark */
	/* Print results */
	xo_parse_args(argc, argv);
#ifdef BENCHMARK_DLOPEN
	xo_open_container("dlopen-dlclose");
#else
	xo_open_container("dlopen-dlclose-sandbox");
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
	xo_close_container("dlopen-dlclose");
#else
	xo_close_container("dlopen-dlclose-sandbox");
#endif

	/* Finish writing out data */
	xo_finish();

	return 0;
}
