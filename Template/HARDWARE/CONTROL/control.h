#ifndef __CONTROL_H
#define __CONTROL_H

#include "stm32f1xx_hal.h"

/* 平衡控制参数（需要在调参时修改） */
extern float Balance_Kp;
extern float Balance_Kd;
extern int Velocity_Kp;
extern float Velocity_Ki;

extern int debug_balance_pwm;
extern int debug_speed_pwm;
extern int debug_final_left;
extern int debug_final_right;

void Balance_Control(void);

#endif
