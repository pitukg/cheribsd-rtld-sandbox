//
// Created by Gábor Pituk on 14/03/2022.
//

#ifndef CHERIBSD_BENCHMARK_UTILS_H
#define CHERIBSD_BENCHMARK_UTILS_H

#include <libxo/xo.h>
#include <pmc.h>

#define NUM_COUNTERS (3U)
static const char *counter_set[] = {
		"CPU_CYCLES",
		"BR_MIS_PRED",
		"INST_RETIRED"
};

static pmc_id_t pmcids[NUM_COUNTERS];

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
