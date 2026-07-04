#ifndef __ATK_MS6050_IIC_H
#define __ATK_MS6050_IIC_H

#include "sys.h"

/* I2C pin definitions: PB8=SCL, PB9=SDA */
#define ATK_MS6050_IIC_SCL_GPIO_PORT            GPIOB
#define ATK_MS6050_IIC_SCL_GPIO_PIN             GPIO_PIN_8
#define ATK_MS6050_IIC_SCL_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)

#define ATK_MS6050_IIC_SDA_GPIO_PORT            GPIOB
#define ATK_MS6050_IIC_SDA_GPIO_PIN             GPIO_PIN_9
#define ATK_MS6050_IIC_SDA_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)

#define ATK_MS6050_IIC_SCL(x)                   do{ x ? \
                                                    HAL_GPIO_WritePin(ATK_MS6050_IIC_SCL_GPIO_PORT, ATK_MS6050_IIC_SCL_GPIO_PIN, GPIO_PIN_SET) : \
                                                    HAL_GPIO_WritePin(ATK_MS6050_IIC_SCL_GPIO_PORT, ATK_MS6050_IIC_SCL_GPIO_PIN, GPIO_PIN_RESET); \
                                                }while(0)

#define ATK_MS6050_IIC_SDA(x)                   do{ x ? \
                                                    HAL_GPIO_WritePin(ATK_MS6050_IIC_SDA_GPIO_PORT, ATK_MS6050_IIC_SDA_GPIO_PIN, GPIO_PIN_SET) : \
                                                    HAL_GPIO_WritePin(ATK_MS6050_IIC_SDA_GPIO_PORT, ATK_MS6050_IIC_SDA_GPIO_PIN, GPIO_PIN_RESET); \
                                                }while(0)

#define ATK_MS6050_IIC_READ_SDA()               HAL_GPIO_ReadPin(ATK_MS6050_IIC_SDA_GPIO_PORT, ATK_MS6050_IIC_SDA_GPIO_PIN)

void atk_ms6050_iic_start(void);
void atk_ms6050_iic_stop(void);
uint8_t atk_ms6050_iic_wait_ack(void);
void atk_ms6050_iic_ack(void);
void atk_ms6050_iic_nack(void);
void atk_ms6050_iic_send_byte(uint8_t dat);
uint8_t atk_ms6050_iic_read_byte(uint8_t ack);
void atk_ms6050_iic_init(void);

#endif
