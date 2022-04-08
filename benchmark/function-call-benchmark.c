//
// Created by Gábor Pituk on 14/03/2022.
//

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
#define NO_INNER_ITERATIONS (10000U)

static const char *test_library_absolute_path = TEST_LIBRARIES_ROOT "/libhello_world.so";

static unsigned long pmc_values[NUM_COUNTERS][NO_OUTER_ITERATIONS];


int
main(int argc, char *argv[])
{
	/* Initialise PMC library */
	pmc_init();

	/* Pin benchmark to a single CPU */
	pin_to_cpu();

	/* Open the test library */
	void *handle = DLOPEN(test_library_absolute_path, RTLD_NOW);
	if (NULL == handle) {
		xo_err(EX_OSERR, "FAIL: dlopen (%s)", dlerror());
	}
	/* Get the test function symbol */
	const char *(*hello_world)(void) = (const char *(*)(void)) dlsym(handle, "hello_world");
	if (NULL == hello_world) {
		xo_err(EX_OSERR, "FAIL: dlsym (%s)", dlerror());
	}

	/* Run function call benchmark in a loop */
	for (unsigned outer_it_no = 0; outer_it_no < NO_OUTER_ITERATIONS; ++outer_it_no) {

		/* Attach and start counters */
		pmc_setup_run();
		pmc_begin();

		/* Call into the library */
		for (unsigned inner_it_no = 0; inner_it_no < NO_INNER_ITERATIONS; ++inner_it_no) {
			(void) hello_world();
		}

		/* Read counter values */
		for (unsigned counter_index = 0; counter_index < NUM_COUNTERS; ++counter_index) {
			if (pmc_read(pmcids[counter_index], &pmc_values[counter_index][outer_it_no]) < 0) {
				xo_err(EX_OSERR, "FAIL: pmc_read (%s) for %s", strerror(errno), counter_set[counter_index]);
			}
		}

		/* Detach counters */
		pmc_end();
		pmc_teardown_run();
	}

	/* End of benchmark */
	/* Print results */
	xo_parse_args(argc, argv);
#ifdef BENCHMARK_DLOPEN
	xo_open_container("function-call");
#else
	xo_open_container("function-call-sandbox");
#endif

	xo_emit("{Lwc:Outer iterations}{:outer-iterations/%u}", NO_OUTER_ITERATIONS);
	xo_emit("{Lwc:Inner iterations}{:inner-iterations/%u}", NO_INNER_ITERATIONS);

	xo_open_list("libraries");
	xo_open_instance("libraries");
	xo_emit("{Lwc:Library path}{:library-path/%s}\n", test_library_absolute_path);
	for (unsigned counter_index = 0; counter_index < NUM_COUNTERS; ++counter_index) {
		xo_open_list(counter_set[counter_index]);
		for (unsigned i = 0; i < NO_OUTER_ITERATIONS; ++i) {
			const char *const fmt = "{Lwc:%s}{l:%s/%cU}\n";
			char fmt_buf[50];
			sprintf(fmt_buf, fmt, counter_set[counter_index], counter_set[counter_index], '%');
			xo_emit(fmt_buf, pmc_values[counter_index][i]);
		}
		xo_close_list(counter_set[counter_index]);
	}
	xo_close_instance("libraries");
	xo_close_list("libraries");

#ifdef BENCHMARK_DLOPEN
	xo_close_container("function-call");
#else
	xo_close_container("function-call-sandbox");
#endif

	/* Finish writing out data */
	xo_finish();

	return 0;
}
