//
// Created by Gábor Pituk on 14/03/2022.
//

#ifndef CHERIBSD_BENCHMARK_UTILS_H
#define CHERIBSD_BENCHMARK_UTILS_H

#include <libxo/xo.h>
#include <pmc.h>
#include <sys/cpuset.h>

#define NUM_COUNTERS (4U)
static const char *counter_set[] = {
		"CPU_CYCLES",
		"BR_MIS_PRED",
		"INST_RETIRED",
		"L1D_CACHE_REFILL"
};

static pmc_id_t pmcids[NUM_COUNTERS];

static void
pin_to_cpu(void)
{
	cpuset_t cpuset_mask;
	/* Pin the benchmark to CPU 0 */
	CPU_ZERO(&cpuset_mask);
	CPU_SET(0, &cpuset_mask);
	if (cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_PID, -1, sizeof(cpuset_mask), &cpuset_mask) < 0) {
		xo_err(EX_OSERR, "FAIL: cpuset_setaffinity (%s)", strerror(errno));
	}
}

static void
pmc_setup_run(void)
{
	for (int counter_index = 0; counter_index < NUM_COUNTERS; ++counter_index) {
		if (pmc_allocate(counter_set[counter_index], PMC_MODE_TC, 0, PMC_CPU_ANY, &pmcids[counter_index], 64 * 1024) < 0) {
			xo_err(EX_OSERR, "FAIL: pmc_allocate (%s) for %s", strerror(errno), counter_set[counter_index]);
		}
		if (pmc_attach(pmcids[counter_index], 0) < 0) {
			xo_err(EX_OSERR, "FAIL: pmc_attach (%s) for %s", strerror(errno), counter_set[counter_index]);
		}
		if (pmc_write(pmcids[counter_index], 0) < 0) {
			xo_err(EX_OSERR, "FAIL: pmc_write (%s) for %s", strerror(errno), counter_set[counter_index]);
		}
	}
}

static void
pmc_teardown_run(void)
{
	for (int counter_index = 0; counter_index < NUM_COUNTERS; ++counter_index) {
		if (pmc_detach(pmcids[counter_index], 0) < 0) {
			xo_err(EX_OSERR, "FAIL: pmc_detach (%s) for %s", strerror(errno), counter_set[counter_index]);
		}
		if (pmc_release(pmcids[counter_index]) < 0) {
			xo_err(EX_OSERR, "FAIL: pmc_release (%s) for %s", strerror(errno), counter_set[counter_index]);
		}
	}
}

static __inline void
pmc_begin(void)
{
	for (int counter_index = 0; counter_index < NUM_COUNTERS; ++counter_index) {
		if (pmc_start(pmcids[counter_index]) < 0) {
			xo_err(EX_OSERR, "FAIL: pmc_start (%s) for %s", strerror(errno), counter_set[counter_index]);
		}
	}
}

static __inline void
pmc_end(void)
{
	for (int counter_index = 0; counter_index < NUM_COUNTERS; ++counter_index) {
		if (pmc_stop(pmcids[counter_index]) < 0) {
			xo_err(EX_OSERR, "FAIL: pmc_stop (%s) for %s", strerror(errno), counter_set[counter_index]);
		}
	}
}

#endif //CHERIBSD_BENCHMARK_UTILS_H
