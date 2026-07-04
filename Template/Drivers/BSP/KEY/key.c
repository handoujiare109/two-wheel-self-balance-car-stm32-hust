#include "key.h"

uint8_t key_scan(void)
{
    static uint8_t key_up = 1;
    if (key_up && HAL_GPIO_ReadPin(KEY_PORT, KEY_PIN) == GPIO_PIN_RESET) {
        HAL_Delay(10);
        key_up = 0;
        if (HAL_GPIO_ReadPin(KEY_PORT, KEY_PIN) == GPIO_PIN_RESET)
            return 1;
    } else if (HAL_GPIO_ReadPin(KEY_PORT, KEY_PIN) == GPIO_PIN_SET) {
        key_up = 1;
    }
    return 0;
}
