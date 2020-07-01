cmake_minimum_required(VERSION 3.14)
project(XyOS C)

set(CMAKE_C_STANDARD 11)

add_executable(XyOS kernel/interrupt/interrupt.c kernel kernel/init.c kernel/driver/console.c kernel/driver/console.h kernel/init.h kernel/test/test.c kernel/test/test.h kernel/interrupt/apic/io_apic.c kernel/interrupt/apic/io_apic.h kernel/interrupt/pic.c kernel/interrupt/pic.h kernel/driver/timer.c kernel/driver/timer.h kernel/driver/driver.c kernel/driver/driver.h kernel/driver/keyboard.c kernel/driver/keyboard.h kernel/lib/asm_wrapper.c kernel/lib/asm_wrapper.h kernel/lib/map.h kernel/lib/bit.h test/bitmap_test.c kernel/driver/ide.c kernel/driver/ide.h kernel/lib/queue.h kernel/thread/thread.c kernel/thread/thread.h kernel/file_system/fs.c kernel/file_system/fs.h kernel/file_system/file.c kernel/file_system/file.h kernel/syscall/syscall.c kernel/syscall/syscall.h)