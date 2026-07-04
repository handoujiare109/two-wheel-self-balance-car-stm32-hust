#ifndef __GTIM_H
#define __GTIM_H

#include "stm32f1xx_hal.h"

extern int speed_left;
extern int speed_right;
extern int target_speed;

void gtim_init(void);
int Vel_PI(int vel, int Target);

#endif
