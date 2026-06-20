# Makefile for my-bpf-sensor CO-RE eBPF program
#
# This Makefile demonstrates how to build a CO-RE eBPF program:
# 1. Compile the BPF C code to a .bpf.o object file with BTF
# 2. Generate a BPF skeleton header using bpftool
# 3. Compile the user-space loader linked against libbpf

CLANG ?= clang
BPFTOOL ?= bpftool
PKG_CONFIG ?= pkg-config

LIBBPF_CFLAGS := $(shell $(PKG_CONFIG) --cflags libbpf 2>/dev/null || echo "-I/usr/include/bpf")
LIBBPF_LIBS   := $(shell $(PKG_CONFIG) --libs libbpf 2>/dev/null || echo "-lbpf")

SRCDIR := src
BUILDDIR := build

BPF_C := $(SRCDIR)/bpf_program.bpf.c
BPF_O := $(BUILDDIR)/bpf_program.bpf.o
SKEL_H := $(BUILDDIR)/bpf_program.skel.h
LOADER_C := $(SRCDIR)/loader.c
LOADER_O := $(BUILDDIR)/loader.o
TARGET := $(BUILDDIR)/my-bpf-sensor

# BPF compilation flags for CO-RE
BPF_CFLAGS := -g -O2 -target bpf -D__TARGET_ARCH_x86
BPF_CFLAGS += -I/usr/include/x86_64-linux-gnu
BPF_CFLAGS += -I/usr/include/bpf
BPF_CFLAGS += -Wall

.PHONY: all clean

all: $(TARGET)

# Step 1: Compile BPF program to object file with BTF
$(BPF_O): $(BPF_C) | $(BUILDDIR)
	$(CLANG) $(BPF_CFLAGS) -c $< -o $@

# Step 2: Generate BPF skeleton header from BPF object
$(SKEL_H): $(BPF_O) | $(BUILDDIR)
	$(BPFTOOL) gen skeleton $< > $@

# Step 3: Compile user-space loader
$(LOADER_O): $(LOADER_C) $(SKEL_H) | $(BUILDDIR)
	$(CC) -g -O2 -I$(BUILDDIR) -c $< $(LIBBPF_CFLAGS) -o $@

# Step 4: Link final binary
$(TARGET): $(LOADER_O) | $(BUILDDIR)
	$(CC) -g -O2 $< $(LIBBPF_LIBS) -lelf -lz -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean