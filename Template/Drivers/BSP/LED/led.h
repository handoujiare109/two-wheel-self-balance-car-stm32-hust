#ifndef __LED_H
#define __LED_H

#include "stm32f1xx_hal.h"

#define LED0_PIN    GPIO_PIN_4
#define LED0_PORT   GPIOA

void led_init(void);
void led_on(void);
void led_off(void);
void led_toggle(void);

#endif
