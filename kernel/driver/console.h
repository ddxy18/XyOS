//
// Created by dxy on 2020/3/25.
//

#ifndef XYOS_CONSOLE_H
#define XYOS_CONSOLE_H

// Some screen stuff.
// The number of columns.
#define COLUMNS 80
// The number of lines.
#define LINES 24
// The attribute of an character.
#define ATTRIBUTE 7
// The video memory address.
#define VIDEO 0xB8000
// Variables.
// Save the X position.
static int xpos;
// Save the Y position.
static int ypos;
// Point to the video memory.
static volatile unsigned char *video;

/**
 * Format a string and print it on the screen, just like the libc function
 * printf.
 */
void printf(const char *format, ...);

/**
 * Convert the integer D to a string and save the string in BUF. If
 * BASE is equal to 'd', interpret that D is decimal, and if BASE is
 * equal to 'x', interpret that D is hexadecimal.
 */
void itoa(char *buf, int base, int d);

/**
 * Clear the screen and initialize VIDEO, XPOS and YPOS.
 */
void cls(void);

#endif // XYOS_CONSOLE_H
