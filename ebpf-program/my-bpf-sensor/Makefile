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

BPF_C := src/bpf_program.bpf.c
BPF_O := src/bpf_program.bpf.o
SKEL_H := src/bpf_program.skel.h
LOADER_C := src/loader.c
LOADER_O := src/loader.o
TARGET := my-bpf-sensor

# BPF compilation flags for CO-RE
BPF_CFLAGS := -g -O2 -target bpf -D__TARGET_ARCH_x86
BPF_CFLAGS += -I/usr/include/x86_64-linux-gnu
BPF_CFLAGS += -I/usr/include/bpf
BPF_CFLAGS += -Wall -Werror

all: $(TARGET)

# Step 1: Compile BPF program to object file with BTF
$(BPF_O): $(BPF_C)
	$(CLANG) $(BPF_CFLAGS) -c $< -o $@

# Step 2: Generate BPF skeleton header from BPF object
$(SKEL_H): $(BPF_O)
	$(BPFTOOL) gen skeleton $< > $@

# Step 3: Compile user-space loader
$(LOADER_O): $(LOADER_C) $(SKEL_H)
	$(CC) -g -O2 -c $< $(LIBBPF_CFLAGS) -o $@

# Step 4: Link final binary
$(TARGET): $(LOADER_O)
	$(CC) -g -O2 $< $(LIBBPF_LIBS) -lelf -lz -o $@

clean:
	rm -f src/*.o src/*.skel.h $(TARGET)

.PHONY: all clean