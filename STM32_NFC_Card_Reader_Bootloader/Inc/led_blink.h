#ifndef LED_BLINK_H_
#define LED_BLINK_H_

#include <stdint.h>
#include <stm32f4xx.h>
#include "systick_driver.h"
#include "gpio_driver.h"

// Bootloader durum led blink'leri.
void Bootloader_Entry_Blink(void);
void Bootloader_Application_Invalid_Blink(void);
void Bootloader_Metadata_Invalid_Blink(void);

#endif /* LED_BLINK_H_ */
