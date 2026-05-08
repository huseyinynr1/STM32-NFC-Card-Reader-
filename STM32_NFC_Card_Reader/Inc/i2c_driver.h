#ifndef I2C_DRIVER_H_
#define I2C_DRIVER_H_


#include <stm32f4xx.h>
#include "timer_driver.h"

// I2C fonksiyon sonuç durumları.
typedef enum{
	I2C_STATUS_OK = 0,
	I2C_STATUS_AF_ERROR,
	I2C_STATUS_BERR_ERROR,
	I2C_STATUS_ARLO_ERROR,
	I2C_STATUS_TIMEOUT
}I2C_Status_t;

// I2C1 için başlangıç konfigürasyon ayarları fonksiyon.
void I2C1_Config(void);

// I2C register'a veri yazma fonksiyonu.
I2C_Status_t I2C1_Write(uint8_t slave_addrr, uint8_t reg_addrr, uint8_t data);

// I2C register'dan veri okuma fonksiyonu.
I2C_Status_t I2C1_Read(uint8_t slave_addrr, uint8_t reg_addrr, uint8_t* data);

// I2C Write ve Read işlemleri sırasında durum bayrakları kontrol fonksiyonları.
I2C_Status_t I2C_Wait_For_BSY_Flag();
I2C_Status_t I2C_Wait_For_SB_Flag();
I2C_Status_t I2C_Wait_For_Addrr_Flag();
I2C_Status_t I2C_Wait_For_TXE_Flag();
I2C_Status_t I2C_Wait_For_RXNE_Flag();
I2C_Status_t I2C_Wait_For_BTF_Flag();
I2C_Status_t I2C_Check_Error_Flag();

#endif /* I2C_DRIVER_H_ */
