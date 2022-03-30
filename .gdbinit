set architecture aarch64
set mi-async
set pagination off
file build/kernel.elf
target remote localhost:1234
