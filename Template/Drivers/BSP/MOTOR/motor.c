#include "motor.h"

TIM_HandleTypeDef htim1;

int motor_left = 0;
int motor_right = 0;
uint8_t motor_enable = 0;

/*
 * 左右电机死区（需用 test2 分别实测）
 *
 * 两个电机的物理特性不同，最小启动 PWM 可能差 50-100。
 * 用 test2 分别测左右电机的死区值，填入此处。
 * 暂时设为相同值，实测后修正。
 */
#define MOTOR_LEFT_DZ       100
#define MOTOR_RIGHT_DZ      100
#define MOTOR_DZ_START      5
#define MOTOR_SAFE_LIMIT    7199

/*
 * 左右电机增益校准
 *
 * 如果两个电机在相同 PWM 下转速明显不同，调整此系数。
 * 1.0 = 不调整，>1.0 = 增强，<1.0 = 减弱。
 */
#define MOTOR_LEFT_GAIN   1.00f
#define MOTOR_RIGHT_GAIN  1.00f

/* 单路死区补偿 — 小信号不补偿，中等以上信号做柔和偏移 */
static void motor_deadzone(int *pwm, int dz)
{
    if (*pwm > MOTOR_DZ_START)
        *pwm += dz;
    else if (*pwm < -MOTOR_DZ_START)
        *pwm -= dz;
    else
        *pwm = 0;
}

void motor_init(void)
{
    __HAL_RCC_TIM1_CLK_ENABLE();

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 7199;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(&htim1);

    TIM_OC_InitTypeDef timx_oc_pwm_chy = {0};
    timx_oc_pwm_chy.OCMode = TIM_OCMODE_PWM1;
    timx_oc_pwm_chy.Pulse = 0;
    timx_oc_pwm_chy.OCPolarity = TIM_OCPOLARITY_HIGH;
    HAL_TIM_PWM_ConfigChannel(&htim1, &timx_oc_pwm_chy, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim1, &timx_oc_pwm_chy, TIM_CHANNEL_4);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

    HAL_TIM_PWM_MspInit(&htim1);
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        /* PWM 引脚: PA8(TIM1_CH1), PA11(TIM1_CH4) */
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitStruct.Pin = PWMA_Pin | PWMB_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* TB6612 方向控制引脚: PB12(BIN2), PB13(BIN1), PB14(AIN1), PB15(AIN2) */
        __HAL_RCC_GPIOB_CLK_ENABLE();
        GPIO_InitStruct.Pin = AIN1_Pin | AIN2_Pin | BIN1_Pin | BIN2_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

void PWM_Limit(int *pwm)
{
    if (*pwm > PWM_MAX)
        *pwm = PWM_MAX;
    else if (*pwm < PWM_MIN)
        *pwm = PWM_MIN;
}

void motorMove(int speedL, int speedR)
{
    motor_left = speedL;
    motor_right = speedR;

    /* 限幅 */
    PWM_Limit(&motor_left);
    PWM_Limit(&motor_right);

    /* 分电机增益校准 */
    motor_left  = (int)(motor_left  * MOTOR_LEFT_GAIN);
    motor_right = (int)(motor_right * MOTOR_RIGHT_GAIN);

    /* 分电机死区补偿 — 解决两轮不同步的核心 */
    motor_deadzone(&motor_left,  MOTOR_LEFT_DZ);
    motor_deadzone(&motor_right, MOTOR_RIGHT_DZ);

    if (motor_left > MOTOR_SAFE_LIMIT)
        motor_left = MOTOR_SAFE_LIMIT;
    else if (motor_left < -MOTOR_SAFE_LIMIT)
        motor_left = -MOTOR_SAFE_LIMIT;
    if (motor_right > MOTOR_SAFE_LIMIT)
        motor_right = MOTOR_SAFE_LIMIT;
    else if (motor_right < -MOTOR_SAFE_LIMIT)
        motor_right = -MOTOR_SAFE_LIMIT;

    if (motor_left >= 0) {
        AIN1(1); AIN2(0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, motor_left);
    } else {
        AIN1(0); AIN2(1);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, -motor_left);
    }

    if (motor_right >= 0) {
        BIN1(1); BIN2(0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, motor_right);
    } else {
        BIN1(0); BIN2(1);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, -motor_right);
    }
}

void turn_Off(void)
{
    motor_left = 0;
    motor_right = 0;
    AIN1(0); AIN2(0);
    BIN1(0); BIN2(0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
}
