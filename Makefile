OBJ_DIR = obj

CC = gcc
LD = ld
ASM = as
QEMU = qemu-system-i386

C_FLAGS = -c -fno-builtin -ffreestanding -nostdlib -m32
LD_FLAGS = -T kernel/scripts/kernel.ld -m elf_i386
QEMU_FLAGS = -kernel

all:start.o boot.o
	$(LD) $(LD_FLAGS) $(OBJ_DIR)/start.o $(OBJ_DIR)/boot.o

start.o:kernel/boot/start.c
	$(CC) $(C_FLAGS) $< -o $(OBJ_DIR)/start.o

boot.o:kernel/boot/boot.S
	$(CC) $(C_FLAGS) kernel/boot/boot.S -o $(OBJ_DIR)/boot.o

qemu:
	$(QEMU) $(QEMU_FLAGS) $(OBJ_DIR)/xy_kernel.elf
	
clean:
	cd obj;rm -rf *.o xy_kernel.elf