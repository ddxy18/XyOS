//
// Created by dxy on 2020/4/2.
//

#ifndef XYOS_PIC_H
#define XYOS_PIC_H

#include <stdint.h>

void PicInit();

void SendEoi(int intr_vec);

void EnableIrq(int intr_vec);

void DisableIrq(int intr_vec);

void EnableAllIrqs();

#endif //XYOS_PIC_H
