#include "atk_ms6050_iic.h"
#include "delay.h"

static inline void atk_ms6050_iic_delay(void)
{
    delay_us(1);
}

void atk_ms6050_iic_start(void)
{
    ATK_MS6050_IIC_SDA(1);
    ATK_MS6050_IIC_SCL(1);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SDA(0);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SCL(0);
    atk_ms6050_iic_delay();
}

void atk_ms6050_iic_stop(void)
{
    ATK_MS6050_IIC_SDA(0);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SCL(1);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SDA(1);
    atk_ms6050_iic_delay();
}

uint8_t atk_ms6050_iic_wait_ack(void)
{
    uint8_t waittime = 0;
    uint8_t rack = 0;

    ATK_MS6050_IIC_SDA(1);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SCL(1);
    atk_ms6050_iic_delay();

    while (ATK_MS6050_IIC_READ_SDA())
    {
        waittime++;
        if (waittime > 250)
        {
            atk_ms6050_iic_stop();
            rack = 1;
            break;
        }
    }

    ATK_MS6050_IIC_SCL(0);
    atk_ms6050_iic_delay();
    return rack;
}

void atk_ms6050_iic_ack(void)
{
    ATK_MS6050_IIC_SDA(0);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SCL(1);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SCL(0);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SDA(1);
    atk_ms6050_iic_delay();
}

void atk_ms6050_iic_nack(void)
{
    ATK_MS6050_IIC_SDA(1);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SCL(1);
    atk_ms6050_iic_delay();
    ATK_MS6050_IIC_SCL(0);
    atk_ms6050_iic_delay();
}

void atk_ms6050_iic_send_byte(uint8_t dat)
{
    uint8_t t;
    for (t = 0; t < 8; t++)
    {
        ATK_MS6050_IIC_SDA((dat & 0x80) >> 7);
        atk_ms6050_iic_delay();
        ATK_MS6050_IIC_SCL(1);
        atk_ms6050_iic_delay();
        ATK_MS6050_IIC_SCL(0);
        dat <<= 1;
    }
    ATK_MS6050_IIC_SDA(1);
}

uint8_t atk_ms6050_iic_read_byte(uint8_t ack)
{
    uint8_t i;
    uint8_t dat = 0;

    for (i = 0; i < 8; i++)
    {
        dat <<= 1;
        ATK_MS6050_IIC_SCL(1);
        atk_ms6050_iic_delay();
        if (ATK_MS6050_IIC_READ_SDA()) dat++;
        ATK_MS6050_IIC_SCL(0);
        atk_ms6050_iic_delay();
    }

    if (ack == 0)
        atk_ms6050_iic_nack();
    else
        atk_ms6050_iic_ack();

    return dat;
}

void atk_ms6050_iic_init(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};

    ATK_MS6050_IIC_SCL_GPIO_CLK_ENABLE();
    ATK_MS6050_IIC_SDA_GPIO_CLK_ENABLE();

    gpio_init_struct.Pin    = ATK_MS6050_IIC_SCL_GPIO_PIN;
    gpio_init_struct.Mode   = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull   = GPIO_PULLUP;
    gpio_init_struct.Speed  = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ATK_MS6050_IIC_SCL_GPIO_PORT, &gpio_init_struct);

    gpio_init_struct.Pin    = ATK_MS6050_IIC_SDA_GPIO_PIN;
    gpio_init_struct.Mode   = GPIO_MODE_OUTPUT_OD;
    HAL_GPIO_Init(ATK_MS6050_IIC_SDA_GPIO_PORT, &gpio_init_struct);

    atk_ms6050_iic_stop();
}
