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
    set_targetdir("../build/user")
    set_toolchains("aarch64-linux-gnu")
    set_kind("static")
    add_files("src/lib/*.c")
    on_load(function(target)
        if((not os.exists("src/lib/usyscall.S")) or os.mtime("../kernel/syscall/syscall.h") > os.mtime("src/lib/usyscall.S") ) then
            cprint('${bright green}Re-generating usyscall.S${clear}')
            os.execv("python3",{"../tool/mksyscall/mksyscall.py", "../kernel/syscall/syscall.h"}, {stdout = "src/lib/usyscall.S"})
            cprint('${bright green}usyscall.S generated${clear}')
        end
    end)
    after_clean(function(target)
        os.tryrm("src/lib/usyscall.S")
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
            set_targetdir("../build/user/bin")
            set_toolchains("aarch64-linux-gnu")
            set_kind("binary")
            add_files("src/" .. taskname .. "/*.c")
            add_cflags(cflags, {force = true})
            add_ldflags(ldflags, {force = true})
            add_deps("user_lib")
        target_end()
    end
end

target("mkfs")
    set_kind("binary")
    add_files("../tool/mkfs/*.c")
    set_targetdir("../build/tool/")
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
        for _, file in ipairs(os.files("../build/user/bin/*")) do
            bins = bins .. " " .. file
        end
        os.exec("../build/tool/mkfs" .. " " .. "../build/fs.img" .. bins)
        print("文件系统 构建完成")
    end)
    after_clean(function ()
        os.rm("../build/fs.img")
        print("文件系统 清理完成")
    end)