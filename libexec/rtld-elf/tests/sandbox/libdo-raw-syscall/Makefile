# $FreeBSD$

.include <bsd.own.mk>

LIB=		do_raw_syscall
SHLIB_MAJOR=	0

LIBDIR=		${TESTSBASE}/libexec/rtld-elf/sandbox
SHLIBDIR=	${TESTSBASE}/libexec/rtld-elf/sandbox

SRCS=		do_raw_syscall.c getrandom.S

CFLAGS+= -I${SRCTOP}/lib/libc/${MACHINE_CPUARCH}

.include <bsd.lib.mk>
