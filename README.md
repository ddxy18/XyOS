# XyOS
It currently contains booting, memory manager, interrupt handler and thread scheduler.
## Prerequisites
- qemu
- gcc
- gdb(optional: If you want to debug the kernel, you should ensure that gdb has been installed correctly.)
## Getting started
- download the source code from <https://github.com/ddxy18/XyOS.git>
- use `make all` to boot the kernel
## Debug
 - Use `make qemu-debug` to boot the kernel in debug mode
 - Use `make debug` to call a gdb server to link the kernel, so we can debug the kernel through the gdb server as usual.
 ## Design
 - Paging is enabled and we use the buddy system to manage the whole physical memory.
 - We have complete the clock interrupt, keyboard interrupt and disk interrupt.
 - We use thread as the minimum unit in scheduling, so when complete the thread and process design, multithread processes may consume more quantum times than single thread process. Now Kernel threads are scheduled by the RR algorithm.
 ## TODO
 - file system
 - process
 - syscallios<br>
