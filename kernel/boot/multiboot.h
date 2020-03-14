//
// Created by dxy on 2020/3/14.
//

#ifndef XYOS_MULTIBOOT_H
#define XYOS_MULTIBOOT_H

/* The magic field should contain this. */
#define MULTIBOOT_HEADER_MAGIC 0x1badb002

#define MULTIBOOT_ARCHITECTURE_I386 0

/* struct multiboot_header {
    multiboot_uint32_t magic;
    multiboot_uint32_t architecture;
    multiboot_uint32_t header_length;
    multiboot_uint32_t checksum;
}; */

#endif //XYOS_MULTIBOOT_H
