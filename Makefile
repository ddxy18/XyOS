OBJ_DIR=obj

C_SOURCE =init.o console.o interrupt.o trap.o memory_manager.o direct_mapping_allocator.o paging.o segment.o \
 idt.o pic.o timer.o driver.o keyboard.o asm_wrapper.o process.o ide.o thread.o syscall.o
ASM_SOURCE =boot.o intr_entry.o intr_vec.o switch_reg.o
LD_SCRIPT =kernel/scripts/kernel.ld

vpath %.c kernel
vpath %.c kernel/boot
vpath %.c kernel/driver
vpath %.c kernel/interrupt
vpath %.c kernel/lib
vpath %.c kernel/memory
vpath %.c kernel/test
vpath %.c kernel/process
vpath %.c kernel/thread
vpath %.c kernel/syscall
vpath %.h kernel
vpath %.h kernel/boot
vpath %.h kernel/driver
vpath %.h kernel/interrupt
vpath %.h kernel/lib
vpath %.h kernel/memory
vpath %.h kernel/test
vpath %.h kernel/process
vpath %.h kernel/thread
vpath %.h kernel/syscall
vpath %.S kernel/boot
vpath %.S kernel/interrupt
vpath %.S kernel/thread
vpath %.sh kernel/scripts
vpath %.o $(OBJ_DIR)

CC = gcc
LD = ld
ASM = as
QEMU = qemu-system-i386

C_FLAGS = -c -g -Wall -fno-builtin -ffreestanding -nostdlib -m32
LD_FLAGS = -T $(LD_SCRIPT) -m elf_i386
QEMU_FLAGS = -drive format=raw,file=obj/disk.img,if=ide,index=1,media=disk -kernel
QEMU_DEBUG_FLAGS = -S -s -no-reboot -no-shutdown

all:$(C_SOURCE) $(ASM_SOURCE)
	$(LD) $(LD_FLAGS) -o $(OBJ_DIR)/xy_kernel.elf $^

$(C_SOURCE): %.o: %.c %.h
	$(CC) $(C_FLAGS) $< -o $(OBJ_DIR)/$@

$(ASM_SOURCE): %.o: %.S
	$(CC) $(C_FLAGS) $< -o $(OBJ_DIR)/$@

intr_vec: kernel/scripts/intr_vec.sh
	sh ./kernel/scripts/intr_vec.sh

qemu:
	$(QEMU) $(QEMU_FLAGS) $(OBJ_DIR)/xy_kernel.elf

qemu-debug:
	$(QEMU) $(QEMU_FLAGS) $(OBJ_DIR)/xy_kernel.elf $(QEMU_DEBUG_FLAGS)
	
debug:
	gdb $(OBJ_DIR)/xy_kernel.elf;target remote localhost:1234
	
clean:
	cd obj;rm -rf *.o xy_kernel.elf

.PHONY: qemu clean