/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 * Copyright 2021 Mariusz Zaborski <oshogbo@FreeBSD.org>
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <atf-c.h>
#include <dlfcn.h>

#include "./utils.h"

ATF_TC(libhelloworld_passes);
ATF_TC_HEAD(libhelloworld_passes, tc)
{
    atf_tc_set_md_var(tc, "descr",
        "Check that we can dlopen_sandbox() an empty library");
}
ATF_TC_BODY(libhelloworld_passes, tc)
{
    test_dlopen_sandbox_success("libhello_world.so.0");
}

ATF_TC(libprints_fails);
ATF_TC_HEAD(libprints_fails, tc)
{
    atf_tc_set_md_var(tc, "descr",
        "Check that dlopen_sandbox() fails for a library using printf");
}
ATF_TC_BODY(libprints_fails, tc)
{
    test_dlopen_sandbox_failure("libprints.so.0",
        "Invalid dependencies for sandboxing");
}

ATF_TC(libdo_raw_syscall_fails);
ATF_TC_HEAD(libdo_raw_syscall_fails, tc)
{
    atf_tc_set_md_var(tc, "descr",
        "Check that doing a raw syscall is not permitted in sandbox");
}
ATF_TC_BODY(libdo_raw_syscall_fails, tc)
{
    void *handle = test_dlopen_sandbox_success("libdo_raw_syscall.so.0");
    const char *(*hello_world)(void) =
            (const char *(*)(void)) dlsym(handle, "hello_world");
    ATF_CHECK(NULL != hello_world);

    const char *msg = hello_world();
    ATF_CHECK_STREQ_MSG("FAIL", msg, "Syscall was permitted in sandbox");
}

static char *
get_as(int n)
{
	char *buf = malloc((n+1) * sizeof(char));
	memset(buf, 'a', n * sizeof(char));
	buf[n] = '\0';
	return buf;
}
static char *(*get_as_fn)(int) = get_as;

ATF_TC(libcallback_without_dlwrap_callback_fails);
ATF_TC_HEAD(libcallback_without_dlwrap_callback_fails, tc)
{
	// TODO: run this test case
	atf_tc_set_md_var(tc, "descr",
		"Check that on callback return to sandbox without trampoline we get SIGPROT");
}
ATF_TC_BODY(libcallback_without_dlwrap_callback_fails, tc)
{
	void *handle = test_dlopen_sandbox_success("libcallback.so.0");
	char *(*b_then_as)(char *(*)(int)) =
			(char *(*)(char *(*)(int))) dlsym(handle, "b_then_as");
	ATF_CHECK(NULL != b_then_as);

	atf_tc_expect_signal(34, "SIGPROT should be received when returning from Executive "
							 "callback into sandbox without a trampoline.");
	(void)b_then_as(get_as_fn);
}

ATF_TC(libcallback_with_dlwrap_callback_passes);
ATF_TC_HEAD(libcallback_with_dlwrap_callback_passes, tc)
{
	// TODO: run this test case
	atf_tc_set_md_var(tc, "descr",
					  "Check that on callback return to sandbox with trampoline should pass");
}
ATF_TC_BODY(libcallback_with_dlwrap_callback_passes, tc)
{
	void *handle = test_dlopen_sandbox_success("libcallback.so.0");
	char *(*b_then_as)(char *(*)(int)) =
	(char *(*)(char *(*)(int))) dlsym(handle, "b_then_as");
	ATF_CHECK(NULL != b_then_as);

	atf_tc_expect_signal(34, "SIGPROT should be received when returning from Executive "
							 "callback into sandbox without a trampoline.");
	get_as_fn = (char *(*)(int)) dlwrap_callback((void *)get_as_fn);
	ATF_CHECK(NULL != b_then_as(get_as_fn));
}

/* Register test cases with ATF. */
ATF_TP_ADD_TCS(tp)
{

	ATF_TP_ADD_TC(tp, libhelloworld_passes);
	ATF_TP_ADD_TC(tp, libprints_fails);
	ATF_TP_ADD_TC(tp, libdo_raw_syscall_fails);
	ATF_TP_ADD_TC(tp, libcallback_without_dlwrap_callback_fails);
	ATF_TP_ADD_TC(tp, libcallback_with_dlwrap_callback_passes);

	return atf_no_error();
}
