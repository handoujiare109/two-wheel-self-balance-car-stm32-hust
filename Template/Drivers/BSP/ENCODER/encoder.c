#include "encoder.h"

TIM_HandleTypeDef htim2;             /* TIM2: left encoder */
TIM_HandleTypeDef htim4_encoder;    /* TIM4: right encoder */

int encoder_left = 0;
int encoder_right = 0;

void leftEncoderInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_Encoder_InitTypeDef sEncoderConfig = {0};

    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* TIM2 CH1=PA0, CH2=PA1 */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 65535;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    sEncoderConfig.EncoderMode = TIM_ENCODERMODE_TI12;
    sEncoderConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    sEncoderConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sEncoderConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sEncoderConfig.IC1Filter = 0;
    sEncoderConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sEncoderConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sEncoderConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sEncoderConfig.IC2Filter = 0;
    HAL_TIM_Encoder_Init(&htim2, &sEncoderConfig);

    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
}

void rightEncoderInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_Encoder_InitTypeDef sEncoderConfig = {0};

    __HAL_RCC_TIM4_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* TIM4 CH1=PB6, CH2=PB7 */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    htim4_encoder.Instance = TIM4;
    htim4_encoder.Init.Prescaler = 0;
    htim4_encoder.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4_encoder.Init.Period = 65535;
    htim4_encoder.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4_encoder.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    sEncoderConfig.EncoderMode = TIM_ENCODERMODE_TI12;
    sEncoderConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    sEncoderConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sEncoderConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sEncoderConfig.IC1Filter = 0;
    sEncoderConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sEncoderConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sEncoderConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sEncoderConfig.IC2Filter = 0;
    HAL_TIM_Encoder_Init(&htim4_encoder, &sEncoderConfig);

    HAL_TIM_Encoder_Start(&htim4_encoder, TIM_CHANNEL_ALL);
}

int Read_Encoder(TIM_HandleTypeDef *htim)
{
    int val = (int16_t)__HAL_TIM_GET_COUNTER(htim);
    __HAL_TIM_SET_COUNTER(htim, 0);
    return val;
}

void encoder_read_all(void)
{
    encoder_left = Read_Encoder(&htim2);
    encoder_right = Read_Encoder(&htim4_encoder);
}
