#ifndef __OLED_H
#define __OLED_H

#include "sys.h"

/* OLED mode: 0 = 4-wire SPI, 1 = 8080 parallel */
#define OLED_MODE       0

/* OLED SPI pin assignments:
 *   PB3=RST, PA7=CS, PA15=RS(DC), PB5=SCLK, PB4=SDIN
 *   GPIO config is register-level in oled.c, no HAL dependency.
 */

/* Pin control macros — 寄存器级 BSRR/BRR */
#define OLED_RST(x)     do{ x ? (GPIOB->BSRR = GPIO_BSRR_BS3) : (GPIOB->BRR = GPIO_BRR_BR3); }while(0)
#define OLED_CS(x)      do{ x ? (GPIOA->BSRR = GPIO_BSRR_BS7) : (GPIOA->BRR = GPIO_BRR_BR7); }while(0)
#define OLED_RS(x)      do{ x ? (GPIOA->BSRR = GPIO_BSRR_BS15): (GPIOA->BRR = GPIO_BRR_BR15); }while(0)
#define OLED_SCLK(x)    do{ x ? (GPIOB->BSRR = GPIO_BSRR_BS5) : (GPIOB->BRR = GPIO_BRR_BR5); }while(0)
#define OLED_SDIN(x)    do{ x ? (GPIOB->BSRR = GPIO_BSRR_BS4) : (GPIOB->BRR = GPIO_BRR_BR4); }while(0)

#define OLED_CMD        0
#define OLED_DATA       1

void oled_init(void);
void oled_clear(void);
void oled_display_on(void);
void oled_display_off(void);
void oled_refresh_gram(void);
void oled_draw_point(uint8_t x, uint8_t y, uint8_t dot);
void oled_fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dot);
void oled_show_char(uint8_t x, uint8_t y, uint8_t chr, uint8_t size, uint8_t mode);
void oled_show_num(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);
void oled_show_string(uint8_t x, uint8_t y, const char *p, uint8_t size);

#endif
