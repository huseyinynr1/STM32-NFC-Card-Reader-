#ifndef SPI_DRIVER_H_
#define SPI_DRIVER_H_

#include <stm32f4xx.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "timer_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// SPI fonksiyon sonuçları durumları.
typedef enum{
	SPI_STATUS_START_VALUE = 0,
	SPI_STATUS_OK,
	SPI_STATUS_OVR_ERR,
	SPI_STATUS_MODF_ERR,
	SPI_STATUS_TIMEOUT
}SPI_Status_t;

// SPI1 için başlangıç konfigürasyon ayarları fonksiyonu
void SPI1_Config(void);

// SPI transfer işlemi fonksiyonu.
SPI_Status_t SPI_Transfer(uint8_t out, uint8_t* value);

// SPI1 için mutex oluşturma.
void SPI1_Mutex_Init(void);

// SP1 için recursive mutex alma fonksiyonu.
bool SPI1_Lock(TickType_t timeout);

// SPI için recursive mutex verme fonksiyonu.
void SPI1_Unlock(void);

#endif /* SPI_DRIVER_H_ */
