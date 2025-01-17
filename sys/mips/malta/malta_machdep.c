/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2006 Wojciech A. Koszek <wkoszek@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_ddb.h"

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/imgact.h>
#include <sys/bio.h>
#include <sys/boot.h>
#include <sys/buf.h>
#include <sys/bus.h>
#include <sys/cpu.h>
#include <sys/cons.h>
#include <sys/exec.h>
#include <sys/ucontext.h>
#include <sys/proc.h>
#include <sys/kdb.h>
#include <sys/ptrace.h>
#include <sys/reboot.h>
#include <sys/signalvar.h>
#include <sys/sysent.h>
#include <sys/sysproto.h>
#include <sys/user.h>
#include <sys/eventhandler.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_phys.h>
#include <vm/vm_dumpset.h>

#include <machine/clock.h>
#include <machine/cpu.h>
#include <machine/cpuregs.h>
#include <machine/hwfunc.h>
#include <machine/md_var.h>
#include <machine/pmap.h>
#include <machine/trap.h>

#ifdef TICK_USE_YAMON_FREQ
#include <mips/malta/yamon.h>
#endif

#ifdef TICK_USE_MALTA_RTC
#include <mips/malta/maltareg.h>
#include <isa/rtc.h>
#endif

#include <mips/malta/maltareg.h>

#ifdef __CHERI_PURE_CAPABILITY__
#include <cheri/cheric.h>
#endif

extern int	*edata;
extern int	*end;

void	lcd_init(void);
void	lcd_puts(char *);
void	malta_reset(void);

/*
 * Temporary boot environment used at startup.
 */
static char boot1_env[4096];

/*
 * Offsets to MALTA LCD characters.
 */
static int malta_lcd_offs[] = {
	MALTA_ASCIIPOS0,
	MALTA_ASCIIPOS1,
	MALTA_ASCIIPOS2,
	MALTA_ASCIIPOS3,
	MALTA_ASCIIPOS4,
	MALTA_ASCIIPOS5,
	MALTA_ASCIIPOS6,
	MALTA_ASCIIPOS7
};

/*
 * When emulating MALTA under Qemu, ISA-level tracing can be enabled and
 * disabled using special NOP instructions.  By default, this is simple a
 * global setting that the kernel doesn't interfere with, allowing usersapce
 * to turn on and off tracing as it sees fit.  If this sysctl is set, then the
 * kernel will instead use a per-thread flag (td->td_mdflags & MDTD_QTRACE) to
 * turn tracing on and off during context switching to reflect the thread's
 * tracing state.  That state can be get/set/cleared using sysarch system
 * calls.
 *
 * NB: This is a Qemu-CHERI feature.
 */
#ifdef CPU_QEMU_MALTA
u_int	qemu_trace_perthread;
SYSCTL_UINT(_hw, OID_AUTO, qemu_trace_perthread, CTLFLAG_RW,
    &qemu_trace_perthread, 0, "Per-thread Qemu ISA-level tracing configured");

/*
 * QEMU ISA-level tracing in buffered mode.
 * The trace entries are stored in a ring buffer and only emitted
 * upon request. This is currently done on trap.
 */
u_int	qemu_trace_buffered;

static int
sysctl_hw_qemu_buffered(SYSCTL_HANDLER_ARGS)
{
	int error;

	error = sysctl_handle_int(oidp, &qemu_trace_buffered, 0, req);
	if (error || !req->newptr)
		return (error);
	if (qemu_trace_buffered)
		QEMU_SET_TRACE_BUFFERED_MODE;
	else
		QEMU_CLEAR_TRACE_BUFFERED_MODE;
	return (0);
}

SYSCTL_PROC(_hw, OID_AUTO, qemu_trace_buffered, CTLTYPE_INT | CTLFLAG_RW,
    0, 0, sysctl_hw_qemu_buffered, "", "Qemu tracing runs in buffered mode");
#endif

void
platform_cpu_init()
{
	/* Nothing special */
}

/*
 * Put character to Malta LCD at given position.
 */
static void
malta_lcd_putc(int pos, char c)
{
	void *addr;
	char *ch;

	if (pos < 0 || pos > 7)
		return;
	addr = (void *)(MALTA_ASCII_BASE + malta_lcd_offs[pos]);
	ch = (char *)MIPS_PHYS_TO_KSEG0(addr);
	*ch = c;
}

/*
 * Print given string on LCD.
 */
static void
malta_lcd_print(char *str)
{
	int i;

	if (str == NULL)
		return;

	for (i = 0; *str != '\0'; i++, str++)
		malta_lcd_putc(i, *str);
}

void
lcd_init(void)
{
	malta_lcd_print("FreeBSD_");
}

void
lcd_puts(char *s)
{
	malta_lcd_print(s);
}

#ifdef TICK_USE_MALTA_RTC
static __inline uint8_t
malta_rtcin(uint8_t addr)
{

	*((volatile uint8_t *)
	    MIPS_PHYS_TO_KSEG1(MALTA_PCI0_ADDR(MALTA_RTCADR))) = addr;
	return (*((volatile uint8_t *)
	    MIPS_PHYS_TO_KSEG1(MALTA_PCI0_ADDR(MALTA_RTCDAT))));
}

static __inline void
malta_writertc(uint8_t addr, uint8_t val)
{

	*((volatile uint8_t *)
	    MIPS_PHYS_TO_KSEG1(MALTA_PCI0_ADDR(MALTA_RTCADR))) = addr;
	*((volatile uint8_t *)
	    MIPS_PHYS_TO_KSEG1(MALTA_PCI0_ADDR(MALTA_RTCDAT))) = val;
}
#endif

static void
mips_init(unsigned long memsize, uint64_t ememsize)
{
	int i;

	for (i = 0; i < PHYS_AVAIL_ENTRIES; i++) {
		phys_avail[i] = 0;
	}

	/*
	 * memsize is the amount of RAM available below 256MB.
	 * ememsize is the total amount of RAM available.
	 *
	 * The second bank starts at 0x90000000.
	 */

	/* phys_avail regions are in bytes */
	phys_avail[0] = MIPS_KSEG0_TO_PHYS(kernel_kseg0_end);
	phys_avail[1] = memsize;
	dump_avail[0] = 0;
	dump_avail[1] = phys_avail[1];

	/* Only specify the extended region if it's set */
	if (ememsize > memsize) {
		phys_avail[2] = 0x90000000;
		phys_avail[3] = 0x90000000 + (ememsize - memsize);
		dump_avail[2] = phys_avail[2];
		dump_avail[3] = phys_avail[3];
	}

	/* XXX realmem assigned in the caller of mips_init() */
	physmem = realmem;

	init_param1();
	init_param2(physmem);
	mips_cpu_init();
	pmap_bootstrap();
	mips_proc0_init();
	mutex_init();
	kdb_init();
#ifdef KDB
	if (boothowto & RB_KDB)
		kdb_enter(KDB_WHY_BOOTFLAGS, "Boot flags requested debugger");
#endif
}

/*
 * Perform a board-level soft-reset.
 * Note that this is not emulated by gxemul.
 */
void
platform_reset(void)
{
	char *c;

	c = (char *)MIPS_PHYS_TO_KSEG0(MALTA_SOFTRES);
	*c = MALTA_GORESET;
}

#ifdef CPU_QEMU_MALTA
static void
malta_poweroff(void *arg, int howto)
{
	char *c;

	if ((howto & RB_POWEROFF) != 0) {
		c = (char *)MIPS_PHYS_TO_KSEG0(MALTA_SOFTRES);
		*c = MALTA_GORESET + 2;
	}
}

EVENTHANDLER_DEFINE(shutdown_final, malta_poweroff, NULL, SHUTDOWN_PRI_LAST);
#endif

static uint64_t
malta_cpu_freq(void)
{
	uint64_t platform_counter_freq = 0;

#if defined(TICK_USE_YAMON_FREQ)
	/*
	 * If we are running on a board which uses YAMON firmware,
	 * then query CPU pipeline clock from the syscon object.
	 * If unsuccessful, use hard-coded default.
	 */
	platform_counter_freq = yamon_getcpufreq();

#elif defined(TICK_USE_MALTA_RTC)
	/*
	 * If we are running on a board with the MC146818 RTC,
	 * use it to determine CPU pipeline clock frequency.
	 */
	u_int64_t counterval[2];

	/* Set RTC to binary mode. */
	malta_writertc(RTC_STATUSB, (malta_rtcin(RTC_STATUSB) | RTCSB_BCD));

	/* Busy-wait for falling edge of RTC update. */
	while (((malta_rtcin(RTC_STATUSA) & RTCSA_TUP) == 0))
		;
	while (((malta_rtcin(RTC_STATUSA)& RTCSA_TUP) != 0))
		;
	counterval[0] = mips_rd_count();

	/* Busy-wait for falling edge of RTC update. */
	while (((malta_rtcin(RTC_STATUSA) & RTCSA_TUP) == 0))
		;
	while (((malta_rtcin(RTC_STATUSA)& RTCSA_TUP) != 0))
		;
	counterval[1] = mips_rd_count();

	platform_counter_freq = counterval[1] - counterval[0];
#endif

	if (platform_counter_freq == 0)
		platform_counter_freq = MIPS_DEFAULT_HZ;

	return (platform_counter_freq);
}

#ifdef __CHERI_PURE_CAPABILITY__
static void
platform_clear_bss(void *kroot)
{
	/*
	 * We need to hand-craft a capability for edata because it is defined
	 * by the linker script and have no size info.
	 */
	void *edata_start;
	size_t edata_siz = (ptraddr_t)&end - (ptraddr_t)&edata;

	edata_start = cheri_ptrperm(
	    cheri_setaddress(kroot, (ptraddr_t)&edata),
	    edata_siz, CHERI_PERM_STORE);
	memset(edata_start, 0, edata_siz);
}

static char *
platform_argv_ptr(int32_t argv)
{
	char *arg;
	size_t len;

	/*
	 * YAMON uses 32bit pointers to strings so
	 * sign-extend them to the correct type manually.
	 */
	arg = cheri_andperm(
	    cheri_setaddress(mips_kseg0_cap, (vm_offset_t)(argv)),
	    CHERI_PERM_LOAD | CHERI_PERM_STORE);
	arg = cheri_setbounds(arg, strlen(arg) + 1);

	return (arg);
}

#else
static void
platform_clear_bss()
{
	vm_offset_t kernend;

	kernend = (vm_offset_t)&end;
	memset(&edata, 0, kernend - (vm_offset_t)(&edata));
}

static char *
platform_argv_ptr(int32_t argv)
{
	/*
	 * YAMON uses 32bit pointers to strings so
	 * sign-extend them to the correct type manually.
	 */
	return ((char *)(intptr_t)argv);
}
#endif

void
platform_start(__register_t a0, __register_t a1,  __register_t a2,
    __register_t a3)
{
	uint64_t platform_counter_freq;
	int argc = a0;
	int32_t *argv;
	int32_t *envp;
	unsigned int memsize = a3;
	uint64_t ememsize = 0;
	int i;

	/* Clear the BSS and SBSS segments */
#ifdef __CHERI_PURE_CAPABILITY__
	argv = cheri_ptrperm(cheri_setaddress(mips_kseg0_cap, a1),
	    argc * sizeof(*argv), CHERI_PERM_LOAD);
	envp = cheri_andperm(cheri_setaddress(mips_kseg0_cap, a2),
	    CHERI_PERM_LOAD);
	platform_clear_bss(kernel_data_cap);
#else
	argv = (int32_t *)a1;
	envp = (int32_t *)a2;
	platform_clear_bss();
#endif

	mips_postboot_fixup();

	mips_pcpu0_init();
	platform_counter_freq = malta_cpu_freq();
	mips_timer_early_init(platform_counter_freq);
	init_static_kenv(boot1_env, sizeof(boot1_env));

	cninit();
	printf("entry: platform_start()\n");

	/*
	 * Parse kernel cmdline.
	 */
	if (bootverbose) {
		printf("cmd line: ");
		for (i = 0; i < argc; i++) {
			char *arg = platform_argv_ptr(argv[i]);
			printf("'%s' ", arg);
		}
		printf("\n");
	}

#ifdef CPU_QEMU_MALTA
	/*
	 * QEMU passes the -kernel path as argv[0] and the value of the
	 * -append command line flag as argv[1]. We must use
	 * boot_parse_cmdline() for argv[0] if we want to support multiple
	 * values to be set in the environment (This happens e.g. when a user
	 * passes -append "foo=bar value2=123"). We also skip argv[0] since it
	 * is not useful to have /path/to/kernel=1 in the environment.
	 */
	if (argc >= 2) {
		boothowto |= boot_parse_cmdline(platform_argv_ptr(argv[1]));
	} else {
		panic("QEMU has changed and allows multiple -append flags?");
	}
#else
	/* On non-QEMU CPUs we can parse argv normally. */
	for (i = 0; i < argc; i++)
		boothowto |= boot_parse_arg(platform_argv_ptr(argv[i]));
#endif

	if (bootverbose)
		printf("envp:\n");

	/*
	 * Parse the environment for things like ememsize.
	 */
	for (i = 0; envp[i]; i += 2) {
		const char *a = platform_argv_ptr(envp[i]);
		const char *v = platform_argv_ptr(envp[i+1]);
		if (bootverbose)
			printf("\t%s = %s\n", a, v);

		if (strcmp(a, "ememsize") == 0) {
			ememsize = strtoul(v, NULL, 0);
		}
	}

	if (bootverbose) {
		printf("memsize = %llu (0x%08x)\n",
		    (unsigned long long) memsize, memsize);
		printf("ememsize = %llu\n", (unsigned long long) ememsize);

#ifdef __mips_o32
		/*
		 * For O32 phys_avail[] can't address memory beyond 2^32,
		 * so cap extended memory to 2GB minus one page.
		 */
		if (ememsize >= 2ULL * 1024 * 1024 * 1024)
			ememsize = 2ULL * 1024 * 1024 * 1024 - PAGE_SIZE;
#endif
	}
	/*
	 * For <= 256MB RAM amounts, ememsize should equal memsize.
	 * For > 256MB RAM amounts it's the total RAM available;
	 * split between two banks.
	 *
	 * XXX TODO: just push realmem assignment into mips_init() ?
	 */
	realmem = btoc(ememsize);
	mips_init(memsize, ememsize);

	mips_timer_init_params(platform_counter_freq, -1);
}
// CHERI CHANGES START
// {
//   "updated": 20190702,
//   "target_type": "kernel",
//   "changes_purecap": [
//     "pointer_as_integer",
//     "support"
//   ]
// }
// CHERI CHANGES END
