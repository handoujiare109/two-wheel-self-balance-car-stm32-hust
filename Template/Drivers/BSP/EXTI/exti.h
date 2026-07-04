#ifndef __EXTI_H
#define __EXTI_H

#include "sys.h"

extern float pitch;          /* 小车倾角 */
extern float gyro_y;         /* 倾角速度 */
extern float battery_voltage;/* 电池电压 */
extern uint8_t balance_enable;

void exti_init(void);
void extix_key2_init(void);
float readVolt(uint8_t cnt);

#endif
