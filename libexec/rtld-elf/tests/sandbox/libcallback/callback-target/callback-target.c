/*-
 * Copyright 2022 Gabor Pituk.
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

#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *
get_as(int n)
{
	char *buf = malloc((n+1) * sizeof(char));
	memset(buf, 'a', n * sizeof(char));
	buf[n] = '\0';
	return buf;
}
static char *(*get_as_fn)(int) = get_as;

int
main(int argc, const char *argv[])
{
	if (argc != 3) {
		printf("Usage: <sandbox-target> <wrap with trampoline: 0 or 1> path-to-libhello_world.so\n");
		return -1;
	}

	bool wrap_with_trampoline = argv[1][0] == '1';

	void *handle = dlopen_sandbox(argv[2], RTLD_LAZY);
	if (!handle) {
		printf("dlopen_sandbox() failed: %s\n", dlerror());
		return 1;
	}
	char *(*b_then_as)(char *(*)(int)) = (char *(*)(char *(*)(int))) dlsym(handle, "b_then_as");
	if (!b_then_as) {
		printf("dlsym() failed: %s\n", dlerror());
		return 2;
	}

	if (wrap_with_trampoline) {
		get_as_fn = (char *(*)(int)) dlwrap_callback((void *) get_as);
	}

	char *msg = b_then_as(get_as_fn);
	printf("%s\n", msg);
	return 0;
}
