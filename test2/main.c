/**
 * 电机死区 + 极性测试程序
 *
 * 纯 CMSIS 寄存器操作，零 HAL 依赖，单文件，约 2KB。
 *
 * 两个测试：
 *   Test 1 — 极性验证：电机交替正转/反转，确认轮子转动方向与预期一致
 *   Test 2 — 死区扫描：PWM 从 60 递增到 600，找到电机刚好开始转动的最小值
 *
 * 接线（与主项目一致）：
 *   PA8  = 左电机 PWM   (TIM1_CH1)
 *   PA11 = 右电机 PWM   (TIM1_CH4)
 *   PB14 = 左电机 AIN1  (TB6612)
 *   PB15 = 左电机 AIN2  (TB6612)
 *   PB13 = 右电机 BIN1  (TB6612)
 *   PB12 = 右电机 BIN2  (TB6612)
 *   PA4  = LED (状态指示)
 *   PA5  = KEY2 (手动步进，低电平有效)
 *   PA9  = USART1 TX (115200 波特率，调试输出)
 *
 * 使用方法：
 *   1. F7 编译 → F8 烧录 → 按复位键
 *   2. 观察 LED 快闪 3 次确认启动
 *   3. 串口助手 115200 查看输出
 *   4. 按 KEY2(PA5) 在测试间切换
 *   5. 找到电机刚开始稳定转动的最小 PWM 值，作为死区阈值
 */

#include "stm32f103xb.h"

/* ================================================================== */
/* 全局变量                                                           */
/* ================================================================== */
static volatile uint32_t g_ms = 0;
uint32_t SystemCoreClock = 64000000UL;

/* ================================================================== */
/* 纯 NOP 延时 — 不依赖 SysTick，绝对可靠                                */
/* ================================================================== */
static void delay_nop(uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms; i++)
        for (volatile uint32_t j = 0; j < 8000; j++) __NOP();
}

/* ================================================================== */
/* SysTick — 1ms 中断                                                  */
/* ================================================================== */
void SysTick_Handler(void) { g_ms++; }

static void delay_ms(uint32_t ms)
{
    uint32_t start = g_ms;
    while ((g_ms - start) < ms) { __NOP(); }
}

/* ================================================================== */
/* 时钟初始化 — HSI → PLL 64MHz                                        */
/* ================================================================== */
static void clock_init(void)
{
    /* 确保 HSI 开启 */
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));

    /* 复位 CFGR 到默认值（确保干净状态） */
    RCC->CFGR &= ~0xFFFFFFFFUL;  /* 只保留 SW=00 (HSI) */

    /* Flash: 2 wait states (48MHz < SYSCLK ≤ 72MHz) */
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= FLASH_ACR_LATENCY_2;

    /* PLL: HSI/2 × 16 = 4MHz × 16 = 64MHz */
    RCC->CFGR |= RCC_CFGR_PLLMULL16 & 0x003C0000UL;  /* PLLMUL=16, PLLSRC=HSI/2 */
    /* APB1 = HCLK/2, APB2 = HCLK/1 (default = 0) */
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));

    /* 切换到 PLL */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

    SystemCoreClock = 64000000UL;
    SysTick_Config(SystemCoreClock / 1000);
}

/* ================================================================== */
/* JTAG 释放 — 让 PB3/PB4/PA15 变为普通 GPIO（消除 OLED 乱码）           */
/* ================================================================== */
static void jtag_release(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->MAPR = (AFIO->MAPR & ~0x07000000UL) | 0x02000000UL;  /* SWJ_CFG=010: 仅 SWD */
}

/* ================================================================== */
/* USART1 TX — 115200 波特率, PA9                                       */
/* ================================================================== */
static void uart_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

    /* PA9: 50MHz 复用推挽输出 */
    GPIOA->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9);
    GPIOA->CRH |= GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9_1 | GPIO_CRH_MODE9_0;

    /* Reset USART to default state first */
    USART1->CR1 = 0;
    /* BRR: 115200 @ 64MHz → DIV = 64000000/(16×115200) = 34.72
     * Mantissa=34, Fraction=round(0.722×16)=12 → 34.75 */
    USART1->BRR = (34 << 4) | 12;
    /* 8N1, enable TX */
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE;
}

static void uart_putc(char c)
{
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = c;
}

static void uart_print(const char *s)
{
    while (*s) uart_putc(*s++);
}

/* 打印整数 */
static void uart_print_int(int32_t n)
{
    char buf[12];
    int i = 0;
    if (n < 0) { uart_putc('-'); n = -n; }
    if (n == 0) { uart_putc('0'); return; }
    while (n) { buf[i++] = (char)('0' + (n % 10)); n /= 10; }
    while (i--) uart_putc(buf[i]);
}

/* ================================================================== */
/* LED (PA4)                                                          */
/* ================================================================== */
#define LED_ON()   (GPIOA->BSRR = GPIO_BSRR_BS4)
#define LED_OFF()  (GPIOA->BRR  = GPIO_BRR_BR4)

static void led_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRL &= ~(GPIO_CRL_CNF4 | GPIO_CRL_MODE4);
    GPIOA->CRL |= GPIO_CRL_MODE4_1 | GPIO_CRL_MODE4_0;   /* 50MHz 推挽输出 */
    LED_OFF();
}

/* LED 快闪 count 次（确认运行） */
static void led_flash(int count)
{
    for (int i = 0; i < count; i++) {
        LED_ON();  delay_ms(100);
        LED_OFF(); delay_ms(100);
    }
}

/* ================================================================== */
/* KEY2 (PA5) — 低电平有效，内部上拉                                     */
/* ================================================================== */
static void key_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRL &= ~(GPIO_CRL_CNF5 | GPIO_CRL_MODE5);
    GPIOA->CRL |= GPIO_CRL_CNF5_1;         /* 输入 + 上拉/下拉 */
    GPIOA->ODR |= GPIO_ODR_ODR5;           /* 内部上拉 */
}

static uint8_t key2_is_pressed(void)
{
    return !(GPIOA->IDR & GPIO_IDR_IDR5);
}

/* 等待按键按下并释放（含 30ms 去抖） */
static void key2_wait_press(void)
{
    /* 等待按下 */
    while (!key2_is_pressed());
    delay_ms(30);
    while (key2_is_pressed());  /* 等待释放 */
    delay_ms(30);
}

/* ================================================================== */
/* 电机控制（TIM1 PWM + 方向引脚）                                      */
/* ================================================================== */

/* 方向引脚: PB12=BIN2, PB13=BIN1, PB14=AIN1, PB15=AIN2 */
#define AIN1_HIGH()  (GPIOB->BSRR = GPIO_BSRR_BS14)
#define AIN1_LOW()   (GPIOB->BRR  = GPIO_BRR_BR14)
#define AIN2_HIGH()  (GPIOB->BSRR = GPIO_BSRR_BS15)
#define AIN2_LOW()   (GPIOB->BRR  = GPIO_BRR_BR15)
#define BIN1_HIGH()  (GPIOB->BSRR = GPIO_BSRR_BS13)
#define BIN1_LOW()   (GPIOB->BRR  = GPIO_BRR_BR13)
#define BIN2_HIGH()  (GPIOB->BSRR = GPIO_BSRR_BS12)
#define BIN2_LOW()   (GPIOB->BRR  = GPIO_BRR_BR12)

/* PWM 周期 = 6399 → 64MHz / 6400 = 10KHz */
#define PWM_PERIOD  6399
#define PWM_MAX     6399

static void motor_init(void)
{
    /* 时钟 */
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN | RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;

    /* PA8(TIM1_CH1), PA11(TIM1_CH4): 50MHz 复用推挽输出 */
    GPIOA->CRH &= ~((GPIO_CRH_CNF8 | GPIO_CRH_MODE8)
                  | (GPIO_CRH_CNF11 | GPIO_CRH_MODE11));
    GPIOA->CRH |= (GPIO_CRH_CNF8_1 | GPIO_CRH_MODE8_1 | GPIO_CRH_MODE8_0)
                | (GPIO_CRH_CNF11_1 | GPIO_CRH_MODE11_1 | GPIO_CRH_MODE11_0);

    /* PB12-15: 50MHz 推挽输出（TB6612 方向控制） */
    GPIOB->CRH &= ~((GPIO_CRH_CNF12 | GPIO_CRH_MODE12)
                  | (GPIO_CRH_CNF13 | GPIO_CRH_MODE13)
                  | (GPIO_CRH_CNF14 | GPIO_CRH_MODE14)
                  | (GPIO_CRH_CNF15 | GPIO_CRH_MODE15));
    GPIOB->CRH |= (GPIO_CRH_MODE12_1 | GPIO_CRH_MODE12_0)
                | (GPIO_CRH_MODE13_1 | GPIO_CRH_MODE13_0)
                | (GPIO_CRH_MODE14_1 | GPIO_CRH_MODE14_0)
                | (GPIO_CRH_MODE15_1 | GPIO_CRH_MODE15_0);

    /* 方向引脚初始低电平 */
    AIN1_LOW(); AIN2_LOW();
    BIN1_LOW(); BIN2_LOW();

    /* TIM1: 边沿对齐, 向上计数, PWM 模式 1 */
    TIM1->ARR = PWM_PERIOD;
    TIM1->PSC = 0;
    TIM1->CR1 = TIM_CR1_ARPE;             /* 自动重载预装载使能 */
    TIM1->BDTR = TIM_BDTR_MOE;            /* 主输出使能 */

    /* CH1 (PA8) PWM 模式 1, 高电平有效 */
    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM1->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;  /* PWM mode 1 */
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE;       /* 预装载使能 */
    TIM1->CCER |= TIM_CCER_CC1E;          /* CH1 输出使能 */

    /* CH4 (PA11) PWM 模式 1, 高电平有效 */
    TIM1->CCMR2 &= ~TIM_CCMR2_OC4M;
    TIM1->CCMR2 |= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1;  /* PWM mode 1 */
    TIM1->CCMR2 |= TIM_CCMR2_OC4PE;       /* 预装载使能 */
    TIM1->CCER |= TIM_CCER_CC4E;          /* CH4 输出使能 */

    TIM1->CCR1 = 0;
    TIM1->CCR4 = 0;

    TIM1->CR1 |= TIM_CR1_CEN;             /* 使能计数器 */
}

/* 设置左右电机 PWM（正值=正转, 负值=反转, 自动限幅） */
static void motor_set(int left, int right)
{
    /* 左电机 */
    if (left >= 0) {
        if (left > PWM_MAX) left = PWM_MAX;
        AIN1_HIGH(); AIN2_LOW();
        TIM1->CCR1 = (uint16_t)left;
    } else {
        if (left < -PWM_MAX) left = -PWM_MAX;
        AIN1_LOW(); AIN2_HIGH();
        TIM1->CCR1 = (uint16_t)(-left);
    }

    /* 右电机 */
    if (right >= 0) {
        if (right > PWM_MAX) right = PWM_MAX;
        BIN1_HIGH(); BIN2_LOW();
        TIM1->CCR4 = (uint16_t)right;
    } else {
        if (right < -PWM_MAX) right = -PWM_MAX;
        BIN1_LOW(); BIN2_HIGH();
        TIM1->CCR4 = (uint16_t)(-right);
    }
}

static void motor_stop(void)
{
    AIN1_LOW(); AIN2_LOW();
    BIN1_LOW(); BIN2_LOW();
    TIM1->CCR1 = 0;
    TIM1->CCR4 = 0;
}

/* ================================================================== */
/* SystemInit — 启动时由 startup 调用，在 main() 之前                      */
/* ================================================================== */
void SystemInit(void)
{
    /* 默认 HSI 8MHz，main() 中会调用 clock_init() 重新配置 */
}

/* ================================================================== */
/* 主程序                                                             */
/* ================================================================== */
int main(void)
{
    /*
     * 阶段 0: 上电探测（纯 NOP 延时，零时钟依赖）
     * 如果 LED 不闪 → 芯片未运行（烧录/供电/BOOT0 问题）
     * 如果 LED 闪了 → 芯片 OK
     */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    GPIOA->CRL &= ~(GPIO_CRL_CNF4 | GPIO_CRL_MODE4);
    GPIOA->CRL |= GPIO_CRL_MODE4_1 | GPIO_CRL_MODE4_0;
    GPIOA->BSRR = GPIO_BSRR_BR4;  /* LED off */

    /* 快速闪 3 次 — 证明芯片跑起来了 */
    for (int i = 0; i < 3; i++) {
        GPIOA->BRR  = GPIO_BRR_BR4;  delay_nop(80);  /* LED on */
        GPIOA->BSRR = GPIO_BSRR_BS4; delay_nop(80);  /* LED off */
    }

    /* 阶段 1: 时钟 + 外设初始化 */
    jtag_release();         /* 释放 JTAG 引脚，消除 OLED SPI 干扰 */
    clock_init();           /* HSI → PLL 64MHz */
    led_init();
    key_init();
    uart_init();
    motor_init();

    uart_print("\r\n========================================\r\n");
    uart_print("  Motor Dead Zone & Polarity Test\r\n");
    uart_print("  SYSCLK = 64MHz, PWM = 10KHz\r\n");
    uart_print("========================================\r\n\r\n");

    /* 启动确认：LED 快闪 3 次 */
    led_flash(3);
    delay_ms(500);

    /* ==================================================================
     * Test 1: 极性验证
     *
     * 电机交替正转/反转，观察轮子是否按预期方向转动。
     * - 正转（+）: AIN1=H, AIN2=L → 轮子应向前转
     * - 反转（-）: AIN1=L, AIN2=H → 轮子应向后转
     *
     * 如果方向反了，在 control.c 的 balance_pwm 前加减负号。
     * ================================================================== */
    uart_print("[Test 1] Polarity Check\r\n");
    uart_print("  PWM+ : wheel should turn FORWARD\r\n");
    uart_print("  PWM- : wheel should turn BACKWARD\r\n");
    uart_print("  Press KEY2 to start...\r\n\r\n");

    key2_wait_press();

    for (int round = 1; round <= 3; round++) {
        uart_print("--- Round "); uart_print_int(round);
        uart_print(" / 3 ---\r\n");

        /* 正转：PWM=500, 2 秒 */
        uart_print("  FORWARD  (PWM=+500) ... ");
        LED_ON();
        motor_set(500, 500);
        delay_ms(2000);
        LED_OFF();

        /* 刹车 */
        motor_stop();
        uart_print("observe wheel direction, then...\r\n");
        delay_ms(1000);

        /* 反转：PWM=-500, 2 秒 */
        uart_print("  BACKWARD (PWM=-500) ... ");
        LED_ON();
        motor_set(-500, -500);
        delay_ms(2000);
        LED_OFF();

        /* 刹车 */
        motor_stop();
        uart_print("observe wheel direction.\r\n\r\n");
        delay_ms(1000);
    }

    uart_print("[Test 1] DONE.\r\n");
    uart_print("  If FORWARD turns wheel FORWARD:  polarity OK.\r\n");
    uart_print("  If FORWARD turns wheel BACKWARD: polarity REVERSED!\r\n\r\n");

    /* ==================================================================
     * Test 2: 死区扫描
     *
     * PWM 从 LOW 递增到 HIGH，每个值运行 2 秒。
     * 观察串口输出：找到电机"刚好开始稳定转动"的最小 PWM 值。
     *
     * 这个值就是 MOTOR_DEAD_ZONE（约 200-400）。
     * 需按比例换算到主项目的 72MHz/7200 PWM 范围：
     *   主项目死区 ≈ 测试值 × (7200/6400) ≈ 测试值 × 1.125
     * ================================================================== */
    uart_print("[Test 2] Dead Zone Scan (fine: 280~320, step=5)\r\n");
    uart_print("  PWM scans 280-320 in small steps.\r\n");
    uart_print("  Press KEY2 to start...\r\n\r\n");

    key2_wait_press();

    /* 精确扫描：280 ~ 320，步长 5 */
    static const int scan_values[] = {
        280, 285, 290, 295, 300, 305, 310, 315, 320
    };
    const int n_steps = sizeof(scan_values) / sizeof(scan_values[0]);

    for (int i = 0; i < n_steps; i++) {
        int pwm_val = scan_values[i];

        uart_print("  [");
        uart_print_int(i + 1);
        uart_print("/");
        uart_print_int(n_steps);
        uart_print("] PWM=");
        uart_print_int(pwm_val);
        uart_print("  (main equiv ~");
        uart_print_int((pwm_val * 7200 + 3200) / 6400);  /* 换算到 72MHz/7200 */
        uart_print(")  ");

        LED_ON();
        motor_set(pwm_val, pwm_val);
        delay_ms(2000);
        LED_OFF();
        motor_stop();

        uart_print("→ observe & press KEY2 for next\r\n");
        key2_wait_press();
    }

    uart_print("\r\n[Test 2] DONE.\r\n");
    uart_print("  Record the LOWEST PWM where motor turned.\r\n");
    uart_print("  That value (×1.125) is your MOTOR_DEAD_ZONE.\r\n\r\n");

    /* ==================================================================
     * 完成
     * ================================================================== */
    uart_print("========================================\r\n");
    uart_print("  All tests complete!\r\n");
    uart_print("  Press KEY2 to run again.\r\n");
    uart_print("========================================\r\n\r\n");

    while (1) {
        LED_ON();  delay_ms(500);
        LED_OFF(); delay_ms(500);

        if (key2_is_pressed()) {
            delay_ms(30);  /* 去抖 */
            if (key2_is_pressed()) {
                while (key2_is_pressed());  /* 等待释放 */
                delay_ms(30);
                /* 软件复位，重新运行 */
                NVIC_SystemReset();
            }
        }
    }
}
