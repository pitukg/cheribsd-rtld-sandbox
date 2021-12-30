# $FreeBSD$

.include <src.opts.mk>

SUBDIR+=	libhelloworld libprints libpythagoras target dlopen_sandbox_target

SUBDIR_DEPEND_target=	libhelloworld libprints libpythagoras

ATF_TESTS_C=	ld_library_pathfds
ATF_TESTS_C+=	ld_preload_fds
ATF_TESTS_C+=	ld_sandbox_checks

.for t in ${ATF_TESTS_C}
SRCS.$t=	$t.c common.c
.endfor

WARNS?=		3

.if ${MACHINE_ABI:Mpurecap}
SUBDIR+=	cheri
.endif

.include <bsd.test.mk>
