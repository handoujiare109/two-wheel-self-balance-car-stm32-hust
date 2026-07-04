

#ifndef __MOTOR_H
#define __MOTOR_H

#include "./SYSTEM/sys/sys.h"

//�������Ŷ���
#define BIN2_Pin       GPIO_PIN_12
#define BIN2_GPIO_Port GPIOB
#define BIN1_Pin       GPIO_PIN_13
#define BIN1_GPIO_Port GPIOB
#define AIN1_Pin       GPIO_PIN_14
#define AIN1_GPIO_Port GPIOB
#define AIN2_Pin       GPIO_PIN_15
#define AIN2_GPIO_Port GPIOB

//PWM���Ŷ���
#define PWMA_Pin       GPIO_PIN_8
#define PWMA_GPIO_Port GPIOA
#define PWMB_Pin GPIO_PIN_11
#define PWMB_GPIO_Port GPIOA

#define PWMA   TIM1->CCR1  //�ȽϼĴ���
#define PWMB   TIM1->CCR4  //�ȽϼĴ���

//�������
#define BIN2(x)   do{ x ? \
                      HAL_GPIO_WritePin(GPIOB, BIN2_Pin, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(GPIOB, BIN2_Pin, GPIO_PIN_RESET); \
                  }while(0)    

#define BIN1(x)   do{ x ? \
                      HAL_GPIO_WritePin(GPIOB, BIN1_Pin, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(GPIOB, BIN1_Pin, GPIO_PIN_RESET); \
                  }while(0)   									
#define AIN2(x)   do{ x ? \
                      HAL_GPIO_WritePin(GPIOB, AIN2_Pin, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(GPIOB, AIN2_Pin, GPIO_PIN_RESET); \
                  }while(0) 
#define AIN1(x)   do{ x ? \
                      HAL_GPIO_WritePin(GPIOB, AIN1_Pin, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(GPIOB, AIN1_Pin, GPIO_PIN_RESET); \
                  }while(0) 

void motorInit(void);
void motorMove(int motor_left,int motor_right);
uint8_t Turn_Off(void);

									
#endif

