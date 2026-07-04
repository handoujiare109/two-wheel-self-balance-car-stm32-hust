#include "exti.h"
#include "atk_ms6050.h"
#include "adc.h"
#include <math.h>

float pitch = 0;
float gyro_y = 0;
float battery_voltage = 12.0f;
uint8_t balance_enable = 1;

static void exti_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* MPU6050 INT pin - PA12 - 5ms external interrupt */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void exti_init(void)
{
    exti_gpio_init();
}

void extix_key2_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

float readVolt(uint8_t cnt)
{
    float sum = 0;

    if (cnt == 0) return 0;

    for (uint8_t i = 0; i < cnt; i++) {
        sum += Get_battery_volt();
    }
    return sum / cnt;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_12) {
        /* --- 5ms 控制循环：由 MPU6050 INT 引脚触发 --- */

        /* 1. 读取 MPU6050 姿态数据（互补滤波） */
        {
            int16_t ax, ay, az, gx, gy, gz;
            static float comp_angle = 0;
            atk_ms6050_get_accelerometer(&ax, &ay, &az);
            atk_ms6050_get_gyroscope(&gx, &gy, &gz);
            /* 尝试用 AY+GX 轴组合（MPU6050 安装方向可能不同） */
            float accel_angle = atan2f((float)ay, (float)az) * 57.29578f;
            gyro_y = (float)gx / 16.4f;
            /* 标准互补滤波 α=0.95 */
            comp_angle = 0.95f * (comp_angle + gyro_y * 0.005f) + 0.05f * accel_angle;
            pitch = comp_angle;
        }

        /* 2. 读取电池电压，每 100 次（500ms）更新一次 */
        static uint8_t volt_cnt = 0;
        volt_cnt++;
        if (volt_cnt >= 100) {
            volt_cnt = 0;
            battery_voltage = readVolt(10);
            /* 暂时禁用低压保护以便调试（ADC 分压比需校准） */
            balance_enable = 1;
            /* 校准后恢复：balance_enable = (battery_voltage >= 10.0f) ? 1 : 0; */
        }

        /* 3. 平衡控制计算（由 HARDWARE/CONTROL 实现） */
        extern void Balance_Control(void);
        Balance_Control();
    }

    if (GPIO_Pin == GPIO_PIN_5) {
        /* KEY2 按键：启停平衡（含 30ms 软件去抖） */
        {
            static uint32_t last_key_time = 0;
            uint32_t now = HAL_GetTick();
            if (now - last_key_time < 30) return;  /* 去抖：忽略 30ms 内的重复触发 */
            last_key_time = now;
        }
        extern uint8_t motor_enable;
        motor_enable = !motor_enable;
        if (motor_enable == 0) {
            extern void turn_Off(void);
            turn_Off();
        }
    }
}

void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
}
