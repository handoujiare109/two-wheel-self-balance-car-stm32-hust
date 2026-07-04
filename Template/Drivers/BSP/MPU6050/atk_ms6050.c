#include "atk_ms6050.h"
#include "atk_ms6050_iic.h"
#include "delay.h"

static void atk_ms6050_hw_init(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};

    ATK_MS6050_AD0_GPIO_CLK_ENABLE();

    gpio_init_struct.Pin    = ATK_MS6050_AD0_GPIO_PIN;
    gpio_init_struct.Mode   = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull   = GPIO_PULLUP;
    gpio_init_struct.Speed  = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ATK_MS6050_AD0_GPIO_PORT, &gpio_init_struct);

    /* AD0 LOW → I2C address = 0x68 */
    ATK_MS6050_AD0(0);
}

uint8_t atk_ms6050_write(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *dat)
{
    uint8_t i;

    atk_ms6050_iic_start();
    atk_ms6050_iic_send_byte((addr << 1) | 0);
    if (atk_ms6050_iic_wait_ack() == 1) { atk_ms6050_iic_stop(); return ATK_MS6050_EACK; }
    atk_ms6050_iic_send_byte(reg);
    if (atk_ms6050_iic_wait_ack() == 1) { atk_ms6050_iic_stop(); return ATK_MS6050_EACK; }
    for (i = 0; i < len; i++)
    {
        atk_ms6050_iic_send_byte(dat[i]);
        if (atk_ms6050_iic_wait_ack() == 1) { atk_ms6050_iic_stop(); return ATK_MS6050_EACK; }
    }
    atk_ms6050_iic_stop();
    return ATK_MS6050_EOK;
}

uint8_t atk_ms6050_write_byte(uint8_t addr, uint8_t reg, uint8_t dat)
{
    return atk_ms6050_write(addr, reg, 1, &dat);
}

uint8_t atk_ms6050_read(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *dat)
{
    atk_ms6050_iic_start();
    atk_ms6050_iic_send_byte((addr << 1) | 0);
    if (atk_ms6050_iic_wait_ack() == 1) { atk_ms6050_iic_stop(); return ATK_MS6050_EACK; }
    atk_ms6050_iic_send_byte(reg);
    if (atk_ms6050_iic_wait_ack() == 1) { atk_ms6050_iic_stop(); return ATK_MS6050_EACK; }
    atk_ms6050_iic_start();
    atk_ms6050_iic_send_byte((addr << 1) | 1);
    if (atk_ms6050_iic_wait_ack() == 1) { atk_ms6050_iic_stop(); return ATK_MS6050_EACK; }
    while (len)
    {
        *dat = atk_ms6050_iic_read_byte((len > 1) ? 1 : 0);
        len--;
        dat++;
    }
    atk_ms6050_iic_stop();
    return ATK_MS6050_EOK;
}

uint8_t atk_ms6050_read_byte(uint8_t addr, uint8_t reg, uint8_t *dat)
{
    return atk_ms6050_read(addr, reg, 1, dat);
}

void atk_ms6050_sw_reset(void)
{
    atk_ms6050_write_byte(ATK_MS6050_IIC_ADDR, MPU_PWR_MGMT1_REG, 0x80);
    delay_ms(100);
    atk_ms6050_write_byte(ATK_MS6050_IIC_ADDR, MPU_PWR_MGMT1_REG, 0x00);
}

uint8_t atk_ms6050_set_gyro_fsr(uint8_t fsr)
{
    return atk_ms6050_write_byte(ATK_MS6050_IIC_ADDR, MPU_GYRO_CFG_REG, fsr << 3);
}

uint8_t atk_ms6050_set_accel_fsr(uint8_t fsr)
{
    return atk_ms6050_write_byte(ATK_MS6050_IIC_ADDR, MPU_ACCEL_CFG_REG, fsr << 3);
}

uint8_t atk_ms6050_set_lpf(uint16_t lpf)
{
    uint8_t dat;
    if (lpf >= 188)       dat = 1;
    else if (lpf >= 98)   dat = 2;
    else if (lpf >= 42)   dat = 3;
    else if (lpf >= 20)   dat = 4;
    else if (lpf >= 10)   dat = 5;
    else                  dat = 6;
    return atk_ms6050_write_byte(ATK_MS6050_IIC_ADDR, MPU_CFG_REG, dat);
}

uint8_t atk_ms6050_set_rate(uint16_t rate)
{
    uint8_t ret, dat;
    if (rate > 1000) rate = 1000;
    if (rate < 4)    rate = 4;
    dat = 1000 / rate - 1;
    ret = atk_ms6050_write_byte(ATK_MS6050_IIC_ADDR, MPU_SAMPLE_RATE_REG, dat);
    if (ret != ATK_MS6050_EOK) return ret;
    ret = atk_ms6050_set_lpf(rate >> 1);
    return ret;
}

uint8_t atk_ms6050_get_temperature(int16_t *temp)
{
    uint8_t dat[2], ret;
    int16_t raw = 0;
    ret = atk_ms6050_read(ATK_MS6050_IIC_ADDR, MPU_TEMP_OUTH_REG, 2, dat);
    if (ret == ATK_MS6050_EOK)
    {
        raw = ((uint16_t)dat[0] << 8) | dat[1];
        *temp = (int16_t)((36.53f + ((float)raw / 340)) * 100);
    }
    return ret;
}

uint8_t atk_ms6050_get_gyroscope(int16_t *gx, int16_t *gy, int16_t *gz)
{
    uint8_t dat[6], ret;
    ret = atk_ms6050_read(ATK_MS6050_IIC_ADDR, MPU_GYRO_XOUTH_REG, 6, dat);
    if (ret == ATK_MS6050_EOK)
    {
        *gx = ((uint16_t)dat[0] << 8) | dat[1];
        *gy = ((uint16_t)dat[2] << 8) | dat[3];
        *gz = ((uint16_t)dat[4] << 8) | dat[5];
    }
    return ret;
}

uint8_t atk_ms6050_get_accelerometer(int16_t *ax, int16_t *ay, int16_t *az)
{
    uint8_t dat[6], ret;
    ret = atk_ms6050_read(ATK_MS6050_IIC_ADDR, MPU_ACCEL_XOUTH_REG, 6, dat);
    if (ret == ATK_MS6050_EOK)
    {
        *ax = ((uint16_t)dat[0] << 8) | dat[1];
        *ay = ((uint16_t)dat[2] << 8) | dat[3];
        *az = ((uint16_t)dat[4] << 8) | dat[5];
    }
    return ret;
}

uint8_t atk_ms6050_init(void)
{
    uint8_t id;

    atk_ms6050_hw_init();
    atk_ms6050_iic_init();
    atk_ms6050_sw_reset();
    atk_ms6050_set_gyro_fsr(3);      /* Gyro: +/-2000dps */
    atk_ms6050_set_accel_fsr(0);     /* Accel: +/-2g */
    atk_ms6050_set_rate(200);        /* Sample rate: 200Hz = 5ms (matches TIM4) */
    atk_ms6050_write_byte(ATK_MS6050_IIC_ADDR, MPU_INT_EN_REG, 0X01);  /* Enable Data Ready interrupt */
    atk_ms6050_write_byte(ATK_MS6050_IIC_ADDR, MPU_USER_CTRL_REG, 0X00);
    atk_ms6050_write_byte(ATK_MS6050_IIC_ADDR, MPU_FIFO_EN_REG, 0X00);
    atk_ms6050_write_byte(ATK_MS6050_IIC_ADDR, MPU_INTBP_CFG_REG, 0X80);  /* INT pin active low */
    atk_ms6050_read_byte(ATK_MS6050_IIC_ADDR, MPU_DEVICE_ID_REG, &id);
    if (id != ATK_MS6050_IIC_ADDR) return ATK_MS6050_EID;
    atk_ms6050_write_byte(ATK_MS6050_IIC_ADDR, MPU_PWR_MGMT1_REG, 0x01);  /* PLL X axis */
    atk_ms6050_write_byte(ATK_MS6050_IIC_ADDR, MPU_PWR_MGMT2_REG, 0x00);  /* Accel & Gyro on */
    atk_ms6050_set_rate(200);        /* Re-apply after clock switch (200Hz) */

    return ATK_MS6050_EOK;
}
