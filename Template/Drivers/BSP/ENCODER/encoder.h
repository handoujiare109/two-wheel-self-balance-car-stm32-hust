#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f1xx_hal.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4_encoder;

extern int encoder_left;
extern int encoder_right;

void leftEncoderInit(void);
void rightEncoderInit(void);
int Read_Encoder(TIM_HandleTypeDef *htim);
void encoder_read_all(void);

#endif
