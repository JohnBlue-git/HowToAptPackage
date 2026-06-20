CC ?= gcc
CFLAGS ?= -g -O2
SRCDIR := src
BUILDDIR := build
TARGET := $(BUILDDIR)/my-daemon
SRC := $(SRCDIR)/main.c

.PHONY: all clean

all: $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(TARGET): $(SRC) | $(BUILDDIR)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(BUILDDIR)
