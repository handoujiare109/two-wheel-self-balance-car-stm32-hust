#ifndef __ATK_MS6050_H
#define __ATK_MS6050_H

#include "sys.h"

/* AD0 pin: PA15 (requires JTAG disable) */
#define ATK_MS6050_AD0_GPIO_PORT            GPIOA
#define ATK_MS6050_AD0_GPIO_PIN             GPIO_PIN_15
#define ATK_MS6050_AD0_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOA_CLK_ENABLE();   \
                                                __HAL_RCC_AFIO_CLK_ENABLE();    \
                                                __HAL_AFIO_REMAP_SWJ_NOJTAG();  \
                                            }while(0)

#define ATK_MS6050_AD0(x)                   do{ x ? \
                                                HAL_GPIO_WritePin(ATK_MS6050_AD0_GPIO_PORT, ATK_MS6050_AD0_GPIO_PIN, GPIO_PIN_SET) : \
                                                HAL_GPIO_WritePin(ATK_MS6050_AD0_GPIO_PORT, ATK_MS6050_AD0_GPIO_PIN, GPIO_PIN_RESET); \
                                            }while(0)

/* I2C address (AD0=LOW → 0x68) */
#define ATK_MS6050_IIC_ADDR     0x68

/* Register definitions */
#define MPU_ACCEL_OFFS_REG      0X06
#define MPU_PROD_ID_REG         0X0C
#define MPU_SELF_TESTX_REG      0X0D
#define MPU_SELF_TESTY_REG      0X0E
#define MPU_SELF_TESTZ_REG      0X0F
#define MPU_SELF_TESTA_REG      0X10
#define MPU_SAMPLE_RATE_REG     0X19
#define MPU_CFG_REG             0X1A
#define MPU_GYRO_CFG_REG        0X1B
#define MPU_ACCEL_CFG_REG       0X1C
#define MPU_MOTION_DET_REG      0X1F
#define MPU_FIFO_EN_REG         0X23
#define MPU_I2CMST_CTRL_REG     0X24
#define MPU_I2CSLV0_ADDR_REG    0X25
#define MPU_I2CSLV0_REG         0X26
#define MPU_I2CSLV0_CTRL_REG    0X27
#define MPU_I2CSLV1_ADDR_REG    0X28
#define MPU_I2CSLV1_REG         0X29
#define MPU_I2CSLV1_CTRL_REG    0X2A
#define MPU_I2CSLV2_ADDR_REG    0X2B
#define MPU_I2CSLV2_REG         0X2C
#define MPU_I2CSLV2_CTRL_REG    0X2D
#define MPU_I2CSLV3_ADDR_REG    0X2E
#define MPU_I2CSLV3_REG         0X2F
#define MPU_I2CSLV3_CTRL_REG    0X30
#define MPU_I2CSLV4_ADDR_REG    0X31
#define MPU_I2CSLV4_REG         0X32
#define MPU_I2CSLV4_DO_REG      0X33
#define MPU_I2CSLV4_CTRL_REG    0X34
#define MPU_I2CSLV4_DI_REG      0X35
#define MPU_I2CMST_STA_REG      0X36
#define MPU_INTBP_CFG_REG       0X37
#define MPU_INT_EN_REG          0X38
#define MPU_INT_STA_REG         0X3A
#define MPU_ACCEL_XOUTH_REG     0X3B
#define MPU_ACCEL_XOUTL_REG     0X3C
#define MPU_ACCEL_YOUTH_REG     0X3D
#define MPU_ACCEL_YOUTL_REG     0X3E
#define MPU_ACCEL_ZOUTH_REG     0X3F
#define MPU_ACCEL_ZOUTL_REG     0X40
#define MPU_TEMP_OUTH_REG       0X41
#define MPU_TEMP_OUTL_REG       0X42
#define MPU_GYRO_XOUTH_REG      0X43
#define MPU_GYRO_XOUTL_REG      0X44
#define MPU_GYRO_YOUTH_REG      0X45
#define MPU_GYRO_YOUTL_REG      0X46
#define MPU_GYRO_ZOUTH_REG      0X47
#define MPU_GYRO_ZOUTL_REG      0X48
#define MPU_PWR_MGMT1_REG       0X6B
#define MPU_PWR_MGMT2_REG       0X6C
#define MPU_FIFO_CNTH_REG       0X72
#define MPU_FIFO_CNTL_REG       0X73
#define MPU_FIFO_RW_REG         0X74
#define MPU_DEVICE_ID_REG       0X75
#define MPU_SIGPATH_RST_REG     0X68
#define MPU_MDETECT_CTRL_REG    0X69
#define MPU_USER_CTRL_REG       0X6A

/* Return codes */
#define ATK_MS6050_EOK      0
#define ATK_MS6050_EID      1
#define ATK_MS6050_EACK     2

uint8_t atk_ms6050_write(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *dat);
uint8_t atk_ms6050_write_byte(uint8_t addr, uint8_t reg, uint8_t dat);
uint8_t atk_ms6050_read(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *dat);
uint8_t atk_ms6050_read_byte(uint8_t addr, uint8_t reg, uint8_t *dat);
void atk_ms6050_sw_reset(void);
uint8_t atk_ms6050_set_gyro_fsr(uint8_t fsr);
uint8_t atk_ms6050_set_accel_fsr(uint8_t fsr);
uint8_t atk_ms6050_set_lpf(uint16_t lpf);
uint8_t atk_ms6050_set_rate(uint16_t rate);
uint8_t atk_ms6050_get_temperature(int16_t *temp);
uint8_t atk_ms6050_get_gyroscope(int16_t *gx, int16_t *gy, int16_t *gz);
uint8_t atk_ms6050_get_accelerometer(int16_t *ax, int16_t *ay, int16_t *az);
uint8_t atk_ms6050_init(void);

#endif
