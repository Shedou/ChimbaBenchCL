# Native build using the system OpenCL headers and ICD loader.
CC ?= cc
CPPFLAGS += -DCL_TARGET_OPENCL_VERSION=100 -DCBCL_USE_SYSTEM_GLIBC
CFLAGS ?= -O2
LDLIBS += -lOpenCL
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
INSTALL ?= install

.PHONY: all install clean

all: ChimbaBenchCL-Linux

ChimbaBenchCL-Linux: $(wildcard src/*.c)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ src/main.c $(LDLIBS)

install: ChimbaBenchCL-Linux
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 755 ChimbaBenchCL-Linux $(DESTDIR)$(BINDIR)/ChimbaBenchCL-Linux

clean:
	$(RM) ChimbaBenchCL-Linux
