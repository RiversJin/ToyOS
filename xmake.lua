includes("user/xmake.lua")
includes("kernel/xmake.lua")

target("sd.img")
    set_default(true)
    add_deps("kernel.elf")
    add_deps("fs")
    set_targetdir("build")
    after_clean(function() 
        os.tryrm("build/sd.img") 
        os.tryrm("build/boot.img")
    end)
    on_build(function(target)
        SECTOR_SIZE  = 512
        -- 128*1024*1024/512 = 262144
        SECTORS = 262144
        BOOT_OFFSET = 2048
        -- 64*1024*1024/512 = 131072
        BOOT_SECTORS = 131072
        FS_OFFSET = BOOT_OFFSET + BOOT_SECTORS
        FS_SECTORS = SECTORS - FS_OFFSET
        sd_img = target:targetfile()
        kernel_img = "build/kernel8.img"
        fs_img = "build/fs.img"
        boot_img = "build/boot.img"
        cprint('${bright green}Generatint boot img...${clear}')
        local boot_img_cmd = "dd if=/dev/zero of=" .. boot_img .. " seek=" .. tostring(BOOT_SECTORS - 1) .. " bs=" .. tostring(SECTOR_SIZE) .. " count=1"
        print(boot_img_cmd)
        os.exec(boot_img_cmd)
        cprint('${bright green}Ok. Formatting it to Fat32...${clear}')
        format_cmd = "mkfs.vfat -F 32 -s 1 " .. boot_img
        print(format_cmd)
        os.exec(format_cmd)
        cprint('${bright green}Ok. ${clear}')
        boot_files = os.files("boot/*")
        for _, file in ipairs(boot_files) do
            copy_cmd = "mcopy -i " .. boot_img .. " " .. file .. " ::" .. path.filename(file)
            print(copy_cmd)
            os.exec(copy_cmd)
        end
        cprint('${bright green}boot img generated.${clear}')
        cprint('${bright green}Generating sd img...${clear}')
        cprint('${bright green}Generating emtpy sd.img${clear}')
        sd_cmd = "dd if=/dev/zero of=" .. sd_img .. " seek=" .. tostring(SECTORS - 1) .. " bs=" .. tostring(SECTOR_SIZE) .. " count=1"
        print(sd_cmd)
        os.exec(sd_cmd)
        cprint('${bright green}Ok. Partitionint the sd img...${clear}')
        partition_cmd = "echo '" .. tostring(BOOT_OFFSET) .. " ," .. tostring(BOOT_SECTORS*SECTOR_SIZE/1024) .. "K, c,\n" .. tostring(FS_OFFSET) .. ", " .. tostring(FS_SECTORS*SECTOR_SIZE/1024) .."K, L,' | sfdisk " .. sd_img
        --os.exec(partition_cmd)
        os.execv("bash",{"-c", partition_cmd})
        cprint('${bright green}Ok. Copying boot img to sd img...${clear}')
        copy_cmd = "dd if=" .. boot_img .. " of=" .. sd_img .. " seek=" .. tostring(BOOT_OFFSET) .. " conv=notrunc"
        print(copy_cmd)
        os.exec(copy_cmd)
        cprint('${bright green}Ok. Copying fs img to sd img...${clear}')
        copy_cmd = "dd if=" .. fs_img .. " of=" .. sd_img .. " seek=" .. tostring(FS_OFFSET) .. " conv=notrunc"
        print(copy_cmd)
        os.exec(copy_cmd)
        cprint('${bright green}Ok. ${clear}')
    end)