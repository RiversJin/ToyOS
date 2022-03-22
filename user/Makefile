ARCH := aarch64-linux-gnu
CC := $(ARCH)-gcc
LD := $(ARCH)-ld
OBJDUMP := $(ARCH)-objdump
OBJCOPY := $(ARCH)-objcopy

KERNEL_SRC_DIR := ../kernel
USER_SRC_DIR := ./src
BUILD_DIR := ../build
TOOL_DIR := ../tool
MKSYSCALL_TOOL := $(TOOL_DIR)/mksyscall/mksyscall.py

USYSCALL_S := $(USER_SRC_DIR)/lib/usyscall.S

$(USYSCALL_S) : $(KERNEL_SRC_DIR)/syscall/syscall.h
	mkdir -p $(dir $@)
	python3 $(MKSYSCALL_TOOL) $^ > $@



.PHONY: clean
clean:
	rm -rf $(USYSCALL_S)