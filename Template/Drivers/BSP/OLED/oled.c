#include "oled.h"
#include "oledfont.h"
#include "delay.h"

static uint8_t g_oled_gram[128][8];

static void oled_wr_byte(uint8_t data, uint8_t cmd)
{
    uint8_t i;
    OLED_RS(cmd);
    OLED_CS(0);

    for (i = 0; i < 8; i++)
    {
        OLED_SCLK(0);
        if (data & 0x80)
            OLED_SDIN(1);
        else
            OLED_SDIN(0);
        OLED_SCLK(1);
        data <<= 1;
    }

    OLED_CS(1);
    OLED_RS(1);
}

void oled_refresh_gram(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        oled_wr_byte(0xb0 + i, OLED_CMD);
        oled_wr_byte(0x00, OLED_CMD);
        oled_wr_byte(0x10, OLED_CMD);
        for (n = 0; n < 128; n++)
            oled_wr_byte(g_oled_gram[n][i], OLED_DATA);
    }
}

void oled_display_on(void)
{
    oled_wr_byte(0X8D, OLED_CMD);
    oled_wr_byte(0X14, OLED_CMD);
    oled_wr_byte(0XAF, OLED_CMD);
}

void oled_display_off(void)
{
    oled_wr_byte(0X8D, OLED_CMD);
    oled_wr_byte(0X10, OLED_CMD);
    oled_wr_byte(0XAE, OLED_CMD);
}

void oled_clear(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
        for (n = 0; n < 128; n++)
            g_oled_gram[n][i] = 0X00;
    oled_refresh_gram();
}

void oled_draw_point(uint8_t x, uint8_t y, uint8_t dot)
{
    uint8_t pos, bx, temp = 0;
    if (x > 127 || y > 63) return;
    pos = y / 8;
    bx = y % 8;
    temp = 1 << bx;
    if (dot)
        g_oled_gram[x][pos] |= temp;
    else
        g_oled_gram[x][pos] &= ~temp;
}

void oled_fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dot)
{
    uint8_t x, y;
    for (x = x1; x <= x2; x++)
        for (y = y1; y <= y2; y++)
            oled_draw_point(x, y, dot);
    oled_refresh_gram();
}

void oled_show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode)
{
    uint8_t temp, t, t1;
    uint8_t y0 = y;
    uint8_t *pfont = 0;
    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
    chr = chr - ' ';

    if (size == 16)
        pfont = (uint8_t *)oled_asc2_1608[chr];
    else
        return;

    for (t = 0; t < csize; t++)
    {
        temp = pfont[t];
        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80) oled_draw_point(x, y, mode);
            else oled_draw_point(x, y, !mode);
            temp <<= 1;
            y++;
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}

static uint32_t oled_pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;
    while (n--) result *= m;
    return result;
}

void oled_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++)
    {
        temp = (num / oled_pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                oled_show_char(x + (size / 2) * t, y, ' ', size, 1);
                continue;
            }
            else enshow = 1;
        }
        oled_show_char(x + (size / 2) * t, y, temp + '0', size, 1);
    }
}

void oled_show_string(uint8_t x, uint8_t y, const char *p, uint8_t size)
{
    while ((*p <= '~') && (*p >= ' '))
    {
        if (x > (128 - (size / 2)))
        {
            x = 0;
            y += size;
        }
        if (y > (64 - size))
        {
            y = x = 0;
            oled_clear();
        }
        oled_show_char(x, y, *p, size, 1);
        x += size / 2;
        p++;
    }
}

void oled_init(void)
{
    /* 寄存器级 GPIO 初始化（替代 HAL_GPIO_Init，确保 JTAG 引脚可用） */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

    /* 禁用 JTAG，释放 PB3(RST) PA15(RS) PB4(SDIN)，保留 SWD */
    AFIO->MAPR = (AFIO->MAPR & ~0x07000000UL) | 0x02000000UL;

    /* PA7(CS): 50MHz 推挽输出 */
    GPIOA->CRL &= ~(GPIO_CRL_CNF7 | GPIO_CRL_MODE7);
    GPIOA->CRL |= GPIO_CRL_MODE7_1 | GPIO_CRL_MODE7_0;

    /* PA15(RS): 50MHz 推挽输出 */
    GPIOA->CRH &= ~(GPIO_CRH_CNF15 | GPIO_CRH_MODE15);
    GPIOA->CRH |= GPIO_CRH_MODE15_1 | GPIO_CRH_MODE15_0;

    /* PB3(RST) PB4(SDIN) PB5(SCK): 50MHz 推挽输出 */
    GPIOB->CRL &= ~((GPIO_CRL_CNF3 | GPIO_CRL_MODE3)
                  | (GPIO_CRL_CNF4 | GPIO_CRL_MODE4)
                  | (GPIO_CRL_CNF5 | GPIO_CRL_MODE5));
    GPIOB->CRL |= (GPIO_CRL_MODE3_1 | GPIO_CRL_MODE3_0)
                | (GPIO_CRL_MODE4_1 | GPIO_CRL_MODE4_0)
                | (GPIO_CRL_MODE5_1 | GPIO_CRL_MODE5_0);

    OLED_SDIN(1);
    OLED_SCLK(1);
    OLED_CS(1);
    OLED_RS(1);

    OLED_RST(0);
    delay_ms(100);
    OLED_RST(1);

    oled_wr_byte(0xAE, OLED_CMD);
    oled_wr_byte(0xD5, OLED_CMD);
    oled_wr_byte(80, OLED_CMD);
    oled_wr_byte(0xA8, OLED_CMD);
    oled_wr_byte(0X3F, OLED_CMD);
    oled_wr_byte(0xD3, OLED_CMD);
    oled_wr_byte(0X00, OLED_CMD);
    oled_wr_byte(0x40, OLED_CMD);
    oled_wr_byte(0x8D, OLED_CMD);
    oled_wr_byte(0x14, OLED_CMD);
    oled_wr_byte(0x20, OLED_CMD);
    oled_wr_byte(0x02, OLED_CMD);
    oled_wr_byte(0xA1, OLED_CMD);
    oled_wr_byte(0xC8, OLED_CMD);
    oled_wr_byte(0xDA, OLED_CMD);
    oled_wr_byte(0x12, OLED_CMD);
    oled_wr_byte(0x81, OLED_CMD);
    oled_wr_byte(0xEF, OLED_CMD);
    oled_wr_byte(0xD9, OLED_CMD);
    oled_wr_byte(0xf1, OLED_CMD);
    oled_wr_byte(0xDB, OLED_CMD);
    oled_wr_byte(0x30, OLED_CMD);
    oled_wr_byte(0xA4, OLED_CMD);
    oled_wr_byte(0xA6, OLED_CMD);
    oled_wr_byte(0xAF, OLED_CMD);

    oled_clear();
}
