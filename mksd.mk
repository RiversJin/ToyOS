# SD卡镜像的路径
SD_IMG ?=
# 内核镜像路径
KERNEL_IMG ?=

BOOT_IMG := $(BUILD_DIR)/boot.img
FS_IMG := $(BUILD_DIR)/fs.img
# 总镜像大小为128M 其中启动分区64M 我们自己的64M
# 
SECTOR_SIZE := 512
# 128*1024*1024/512 = 262144
SECTORS := 262144
BOOT_OFFSET := 2048
# 64*1024*1024/512 = 131072
BOOT_SECTORS := 131072

FS_OFFSET := $(shell echo $$(($(BOOT_OFFSET) + $(BOOT_SECTORS))))
FS_SECTORS := $(shell echo $$(($(SECTORS) - $(FS_OFFSET))))

#如果生成失败 删除
.DELETE_ON_ERROR: $(BOOT_IMG) $(SD_IMG)

#先用dd创建一个指定大小的空文件(64M) 然后使用fat32文件系统将其格式化
$(BOOT_IMG): $(KERNEL_IMG) $(shell find boot/*)
	dd if=/dev/zero of=$@ seek=$(shell echo $$(($(BOOT_SECTORS) - 1))) bs=$(SECTOR_SIZE) count=1
	mkfs.vfat -F 32 -s 1 $@
	$(foreach x, $^, mcopy -i $@ $(x) ::$(notdir $(x));)

$(FS_IMG): $(shell find build/user/bin -type f)
	$(MAKE) -C $(USER_SRC_DIR)
	mkdir -p build/tool
	gcc -o build/tool/mkfs tool/mkfs/mkfs.c
	$(info Our filesystem files: $^)
	$(BUILD_DIR)/tool/mkfs $@ $^


$(SD_IMG): $(FS_IMG) $(BOOT_IMG)
	dd if=/dev/zero of=$@ seek=$(shell echo $$(($(SECTORS) - 1))) bs=$(SECTOR_SIZE) count=1
	@printf "\
	$(BOOT_OFFSET), $(shell echo $$(($(BOOT_SECTORS) * $(SECTOR_SIZE) / 1024)))K, c,\n\
	$(FS_OFFSET), $(shell echo $$(($(FS_SECTORS) * $(SECTOR_SIZE) / 1024)))K, L,\n\
	" | sfdisk $@
	dd if=$(BOOT_IMG) of=$@ seek=$(BOOT_OFFSET) conv=notrunc
	dd if=$(FS_IMG) of=$@ seek=$(FS_OFFSET) conv=notrunc






