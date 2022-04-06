SYSROOT=$(HOME)/cheri/output/morello-sdk/sysroot-morello-purecap
BUILDDIR=$(HOME)/cheri/build/benchmark-morello
CC=$(HOME)/cheri/output/morello-sdk/bin/clang
CPP=$(HOME)/cheri/output/morello-sdk/bin/clang++

CFLAGS=-Wall -g --sysroot=$(SYSROOT) -DCOMPILING_ON_ZENO -target aarch64-unknown-freebsd14.0 -march=morello+c64 -mabi=purecap -femulated-tls \
       -DTEST_LIBRARIES_ROOT=\"/home/gp472/testlibs\"

CPPLINKFLAGS=--sysroot=$(SYSROOT) -target aarch64-unknown-freebsd14.0 -march=morello+c64 -mabi=purecap -femulated-tls

all: benchmark-dlopen benchmark-dlopen_sandbox benchmark-function-call benchmark-function-call-sandbox benchmark-dlsym benchmark-dlsym-sandbox benchmark-lazy-binding benchmark-lazy-binding-sandbox

libhello_world_1000.so: helloworld_1000.c
	$(CC) --shared -o $(BUILDDIR)/$@ $(CFLAGS) -Wl,-Bsymbolic $<

libhello_world_with_deps_10.so: helloworld_with_deps_10.c
	$(CC) --shared -o $(BUILDDIR)/$@ $(CFLAGS) -fno-builtin -lc -lm -Wl,-Bsymbolic $<

benchmark-dlopen.o: dlopen-benchmark.c
	$(CC) -c -o $(BUILDDIR)/$@ $(CFLAGS) -DBENCHMARK_DLOPEN $< -dynamic

benchmark-dlopen_sandbox.o: dlopen-benchmark.c
	$(CC) -c -o $(BUILDDIR)/$@ $(CFLAGS) -DBENCHMARK_DLOPEN_SANDBOX $< -dynamic

benchmark-function-call.o: function-call-benchmark.c
	$(CC) -c -o $(BUILDDIR)/$@ $(CFLAGS) -DBENCHMARK_DLOPEN $< -dynamic

benchmark-function-call-sandbox.o: function-call-benchmark.c
	$(CC) -c -o $(BUILDDIR)/$@ $(CFLAGS) -DBENCHMARK_DLOPEN_SANDBOX $< -dynamic

benchmark-dlsym.o: dlsym-benchmark.c
	$(CC) -c -o $(BUILDDIR)/$@ $(CFLAGS) -DBENCHMARK_DLOPEN $< -dynamic

benchmark-dlsym-sandbox.o: dlsym-benchmark.c
	$(CC) -c -o $(BUILDDIR)/$@ $(CFLAGS) -DBENCHMARK_DLOPEN_SANDBOX $< -dynamic

benchmark-lazy-binding.o: lazy-binding-benchmark.c
	$(CC) -c -o $(BUILDDIR)/$@ $(CFLAGS) -DBENCHMARK_DLOPEN $< -dynamic

benchmark-lazy-binding-sandbox.o: lazy-binding-benchmark.c
	$(CC) -c -o $(BUILDDIR)/$@ $(CFLAGS) -DBENCHMARK_DLOPEN_SANDBOX $< -dynamic

benchmark-dlopen: benchmark-dlopen.o
	$(CPP) -o $(BUILDDIR)/$@ $(CPPLINKFLAGS) -L$(SYSROOT)/usr/lib $(BUILDDIR)/$< $(SYSROOT)/lib/libc.so.7 $(SYSROOT)/usr/lib/lib{pmc,xo,util}.so -dynamic -lutil

benchmark-dlopen_sandbox: benchmark-dlopen_sandbox.o
	$(CPP) -o $(BUILDDIR)/$@ $(CPPLINKFLAGS) -L$(SYSROOT)/usr/lib $(BUILDDIR)/$< $(SYSROOT)/lib/libc.so.7 $(SYSROOT)/usr/lib/lib{pmc,xo,util}.so -dynamic -lutil

benchmark-function-call: benchmark-function-call.o
	$(CPP) -o $(BUILDDIR)/$@ $(CPPLINKFLAGS) -L$(SYSROOT)/usr/lib $(BUILDDIR)/$< $(SYSROOT)/lib/libc.so.7 $(SYSROOT)/usr/lib/lib{pmc,xo,util}.so -dynamic -lutil

benchmark-function-call-sandbox: benchmark-function-call-sandbox.o
	$(CPP) -o $(BUILDDIR)/$@ $(CPPLINKFLAGS) -L$(SYSROOT)/usr/lib $(BUILDDIR)/$< $(SYSROOT)/lib/libc.so.7 $(SYSROOT)/usr/lib/lib{pmc,xo,util}.so -dynamic -lutil

benchmark-dlsym: benchmark-dlsym.o
	$(CPP) -o $(BUILDDIR)/$@ $(CPPLINKFLAGS) -L$(SYSROOT)/usr/lib $(BUILDDIR)/$< $(SYSROOT)/lib/libc.so.7 $(SYSROOT)/usr/lib/lib{pmc,xo,util}.so -dynamic -lutil

benchmark-dlsym-sandbox: benchmark-dlsym-sandbox.o
	$(CPP) -o $(BUILDDIR)/$@ $(CPPLINKFLAGS) -L$(SYSROOT)/usr/lib $(BUILDDIR)/$< $(SYSROOT)/lib/libc.so.7 $(SYSROOT)/usr/lib/lib{pmc,xo,util}.so -dynamic -lutil

benchmark-lazy-binding: benchmark-lazy-binding.o
	$(CPP) -o $(BUILDDIR)/$@ $(CPPLINKFLAGS) -L$(SYSROOT)/usr/lib $(BUILDDIR)/$< $(SYSROOT)/lib/libc.so.7 $(SYSROOT)/usr/lib/lib{pmc,xo,util}.so -dynamic -lutil

benchmark-lazy-binding-sandbox: benchmark-lazy-binding-sandbox.o
	$(CPP) -o $(BUILDDIR)/$@ $(CPPLINKFLAGS) -L$(SYSROOT)/usr/lib $(BUILDDIR)/$< $(SYSROOT)/lib/libc.so.7 $(SYSROOT)/usr/lib/lib{pmc,xo,util}.so -dynamic -lutil

#cp-to-mdev: benchmark-dlopen benchmark-dlopen_sandbox
#	scp $(BUILDDIR)/benchmark-dlopen $(BUILDDIR)/benchmark-dlopen_sandbox mdev:benchmark

clean:
	rm $(BUILDDIR)/*.o $(BUILDDIR)/benchmark-dlopen $(BUILDDIR)/benchmark-dlopen_sandbox $(BUILDDIR)/benchmark-function-call $(BUILDDIR)/benchmark-function-call-sandbox  $(BUILDDIR)/benchmark-dlsym $(BUILDDIR)/benchmark-dlsym-sandbox $(BUILDDIR)/benchmark-lazy-binding $(BUILDDIR)/benchmark-lazy-binding-sandbox
