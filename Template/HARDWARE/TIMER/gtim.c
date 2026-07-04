#include "gtim.h"
#include "encoder.h"
#include "control.h"
#include "motor.h"

TIM_HandleTypeDef htim3;             /* TIM3: 5ms 定时器 (原 TIM4 已用于右编码器) */

int speed_left = 0;
int speed_right = 0;
int target_speed = 0;

void gtim_init(void)
{
    uint32_t prescaler;

    __HAL_RCC_TIM3_CLK_ENABLE();

    prescaler = (SystemCoreClock / 10000UL) - 1UL;

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = prescaler;        /* TIM3 clock / prescaler = 10KHz */
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 50 - 1;              /* 10KHz / 50 = 200Hz = 5ms */
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&htim3);

    HAL_NVIC_SetPriority(TIM3_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    HAL_TIM_Base_Start_IT(&htim3);
}

int Vel_PI(int vel, int Target)
{
    int error;
    static int last_error = 0;
    static int pwm_out = 0;
    int delta;

    /* 电机关闭时重置积分和上一次误差，防止重启后猛冲 */
    extern uint8_t motor_enable;
    if (!motor_enable) {
        last_error = 0;
        pwm_out = 0;
        return 0;
    }

    error = Target - vel;

    /* 增量式 PI: ΔU = Kp*(e(k)-e(k-1)) + Ki*e(k) */
    delta = (int)(Velocity_Kp * (error - last_error) + Velocity_Ki * error);

    pwm_out += delta;

    /* 抗积分饱和：限幅到 PWM 范围 */
    if (pwm_out > PWM_MAX)
        pwm_out = PWM_MAX;
    else if (pwm_out < PWM_MIN)
        pwm_out = PWM_MIN;

    last_error = error;

    return pwm_out;
}

void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3) {
        /* 每 5ms 读取编码器并计算速度 */
        encoder_read_all();
        speed_left = encoder_left;
        speed_right = encoder_right;
    }
}
