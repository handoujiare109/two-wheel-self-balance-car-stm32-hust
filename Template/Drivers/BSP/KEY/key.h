#ifndef __KEY_H
#define __KEY_H

#include "stm32f1xx_hal.h"

#define KEY_PIN     GPIO_PIN_5
#define KEY_PORT    GPIOA

uint8_t key_scan(void);

#endif
