//
// Created by dxy on 2020/4/3.
//

#include "keyboard.h"

#include "../lib/asm_wrapper.h"
#include "../interrupt/idt.h"
#include "../interrupt/interrupt.h"
#include "../interrupt/pic.h"
#include "../lib/map.h"

static const int kPs2DataPort = 0x60;
static const int kStatusReg = 0x64;
static const int kCmdReg = 0x64;

// use 1 KB memory to store keyboard input
#define kKeyboardBufSize 0x400

/**
 * @def controller configuration byte
 * @verbatim
 * format: 0fed0cba
 * a	first PS/2 port interrupt (1 = enabled, 0 = disabled)
 * b	second PS/2 port interrupt (1 = enabled, 0 = disabled, only if 2 PS/2
 *      ports supported)
 * c	system Flag (1 = system passed POST, 0 = your OS shouldn't be running)
 * d	first PS/2 port clock (1 = disabled, 0 = enabled)
 * e	second PS/2 port clock (1 = disabled, 0 = enabled, only if 2 PS/2
 *      ports supported)
 * f	first PS/2 port translation (1 = enabled, 0 = disabled)
 * @endverbatim
 */
static const int kCtrlConfig = 0b00000100;

// controller command
static const int kDisablePort1 = 0xad;
static const int kEnablePort1 = 0xae;
static const int kDisablePort2 = 0xa7;
static const int kEnablePort2 = 0xa8;
static const int kCtrlSelfTest = 0xaa;
// test first PS/2 port
static const int kTestPort1 = 0xab;
static const int kRCtrlConfig = 0x20;
static const int kWCtrlConfig = 0x60;
static const int kRCtrlOutput = 0xd0;
static const int kWCtrlOutput = 0xd1;

// keyboard command
static const int kResetDevice = 0xff;

// keyboard response
static const int kKeyboardAck = 0xfa;
static const int kKeyboardResend = 0xfe;

// key scan code and ascii maps
Map *keymap;

/**
 * Record how many bytes have been received since a key press. So we can
 * distinguish between different key presses.
 */
static uint8_t key_counter;

// TODO: determine keyboard input buffer
// keyboard input buffer, 'cur' points to the next free element
unsigned char input_buf[kKeyboardBufSize];
static uint32_t cur;

static uint8_t Read(uint16_t port);

static void Write(uint16_t port, uint8_t data);

void KeymapInit();

static void KeyboardHandler(IntrReg *intr_reg);

bool KeyboardInit() {
    // disable two ports
    Write(kCmdReg, kDisablePort1);
    Write(kCmdReg, kDisablePort2);
    // flush the output buffer
    // We will not use 'read(uint16_t)' here since output buffer may be empty.
    inb(kPs2DataPort);
    // set the controller configuration byte
    Write(kCmdReg, kWCtrlConfig);
    Write(kPs2DataPort, kCtrlConfig);
    // controller self test
    Write(kCmdReg, kCtrlSelfTest);
    if (Read(kPs2DataPort) != 0x55u) {
        // test failed
        return FALSE;
    }
    // test port1
    Write(kCmdReg, kTestPort1);
    if (Read(kPs2DataPort) != 0) {
        // test failed
        return FALSE;
    }
    // enable port1 for keyboard
    Write(kCmdReg, kEnablePort1);
    // reset keyboard
    Write(kPs2DataPort, kResetDevice);
    /**
     * If reset command successes, we firstly received 0xfa(command
     * acknowledged), then 0xaa(self test successful). 'Read' will block when
     * no device exists, so we use 'inb' to clear the first response data.
     */
    Read(kPs2DataPort);
    if (inb(kPs2DataPort) != 0xaau) {
        return FALSE;
    }
    // initialize data structure used to handle keyboard interrupts
    key_counter = 0;
    cur = 0;
    KeymapInit();
    // enable port1 interrupt
    Write(kCmdReg, kWCtrlConfig);
    Write(kPs2DataPort, (unsigned) kCtrlConfig | 0x1u);
    RegIntrHandler(kKeyboardVec, KeyboardHandler);
    EnableIrq(kKeyboardVec);
    return TRUE;
}

void free_keyboard_buffer() {
    cur = 0;
    key_counter = 0;
}

static uint8_t Read(uint16_t port) {
    // wait for output
    while ((inb(kStatusReg) & 0x1u) == 0);
    return inb(port);
}

static void Write(uint16_t port, uint8_t data) {
    // wait for last input read
    while ((inb(kStatusReg) & 0x2u) != 0);
    outb(port, data);
}

void KeymapInit() {
    keymap = map();
    // TODO: insert all 26 characters and 10 numbers scan code and ascii code
}

/**
 * We use scan code set 2. Now it only handles 26 characters and 10 numbers.
 * These keys will generate 8 bit 0x?? when a key is pressed and 16 bit 0xf0,
 * 0x?? when a key is released.
 *
 * @param intr_reg
 */
static void KeyboardHandler(IntrReg *intr_reg) {
    unsigned char input = Read(kPs2DataPort);
    switch (key_counter % 3) {
        case 0:
            input_buf[cur] = (char) MapGet(keymap, input);
            cur++;
            ++key_counter;
            break;
        case 1:
            if (input != 0xf0u) {
                // invalid input
                cur--;
                ++key_counter;
                break;
            }
        case 2:
            key_counter = 0;
            break;
    }
    SendEoi(intr_reg->intr_vec_no);
}