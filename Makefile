ARCH := aarch64-linux-gnu
CC := $(ARCH)-gcc
LD := $(ARCH)-ld
OBJDUMP := $(ARCH)-objdump
OBJCOPY := $(ARCH)-objcopy

SRC_DIR := kernel
BUILD_DIR := build
USER_SRC_DIR := user

CFLAGS := -Wall -g -O0 \
          -fno-pie -fno-pic -fno-stack-protector \
          -static -fno-builtin -nostdlib -ffreestanding -nostartfiles \
          -mgeneral-regs-only \
	      -MMD -MP -Ikernel/
LINKER_SCRIPT := $(SRC_DIR)/linker.ld
KERNEL_ELF := $(BUILD_DIR)/kernel8.elf
KERNEL_IMG := $(BUILD_DIR)/kernel8.img
SD_IMG := $(BUILD_DIR)/sd.img

all: $(SD_IMG)

SRCS := $(shell find $(SRC_DIR) -name *.c -or -name *.S)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
-include $(DEPS) # 使得make根据gcc分析出的源代码依赖进行编译

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
$(BUILD_DIR)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(KERNEL_ELF): $(LINKER_SCRIPT) $(OBJS) $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.bin
	$(LD) -T $< -o $@  $(OBJS) -b binary $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.bin
	$(OBJDUMP) -S -D $@ > $(basename $@).asm
	$(OBJDUMP) -x $@ > $(basename $@).hdr

$(BUILD_DIR)/$(USER_SRC_DIR)/initcode.bin: $(USER_SRC_DIR)/initcode.S
	@echo + as $<
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.o $<
	@echo + ld $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.out
	$(LD) -N -e start -Ttext 0 -o $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.out $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.o
	@echo + objcopy $@
	$(OBJCOPY) -S -O binary --prefix-symbols="_binary_user_initcode" $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.out $@
	@echo + objdump $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.o
	$(OBJDUMP) -S $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.o > $(BUILD_DIR)/$(USER_SRC_DIR)/initcode.asm

$(KERNEL_IMG): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@
# 改用SD卡的形式
# QEMU := qemu-system-aarch64 -M raspi3 -nographic -serial null -chardev stdio,id=uart1 -serial chardev:uart1 -monitor none
-include mksd.mk
QEMU := qemu-system-aarch64 -M raspi3 -nographic -serial null -serial mon:stdio -drive file=$(SD_IMG),if=sd,format=raw

qemu: $(KERNEL_IMG) $(SD_IMG)
	$(QEMU) -kernel $<
qemu-gdb: $(KERNEL_IMG)
	$(QEMU) -kernel $< -S -gdb tcp::1234
gdb: 
	gdb -x .gdbinit

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)
