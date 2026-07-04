

#include "./BSP/MOTOR/motor.h"


TIM_HandleTypeDef g_timx_pwm_chy_handle={0};                       /* 定时器x句柄 */
void motorInit()
{
	    
	    __HAL_RCC_TIM1_CLK_ENABLE();                                        /*开启TIM1时钟*/
				
			g_timx_pwm_chy_handle.Instance = TIM1;                             /* 指定使用定时器TIM1 */
			g_timx_pwm_chy_handle.Init.Prescaler = 0;                         /* 定时器不分频 */
			g_timx_pwm_chy_handle.Init.CounterMode = TIM_COUNTERMODE_UP;        /* 递增计数模式 */
			g_timx_pwm_chy_handle.Init.Period = 7199;                            /* 自动重装载值7199 */
			HAL_TIM_PWM_Init(&g_timx_pwm_chy_handle);                           /* 初始化PWM */
	
	    TIM_OC_InitTypeDef timx_oc_pwm_chy  = {0};                          /* 定时器PWM输出配置 */
			timx_oc_pwm_chy.OCMode = TIM_OCMODE_PWM1;                           /* 模式选择PWM1 */
			timx_oc_pwm_chy.Pulse = 0;                                          /* 设置比较值,此值用来确定占空比 */																												
			timx_oc_pwm_chy.OCPolarity = TIM_OCPOLARITY_HIGH;                    /* 输出比较极性 */
			
			HAL_TIM_PWM_ConfigChannel(&g_timx_pwm_chy_handle, &timx_oc_pwm_chy, TIM_CHANNEL_1); /* 配置TIMx通道y */
			HAL_TIM_PWM_ConfigChannel(&g_timx_pwm_chy_handle, &timx_oc_pwm_chy, TIM_CHANNEL_4); /* 配置TIMx通道y */
			HAL_TIM_PWM_Start(&g_timx_pwm_chy_handle, TIM_CHANNEL_1);       /* 开启对应PWM通道 */
			HAL_TIM_PWM_Start(&g_timx_pwm_chy_handle, TIM_CHANNEL_4);       /* 开启对应PWM通道 */
    	
			HAL_TIM_PWM_MspInit(&g_timx_pwm_chy_handle);
}


void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* timHandle)
{

 
  if(timHandle->Instance==TIM1)
  {
		/* 电机PWM引脚 */
		 GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = PWMA_Pin|PWMB_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		
		
		/* 电机方向引脚 */
		 GPIO_InitTypeDef gpio_init_struct;
		__HAL_RCC_GPIOB_CLK_ENABLE();
    gpio_init_struct.Pin = BIN2_Pin|BIN1_Pin|AIN1_Pin|AIN2_Pin;                  
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;            /* 推挽输出 */
    gpio_init_struct.Pull = GPIO_NOPULL;                   /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;          /* 高速 */
    HAL_GPIO_Init(GPIOB, &gpio_init_struct);                /* 初始化方向引脚 */
		
	}

}





void motorMove(int motor_left,int motor_right)
{

	
			
}

uint8_t Turn_Off(void)
{
		
}





