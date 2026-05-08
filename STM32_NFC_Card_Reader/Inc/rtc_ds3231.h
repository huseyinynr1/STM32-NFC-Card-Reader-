#ifndef RTC_DS3231_H_
#define RTC_DS3231_H_

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "gpio_driver.h"
#include "i2c_driver.h"
#include "FreeRTOS.h"
#include "task.h"

#define Slave_Addrr 0x68

#define Second_Reg            0x00    // Saniye bilgisi tutan register.
#define Minute_Reg      	  0x01    // Dakika bilgisi tutan register.
#define Hour_Reg        	  0x02    // Saat bilgisi tutan register.
#define Day_Name_Reg          0x03    // Gün ismi bilgisi tutan register.
#define Day_Number_Reg  	  0x04    // Aydaki gün sayısını tutan register.
#define Month_Reg       	  0x05    // Ay bilgisi tutan register.
#define Year_Reg        	  0x06    // Yıl bilgisi tutan register.
#define Control_Register      0x0E    // Kontrol işlemleri yapılan register.
#define Status_Reg            0x0F    // Durumlar bilgisi tutan register.
#define Temperature_Reg_Upper 0x11    // Sıcaklık bilgisi tutan üst register.
#define Temperature_Reg_Lower 0x12    // Sıcaklık bilgisi tutan alt register.

// Register işlemleri ve kod akışında oluşan durum bilgileri.
typedef enum
{
	RTC_DS3231_OK = 0,
	RTC_DS3231_POWER_FAIL,
	RTC_DS3231_REG_READ_FAIL,
	RTC_DS3231_REG_WRITE_FAIL
}RTC_DS3231_Status;

// Gün isimlerini, isim cinsinden göstermek için.
typedef enum{
	Sunday = 1,
	Monday,
	Tuesday,
	Wednesday,
	Thursday,
	Friday,
	Saturday
}RTC_DS3231_Day_Names;

// Alınan zaman bilgisini tutucak yapı.
typedef struct{
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	char day_name[10];
	uint8_t day_number;
	uint8_t month;
	uint8_t year;
}RTC_DS321_Time;

// RTC başlangıç işlemleri.
RTC_DS3231_Status RTC_Init();

// RTC zaman ve tarih ilk ayarları.
RTC_DS3231_Status RTC_DS3231_Set_Time_And_Date();

// RTC zaman bilgisi alma işlemi.
RTC_DS3231_Status RTC_DS3231_Get_Time(RTC_DS321_Time* time);

// RTC tarih bilgisi alma işlemi.
RTC_DS3231_Status RTC_DS3231_Get_Date(RTC_DS321_Time* time);

// RTC günün ismini alma işlemi.
RTC_DS3231_Status RTC_DS3231_Get_Day_Name(RTC_DS321_Time* time);

// Decimal'den BCD yapıya çeviren yardımcı fonksiyon.
uint8_t DEC_to_BCD(uint8_t value);

// BCD'den decimal yapıya çeviren yardımcı fonksiyon.
uint8_t BCD_to_DEC(uint8_t value);

// Gün isminin int karşılığını alıp string karşılığını veren yardımcı fonksiyon.
char* Get_Day_Name(uint8_t day);

#endif /* RTC_DS3231_H_ */
