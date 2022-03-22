ARCH := aarch64-linux-gnu
CC := $(ARCH)-gcc
LD := $(ARCH)-ld
OBJDUMP := $(ARCH)-objdump
OBJCOPY := $(ARCH)-objcopy

CFLAGS := -Wall -g -O0 \
          -fno-pie -fno-pic -fno-stack-protector \
          -static -fno-builtin -nostdlib -ffreestanding \
          -mgeneral-regs-only \
	      -MMD -MP -Isrc/lib/

KERNEL_SRC_DIR := ../kernel
USER_SRC_DIR := ./src
BUILD_DIR := ../build
TOOL_DIR := ../tool
MKSYSCALL_TOOL := $(TOOL_DIR)/mksyscall/mksyscall.py

USYSCALL_S := $(USER_SRC_DIR)/lib/usyscall.S

$(USYSCALL_S) : $(KERNEL_SRC_DIR)/syscall/syscall.h
	mkdir -p $(dir $@)
	python3 $(MKSYSCALL_TOOL) $^ > $@

$(BUILD_DIR)/user/obj/lib/usyscall.S.o: $(USER_SRC_DIR)/lib/usyscall.S
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^

$(BUILD_DIR)/user/obj/test/test.c.o: $(USER_SRC_DIR)/test/test.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^

$(BUILD_DIR)/user/bin/test: $(BUILD_DIR)/user/obj/test/test.c.o $(BUILD_DIR)/user/obj/lib/usyscall.S.o
	mkdir -p $(dir $@)
	$(LD) -N -e main -Ttext 0 -o $@ $^

all: $(BUILD_DIR)/user/bin/test

.PHONY: clean
clean:
	rm -rf $(USYSCALL_S)
	rm -rf $(BUILD_DIR)/user