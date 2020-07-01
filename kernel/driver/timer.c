//
// Created by dxy on 2020/4/2.
//

#include "timer.h"

#include <stdint.h>

#include "../interrupt/pic.h"
#include "../interrupt/interrupt.h"
#include "../lib/asm_wrapper.h"
#include "console.h"
#include "../thread/thread.h"

static const int kChannel0Port = 0x40;
static const int kModeRegPort = 0x43;

/**
 * select channel: channel0
 * Access mode: lobyte/hibyte
 * operating mode: mode 3 (square wave generator)
 */
static const int kModeRegVal = 0x36;
static const int kCounterLatchCmd = 0x00000000;
static const int kReloadVal = 0x0;
/**
 * @def how many timer interrupts before next process switch
 */
static const int kTimerReset = 0xf;

static void TimerHandler(IntrReg *intr_reg);

uint8_t timer;

void TimerInit() {

    outb(kModeRegPort, kModeRegVal);

    outb(kChannel0Port, kReloadVal);
    outb(kChannel0Port, (unsigned) kReloadVal >> 8u);

    RegIntrHandler(kTimerVec, TimerHandler);
    timer = kTimerReset;
    EnableIrq(kTimerVec);
}

static void TimerHandler(IntrReg *intr_reg) {
    timer--;
    SendEoi(intr_reg->intr_vec_no);
    if (timer == 0) {
        timer = kTimerReset;
        printf("Process switch happens.\n");
        ThreadSwitch();
    }
}


