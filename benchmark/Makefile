SYSROOT=$(HOME)/cheri/output/morello-sdk/sysroot-morello-purecap
BUILDDIR=$(HOME)/cheri/build/benchmark-morello
CC=$(HOME)/cheri/output/morello-sdk/bin/clang
CPP=$(HOME)/cheri/output/morello-sdk/bin/clang++

CFLAGS=-Wall -g --sysroot=$(SYSROOT) -DCOMPILING_ON_ZENO -target aarch64-unknown-freebsd14.0 -march=morello+c64 -mabi=purecap -femulated-tls \
       -DTEST_LIBRARIES_ROOT=\"/home/gp472/testlibs\"

CPPLINKFLAGS=--sysroot=$(SYSROOT) -target aarch64-unknown-freebsd14.0 -march=morello+c64 -mabi=purecap -femulated-tls

all: benchmark-dlopen benchmark-dlopen_sandbox

benchmark-dlopen.o: dlopen-benchmark.c
	$(CC) -c -o $(BUILDDIR)/$@ $(CFLAGS) -DBENCHMARK_DLOPEN $< -dynamic

benchmark-dlopen_sandbox.o: dlopen-benchmark.c
	$(CC) -c -o $(BUILDDIR)/$@ $(CFLAGS) -DBENCHMARK_DLOPEN_SANDBOX $< -dynamic

benchmark-dlopen: benchmark-dlopen.o
	$(CPP) -o $(BUILDDIR)/$@ $(CPPLINKFLAGS) -L$(SYSROOT)/usr/lib $(BUILDDIR)/$< $(SYSROOT)/lib/libc.so.7 $(SYSROOT)/usr/lib/lib{pmc,xo,util}.so -dynamic -lutil

benchmark-dlopen_sandbox: benchmark-dlopen_sandbox.o
	$(CPP) -o $(BUILDDIR)/$@ $(CPPLINKFLAGS) -L$(SYSROOT)/usr/lib $(BUILDDIR)/$< $(SYSROOT)/lib/libc.so.7 $(SYSROOT)/usr/lib/lib{pmc,xo,util}.so -dynamic -lutil

cp-to-mdev: benchmark-dlopen benchmark-dlopen_sandbox
	scp $(BUILDDIR)/benchmark-dlopen $(BUILDDIR)/benchmark-dlopen_sandbox mdev:benchmark

clean:
	rm $(BUILDDIR)/*.o $(BUILDDIR)/benchmark-dlopen $(BUILDDIR)/benchmark-dlopen_sandbox
