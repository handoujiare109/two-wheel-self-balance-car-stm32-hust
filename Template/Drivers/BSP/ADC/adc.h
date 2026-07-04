#ifndef __ADC_H
#define __ADC_H

#include "stm32f1xx.h"  /* 直接用 CMSIS，不用 HAL */

void adc_init(void);
float Get_battery_volt(void);

#endif
