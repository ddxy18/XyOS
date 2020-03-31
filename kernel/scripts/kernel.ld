/* GRUB2 only supports kernels loaded at 1 megabyte or above. */

OUTPUT("obj/xy_kernel.elf")
ENTRY(_start)

SECTIONS
{
  . = 0x100000;
  .multiboot : {
    PROVIDE(_K_BEGIN = .);
    *(.multiboot)
  }
  .text : {
    *(.text)
  }
  .data : {
    *(.data)
  }
  .bss : {
    *(.bss)
  }
  PROVIDE(_K_END = .);
}