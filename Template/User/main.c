/**
 * 平衡车主程序 — 所有 Bug 已修复
 */
#include "main.h"
#include "led.h"
#include "key.h"
#include "exti.h"
#include "motor.h"
#include "encoder.h"
#include "atk_ms6050.h"
#include "adc.h"
#include "oled.h"
#include "usart.h"
#include "delay.h"
#include "gtim.h"
#include "control.h"

int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(9);
    delay_init(SystemCoreClock / 1000000);

    oled_init();
    oled_show_string(0, 0, "Init...", 16);
    oled_refresh_gram();

    usart_init(115200);
    printf("Balance Car Starting...\r\n");

    led_init();
    motor_init();
    leftEncoderInit();
    rightEncoderInit();
    gtim_init();
    adc_init();

    atk_ms6050_init();
    printf("MPU6050 OK.\r\n");

    exti_init();
    extix_key2_init();
    printf("EXTI OK.\r\n");

    printf("DBG,t,p100,g10,sl,sr,ss,bp,sp,fl,fr,ml,mr,en,be\r\n");

    uint32_t last_show = 0;

    while (1)
    {
        if (HAL_GetTick() - last_show >= 50) {
            last_show = HAL_GetTick();

            char disp_buf[16];
            /* y 是像素行号，16×16 字符占 16 像素高 → 每行间隔 16 像素 */
            oled_show_string(0, 0, "U202513436WDZ", 16);

            oled_show_string(0, 16, "Pitch:", 16);
            sprintf(disp_buf, "%.2f", pitch);
            oled_show_string(48, 16, disp_buf, 16);

            oled_show_string(0, 32, "Batt:", 16);
            sprintf(disp_buf, "%.2fV", battery_voltage);
            oled_show_string(48, 32, disp_buf, 16);

            oled_show_string(0, 48, "Motor:", 16);
            oled_show_string(48, 48, motor_enable ? "ON " : "OFF", 16);

            oled_refresh_gram();  /* 关键：推送到屏幕！ */

            printf("D,%lu,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
                   HAL_GetTick(),
                   (int)(pitch * 100.0f),
                   (int)(gyro_y * 10.0f),
                   speed_left,
                   speed_right,
                   speed_left + speed_right,
                   debug_balance_pwm,
                   debug_speed_pwm,
                   debug_final_left,
                   debug_final_right,
                   motor_left,
                   motor_right,
                   motor_enable,
                   balance_enable);
        }
    }
}

void Error_Handler(void) { __disable_irq(); while (1) {} }

void HAL_TIMEx_BreakCallback(TIM_HandleTypeDef *htim) { (void)htim; }
void HAL_TIMEx_CommutCallback(TIM_HandleTypeDef *htim) { (void)htim; }
void TIMEx_DMACommutationCplt(DMA_HandleTypeDef *hdma) { (void)hdma; }
void TIMEx_DMACommutationHalfCplt(DMA_HandleTypeDef *hdma) { (void)hdma; }

#include "stm32f1xx_hal_dma.h"
HAL_StatusTypeDef HAL_DMA_Abort_IT(DMA_HandleTypeDef *hdma) { (void)hdma; return HAL_OK; }
HAL_StatusTypeDef HAL_DMA_Start_IT(DMA_HandleTypeDef *hdma, uint32_t SrcAddress,
                                    uint32_t DstAddress, uint32_t DataLength)
{ (void)hdma;(void)SrcAddress;(void)DstAddress;(void)DataLength; return HAL_OK; }
HAL_StatusTypeDef HAL_DMA_Abort(DMA_HandleTypeDef *hdma) { (void)hdma; return HAL_OK; }
uint32_t HAL_DMA_GetError(DMA_HandleTypeDef *hdma) { (void)hdma; return 0; }

uint32_t HAL_RCCEx_GetPeriphCLKFreq(uint32_t PeriphClk)
{ (void)PeriphClk; return 36000000UL; }

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) { (void)file;(void)line; }
#endif
