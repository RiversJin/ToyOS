initcode_cflags = {"-Wall", "-g", "-O0", "-fno-pie","-fno-pic","-fno-stack-protector",
            "-static", "-fno-builtin","-nostdlib","-ffreestanding", "-mgeneral-regs-only",
            "-MMD","-MP"}
initcode_ldflags = {"-N", "-e start", "-Ttext 0"}
cflags = {"-Wall", "-g", "-O0", "-fno-pie","-fno-pic","-fno-stack-protector",
            "-static", "-fno-builtin","-nostdlib","-ffreestanding", "-nostartfiles","-mcmodel=large" ,"-mgeneral-regs-only",
            "-MMD","-MP", "-Ikernel"}
ARCH = "aarch64-linux-gnu-"
OBJDUMP = ARCH .. "objdump"
OBJCOPY = ARCH .. "objcopy"

toolchain("aarch64-linux-gnu")
    set_kind("standalone")
    set_toolset("cc",ARCH .. "gcc")
    set_toolset("ld",ARCH .. "ld")
    set_toolset("ar",ARCH .. "ar")
    set_toolset("as",ARCH .. "gcc")

target("initcode.out")
    set_targetdir("$(projectdir)/build/user")
    set_toolchains("aarch64-linux-gnu")
    add_cflags(initcode_cflags, {force = true})
    add_ldflags(initcode_ldflags, {force=true})
    add_files("$(projectdir)/user/initcode.S")
    set_kind("binary")
    after_build(function (target)
        local ARCH = "aarch64-linux-gnu-"
        local OBJDUMP = ARCH .. "objdump"
        local OBJCOPY = ARCH .. "objcopy"
        local target_elf = target:targetfile()
        local target_bin = path.join(target:targetdir(),path.basename(target:filename())..".bin")
        local target_asm = path.join(target:targetdir(),path.basename(target:filename())..".asm")
        local cmd = OBJCOPY .. " " .. "-S -O binary" .. " " .. target_elf .. " " .. target_bin
        --print(cmd)
        os.exec(cmd)
        os.execv(OBJDUMP, {"-S",target_elf}, {stdout = target_asm})
        cprint('${bright green}initcode.bin generated.${clear}')
    end)

target("kernel.elf")
    add_deps("initcode.out")
    set_toolchains("aarch64-linux-gnu")
    set_kind("binary")
    add_files("$(projectdir)/kernel/**.S")
    add_files("$(projectdir)/kernel/**.c")
    add_files("$(projectdir)/kernel/linker.ld")
    add_includedirs("$(buildir)/kernel")
    set_targetdir("$(projectdir)/build")
    add_cflags(cflags, {force = true})
    add_asflags(cflags, {force = true})
    add_ldflags("-T $(projectdir)/kernel/linker.ld",{force=true})
    add_ldflags("-b binary build/user/initcode.bin",{force=true})
    after_build(function (target)
        local ARCH = "aarch64-linux-gnu-"
        local OBJDUMP = ARCH .. "objdump"
        local OBJCOPY = ARCH .. "objcopy"
        local target_elf = target:targetfile()
        local target_bin = path.join(target:targetdir(),path.basename(target:filename()).."8.img")
        local cmd = OBJCOPY .. " " .. "-S -O binary" .. " " .. target_elf .. " " .. target_bin
        os.exec(cmd)
        cprint('${bright green}kernel8.img generated.${clear}')
    end)

