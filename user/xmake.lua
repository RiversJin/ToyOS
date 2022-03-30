cflags = {"-Wall", "-g", "-O0", "-fno-pie","-fno-pic","-fno-stack-protector",
            "-static", "-fno-builtin","-nostdlib","-ffreestanding", "-mgeneral-regs-only",
            "-MMD","-MP"}
ldflags = {"-N", "-e main", "-Ttext 0"}
ARCH = "aarch64-linux-gnu-"

toolchain("aarch64-linux-gnu")
    set_kind("standalone")
    set_toolset("cc",ARCH .. "gcc")
    set_toolset("ld",ARCH .. "ld")
    set_toolset("ar",ARCH .. "ar")
    set_toolset("as",ARCH .. "gcc")


target("user_lib")
    set_targetdir("$(projectdir)/build/user")
    set_toolchains("aarch64-linux-gnu")
    set_kind("static")
    add_files("src/lib/*.c")
    on_load(function(target)
        local projectdir = os.projectdir()
        local target_s = path.join(projectdir, path.translate("user/src/lib/usyscall.S"))
        local source_h = path.join(projectdir, path.translate("kernel/syscall/syscall.h"))
        local tool_path = path.join(projectdir, path.translate("tool/mksyscall/mksyscall.py"))
        if((not os.exists(target_s)) or os.mtime(source_h) > os.mtime(target_s) ) then
            cprint('${bright green}Re-generating usyscall.S${clear}')
            os.execv("python3",{tool_path, source_h}, {stdout = target_s})
            cprint('${bright green}usyscall.S generated${clear}')
        end
    end)
    after_clean(function(target)
        local projectdir = os.projectdir()
        local target_s = path.join(projectdir, path.translate("user/src/lib/usyscall.S"))
        os.tryrm(target_s)
    end)
    add_files("src/lib/usyscall.S")
    add_cflags(cflags, {force = true})
target_end()

tasknames = {}

for _, taskname in ipairs(os.dirs("src/*")) do
    taskname = path.basename(taskname)
    if taskname ~= "lib" then
        table.insert(tasknames, taskname)
        target(taskname)
            set_targetdir("$(projectdir)/build/user/bin")
            set_toolchains("aarch64-linux-gnu")
            set_kind("binary")
            add_files("src/" .. taskname .. "/*.c")
            add_cflags(cflags, {force = true})
            add_ldflags(ldflags, {force = true})
            add_deps("user_lib")
            after_build(function(target)
                local ARCH = "aarch64-linux-gnu-"
                local OBJDUMP = ARCH .. "objdump"
                local targetfile = target:targetfile()
                path.join(target:targetdir(),path.basename(target:filename()).."8.img")
                local asm_path = target:targetdir() .. "/../" .. path.basename(target:filename())
                os.execv(OBJDUMP, {"-S",targetfile}, {stdout = asm_path .. ".asm"})
            end)
        target_end()
    end
end

target("mkfs")
    set_kind("binary")
    add_files("$(projectdir)/tool/mkfs/*.c")
    set_targetdir("$(projectdir)/build/tool/")
target_end()

target("fs")
    set_kind("phony")
    add_deps("user_lib")
    add_deps("mkfs")
    for _, v in ipairs(tasknames) do
        add_deps(v)
    end
    after_build(function ()
        bins = ""
        for _, file in ipairs(os.files("$(projectdir)/build/user/bin/*")) do
            bins = bins .. " " .. file
        end
        os.exec("$(projectdir)/build/tool/mkfs" .. " " .. "$(projectdir)/build/fs.img" .. " " .. bins)
        cprint('${bright green}file system img is built --> ' .. "$(projectdir)/build/fs.img".. '${clear}')
    end)
    after_clean(function ()
        os.rm("$(projectdir)/build/fs.img")
        print("文件系统 清理完成")
    end)