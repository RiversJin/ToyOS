ARCH := aarch64-linux-gnu
CC := $(ARCH)-gcc
LD := $(ARCH)-ld
OBJDUMP := $(ARCH)-objdump
OBJCOPY := $(ARCH)-objcopy

SRC_DIR := kernel
BUILD_DIR := build

CFLAGS := -Wall -g -O2 \
          -fno-pie -fno-pic -fno-stack-protector \
          -static -fno-builtin -nostdlib -ffreestanding -nostartfiles \
          -mgeneral-regs-only \
	      -MMD -MP -Iinc
LINKER_SCRIPT := $(SRC_DIR)/linker.ld
KERNEL_ELF := $(BUILD_DIR)/kernel8.elf
KERNEL_IMG := $(BUILD_DIR)/kernel8.img

all: $(KERNEL_IMG)

SRCS := $(shell find $(SRC_DIRS) -name *.c -or -name *.S)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
-include $(DEPS) # 使得make根据gcc分析出的源代码依赖进行编译

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
$(BUILD_DIR)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KERNEL_ELF): $(LINKER_SCRIPT) $(OBJS)
	$(LD) -o $@ -T $< $(OBJS)
	$(OBJDUMP) -S -D $@ > $(basename $@).asm
	$(OBJDUMP) -x $@ > $(basename $@).hdr

$(KERNEL_IMG): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

QEMU := qemu-system-aarch64 -M raspi3 -nographic -serial null -chardev stdio,id=uart1 -serial chardev:uart1 -monitor none

qemu: $(KERNEL_IMG) 
	$(QEMU) -kernel $<
qemu-gdb: $(KERNEL_IMG)
	$(QEMU) -kernel $< -S -gdb tcp::1234
gdb: 
	gdb -x .gdbinit

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)
