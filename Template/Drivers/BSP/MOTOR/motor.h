#ifndef __MOTOR_H
#define __MOTOR_H

#include "sys.h"

/* TB6612 方向控制引脚 */
#define AIN1_Pin       GPIO_PIN_14
#define AIN1_GPIO_Port GPIOB
#define AIN2_Pin       GPIO_PIN_15
#define AIN2_GPIO_Port GPIOB
#define BIN1_Pin       GPIO_PIN_13
#define BIN1_GPIO_Port GPIOB
#define BIN2_Pin       GPIO_PIN_12
#define BIN2_GPIO_Port GPIOB

/* PWM 引脚 */
#define PWMA_Pin       GPIO_PIN_8
#define PWMA_GPIO_Port GPIOA
#define PWMB_Pin       GPIO_PIN_11
#define PWMB_GPIO_Port GPIOA

/* PWM 比较寄存器 */
#define PWMA   TIM1->CCR1
#define PWMB   TIM1->CCR4

/* 方向控制宏 */
#define AIN1(x)   do{ x ? \
                      HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(AIN1_GPIO_Port, AIN1_Pin, GPIO_PIN_RESET); \
                  }while(0)

#define AIN2(x)   do{ x ? \
                      HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(AIN2_GPIO_Port, AIN2_Pin, GPIO_PIN_RESET); \
                  }while(0)

#define BIN1(x)   do{ x ? \
                      HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(BIN1_GPIO_Port, BIN1_Pin, GPIO_PIN_RESET); \
                  }while(0)

#define BIN2(x)   do{ x ? \
                      HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(BIN2_GPIO_Port, BIN2_Pin, GPIO_PIN_RESET); \
                  }while(0)

#define PWM_MAX     7199
#define PWM_MIN     (-7199)

extern int motor_left;
extern int motor_right;
extern uint8_t motor_enable;

void motor_init(void);
void motorMove(int speedL, int speedR);
void turn_Off(void);
void PWM_Limit(int *pwm);

#endif
