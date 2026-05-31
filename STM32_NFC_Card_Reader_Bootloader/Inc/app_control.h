#ifndef APP_CONTROL_H_
#define APP_CONTROL_H_

#include <stm32f4xx.h>
#include "boot_config.h"
#include "bootloader_driver.h"
// Ana program kontrol durumları.
typedef enum
{
	Application_Invalid = 0,  // Ana program yok veya geçersiz.
	Application_Valid,        // Ana program var veya geçerli.
}Application_Status_Typedef;


// Bootloader harici ana program varmı kontorlü
Application_Status_Typedef Application_Control(void);

// Bootloader'den ana uygulamaya geçiş fonksiyonu.
void Jump_To_Application();

#endif /* APP_CONTROL_H_ */
