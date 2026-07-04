/**
 * OLED 显示验证程序 — 纯寄存器操作，零 HAL 依赖
 *
 * 接线：PB3=RST, PA7=CS, PA15=RS(DC), PB5=SCK, PB4=MOSI
 *
 * 运行效果：全屏亮(1s) → 全屏灭(1s) → 显示"OK O" → 循环
 * 这明确证明：烧录 / 芯片 / OLED 接线 / 供电 全部正常
 */
#include "stm32f103xb.h"

/* ================================================================== */
/* 纯 NOP 延时（不依赖 SysTick，绝对可靠）                             */
/* ================================================================== */
static void delay_ms(uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms; i++)
        for (volatile uint32_t j = 0; j < 8000; j++) __NOP();
}

/* ================================================================== */
/* OLED 引脚控制（寄存器级）                                          */
/* ================================================================== */
#define OLED_CS_LOW()   (GPIOA->BRR = GPIO_BRR_BR7)
#define OLED_CS_HIGH()  (GPIOA->BSRR = GPIO_BSRR_BS7)
#define OLED_RS_LOW()   (GPIOA->BRR = GPIO_BRR_BR15)
#define OLED_RS_HIGH()  (GPIOA->BSRR = GPIO_BSRR_BS15)
#define OLED_RST_LOW()  (GPIOB->BRR = GPIO_BRR_BR3)
#define OLED_RST_HIGH() (GPIOB->BSRR = GPIO_BSRR_BS3)
#define OLED_SCK_LOW()  (GPIOB->BRR = GPIO_BRR_BR5)
#define OLED_SCK_HIGH() (GPIOB->BSRR = GPIO_BSRR_BS5)
#define OLED_SDI_LOW()  (GPIOB->BRR = GPIO_BRR_BR4)
#define OLED_SDI_HIGH() (GPIOB->BSRR = GPIO_BSRR_BS4)
#define OLED_CMD  0
#define OLED_DATA 1

/* ================================================================== */
/* 'O' 'K' 16x16 ASCII 字模（来自 oledfont.h）                         */
/* ================================================================== */
static const unsigned char font_O[16] = {
    0x07,0xF0,0x08,0x08,0x10,0x04,0x10,0x04,
    0x10,0x04,0x08,0x08,0x07,0xF0,0x00,0x00
};
static const unsigned char font_K[16] = {
    0x10,0x04,0x1F,0xFC,0x11,0x04,0x03,0x80,
    0x14,0x64,0x18,0x1C,0x10,0x04,0x00,0x00
};

/* ================================================================== */
/* OLED 软件 SPI 写字节                                                */
/* ================================================================== */
static void oled_write_byte(uint8_t data, uint8_t cmd)
{
    if (cmd == OLED_CMD) OLED_RS_LOW(); else OLED_RS_HIGH();
    OLED_CS_LOW();
    for (uint8_t i = 0; i < 8; i++) {
        OLED_SCK_LOW();
        if (data & 0x80) OLED_SDI_HIGH(); else OLED_SDI_LOW();
        OLED_SCK_HIGH();
        data <<= 1;
    }
    OLED_CS_HIGH();
    OLED_RS_HIGH();
}

/* ================================================================== */
/* OLED 驱动                                                          */
/* ================================================================== */
static void oled_set_pos(uint8_t page, uint8_t col)
{
    oled_write_byte(0xB0 + page, OLED_CMD);
    oled_write_byte(0x00 + (col & 0x0F), OLED_CMD);
    oled_write_byte(0x10 + (col >> 4), OLED_CMD);
}

static void oled_init(void)
{
    OLED_RST_LOW();  delay_ms(100);  OLED_RST_HIGH();

    oled_write_byte(0xAE, OLED_CMD); oled_write_byte(0xD5, OLED_CMD);
    oled_write_byte(0x80, OLED_CMD); oled_write_byte(0xA8, OLED_CMD);
    oled_write_byte(0x3F, OLED_CMD); oled_write_byte(0xD3, OLED_CMD);
    oled_write_byte(0x00, OLED_CMD); oled_write_byte(0x40, OLED_CMD);
    oled_write_byte(0x8D, OLED_CMD); oled_write_byte(0x14, OLED_CMD);
    oled_write_byte(0x20, OLED_CMD); oled_write_byte(0x02, OLED_CMD);
    oled_write_byte(0xA1, OLED_CMD); oled_write_byte(0xC8, OLED_CMD);
    oled_write_byte(0xDA, OLED_CMD); oled_write_byte(0x12, OLED_CMD);
    oled_write_byte(0x81, OLED_CMD); oled_write_byte(0xEF, OLED_CMD);
    oled_write_byte(0xD9, OLED_CMD); oled_write_byte(0xF1, OLED_CMD);
    oled_write_byte(0xDB, OLED_CMD); oled_write_byte(0x30, OLED_CMD);
    oled_write_byte(0xA4, OLED_CMD); oled_write_byte(0xA6, OLED_CMD);
    oled_write_byte(0xAF, OLED_CMD);
}

static void oled_fill_page(uint8_t page, uint8_t val)
{
    oled_set_pos(page, 0);
    for (uint16_t i = 0; i < 128; i++) oled_write_byte(val, OLED_DATA);
}

static void oled_fill_screen(uint8_t val)
{
    for (uint8_t p = 0; p < 8; p++) oled_fill_page(p, val);
}

static void oled_show_16x16(uint8_t page, uint8_t col, const unsigned char *font)
{
    oled_set_pos(page, col);
    for (uint8_t i = 0; i < 8; i++)  oled_write_byte(font[i], OLED_DATA);
    oled_set_pos(page + 1, col);
    for (uint8_t i = 8; i < 16; i++) oled_write_byte(font[i], OLED_DATA);
}

/* ================================================================== */
/* GPIO 初始化                                                        */
/* ================================================================== */
static void gpio_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

    /* 禁用 JTAG (释放 PA15/PB3)，保留 SWD */
    AFIO->MAPR = (AFIO->MAPR & ~0x07000000UL) | 0x02000000UL;

    /* PA7 (CS): 50MHz 推挽输出 */
    GPIOA->CRL &= ~(GPIO_CRL_CNF7 | GPIO_CRL_MODE7);
    GPIOA->CRL |= GPIO_CRL_MODE7_1 | GPIO_CRL_MODE7_0;

    /* PA15 (RS): 50MHz 推挽输出 */
    GPIOA->CRH &= ~(GPIO_CRH_CNF15 | GPIO_CRH_MODE15);
    GPIOA->CRH |= GPIO_CRH_MODE15_1 | GPIO_CRH_MODE15_0;

    /* PB3(RST) PB4(SDI) PB5(SCK): 50MHz 推挽输出 */
    GPIOB->CRL &= ~((GPIO_CRL_CNF3 | GPIO_CRL_MODE3)
                  | (GPIO_CRL_CNF4 | GPIO_CRL_MODE4)
                  | (GPIO_CRL_CNF5 | GPIO_CRL_MODE5));
    GPIOB->CRL |= (GPIO_CRL_MODE3_1 | GPIO_CRL_MODE3_0)
                | (GPIO_CRL_MODE4_1 | GPIO_CRL_MODE4_0)
                | (GPIO_CRL_MODE5_1 | GPIO_CRL_MODE5_0);

    OLED_CS_HIGH(); OLED_RS_HIGH(); OLED_SCK_HIGH(); OLED_SDI_HIGH(); OLED_RST_HIGH();
}

/* ================================================================== */
void SystemInit(void) { /* HSI 8MHz */ }

/* ================================================================== */
int main(void)
{
    gpio_init();
    oled_init();

    for (;;) {
        oled_fill_screen(0xFF);  delay_ms(1000);  /* 全屏亮 */
        oled_fill_screen(0x00);  delay_ms(1000);  /* 全屏灭 */
        oled_show_16x16(2, 20, font_O);            /* O */
        oled_show_16x16(2, 56, font_K);            /* K */
        oled_show_16x16(2, 92, font_O);            /* O */
        delay_ms(2000);
    }
}