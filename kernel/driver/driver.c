//
// Created by dxy on 2020/4/2.
//

#include "driver.h"

#include "timer.h"
#include "keyboard.h"
#include "ide.h"

void DriverInit() {
    IdeInit();
    TimerInit();
    KeyboardInit();
}