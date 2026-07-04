#include "adc.h"

/* 极简寄存器版 ADC（替代 HAL_ADC，节省 ~2.7KB 代码） */

void adc_init(void)
{
    /* 1. 启用 ADC1 和 GPIOB 时钟 */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN | RCC_APB2ENR_IOPBEN;

    /* 2. PB0 设为模拟输入 (CNF=00, MODE=00) */
    GPIOB->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0);

    /* 3. 复位并配置 ADC */
    RCC->APB2RSTR |= RCC_APB2RSTR_ADC1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_ADC1RST;

    /* 4. 设置 ADC 时钟 (PCLK2/6 = 72MHz/6 = 12MHz) */
    RCC->CFGR |= RCC_CFGR_ADCPRE_1;  /* 10 = /6 */

    /* 5. 单通道，软件触发，右对齐 */
    ADC1->CR1 = 0;                   /* 无中断，无扫描 */
    ADC1->CR2 = ADC_CR2_ADON         /* 启用 ADC */
              | ADC_CR2_EXTSEL       /* SWSTART 触发 */
              | ADC_CR2_EXTTRIG;     /* 允许外部触发 */
    ADC1->SQR1 = 0;                  /* 1个转换 (L=0000) */

    /* 6. 通道 8，采样时间 55.5 周期 */
    ADC1->SQR3 = 8;                  /* SQ1 = channel 8 */
    ADC1->SMPR2 = ADC_SMPR2_SMP8_1 | ADC_SMPR2_SMP8_0;  /* 011 = 28.5 周期 */

    /* 7. 复位校准 */
    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while (ADC1->CR2 & ADC_CR2_RSTCAL);

    /* 8. 校准 */
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL);
}

float Get_battery_volt(void)
{
    uint32_t adc_val;

    /* 启动转换 */
    ADC1->CR2 |= ADC_CR2_SWSTART;

    /* 等待转换完成 */
    while (!(ADC1->SR & ADC_SR_EOC));

    /* 读取结果 */
    adc_val = ADC1->DR;

    /* 计算电压 */
    float volt = (float)adc_val * 3.3f * 43.0f / 4096.0f / 10.0f;
    return volt;
}
