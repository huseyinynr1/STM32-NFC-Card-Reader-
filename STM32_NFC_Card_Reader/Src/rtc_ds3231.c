#include "rtc_ds3231.h"

// RTC başlangıç işlemleri.
RTC_DS3231_Status RTC_Init()
{
	// 250ms bekle ve RST pini oku, eğer LOW değilse güçte sorun var.
	Delay_Ms(250);
	if((GPIOB->IDR & GPIO_IDR_IDR_5) == 0) return RTC_DS3231_POWER_FAIL;

	// Veriler ve işlem sonuçlarını tutucak değişkenler.
	uint8_t status_reg_value = 0;
	uint8_t temp_upper_value = 0;
	uint8_t temp_lower_value = 0;
	I2C_Status_t st;
	RTC_DS3231_Status st_rtc;

	// Status register'ı oku ve işlem başarılı değilse register okuma başarısız.
	st = I2C1_Read(Slave_Addrr, Status_Reg, &status_reg_value);
	if(st != I2C_STATUS_OK) return RTC_DS3231_REG_READ_FAIL;

	// Eğer OSF = 1 ise şuanki zaman ve tarih bilgilerini RTC'ye yaz.
	if(status_reg_value & 0x80)
	{
		st_rtc = RTC_DS3231_Set_Time_And_Date();
		if(st_rtc != RTC_DS3231_OK) return st_rtc;

		status_reg_value &= ~(1U << 7);
		st = I2C1_Write(Slave_Addrr, Status_Reg, status_reg_value);
		if(st != I2C_STATUS_OK) return RTC_DS3231_REG_WRITE_FAIL;
		status_reg_value = 0;
	}

	// EN32kHz temizle (32kHz pini disable.)
	st = I2C1_Read(Slave_Addrr, Status_Reg, &status_reg_value);
	if(st != I2C_STATUS_OK) return RTC_DS3231_REG_READ_FAIL;
	status_reg_value &= ~(1U << 3);
	st = I2C1_Write(Slave_Addrr, Status_Reg, status_reg_value);
	if(st != I2C_STATUS_OK) return RTC_DS3231_REG_WRITE_FAIL;
	status_reg_value = 0;

	// BSY = 0 olunca sıcaklık değerini oku , eğer bu işlem başarılıysa RTC sorunsuz başlamıştır.
	for(int i = 0; i < 3 ; i++)
	{
		st = I2C1_Read(Slave_Addrr, Status_Reg, &status_reg_value);
		Delay_Ms(10);
		if(st != I2C_STATUS_OK) return RTC_DS3231_REG_READ_FAIL;

		if((status_reg_value & 0x04) == 0)
		{
			st = I2C1_Read(Slave_Addrr, Temperature_Reg_Upper, &temp_upper_value);
			if(st != I2C_STATUS_OK) return RTC_DS3231_REG_WRITE_FAIL;
			st = I2C1_Read(Slave_Addrr, Temperature_Reg_Lower, &temp_lower_value);
			if(st != I2C_STATUS_OK) return RTC_DS3231_REG_WRITE_FAIL;

			uint8_t fractional_part = (temp_lower_value >> 6) & 0x03;
			float temperature = (float)temp_upper_value + (float)(fractional_part * 0.25f);
			break;
		}
		Delay_Ms(1000);
	}

	return RTC_DS3231_OK;
}

// RTC zaman ve tarih ilk ayarları.
RTC_DS3231_Status RTC_DS3231_Set_Time_And_Date()
{
	// Zaman bilgileri tutan değişken
	uint8_t data_second = 0;
	uint8_t data_minute = 40;
	uint8_t data_hour = 17;
	I2C_Status_t st_time;

	// Tarih bilgileri tutan değişken
	RTC_DS3231_Day_Names day = Saturday;
	uint8_t day_number = 2;
	uint8_t month = 4;
	uint8_t year = 26;
	I2C_Status_t st_date;

	// Zaman bilgilerini DEC'ten BCD'ye çevir ve ilgili register'a yaz.
	data_second = DEC_to_BCD(data_second);
	st_time = I2C1_Write(Slave_Addrr, Second_Reg, data_second);
	if(st_time != I2C_STATUS_OK) return RTC_DS3231_REG_WRITE_FAIL;

	data_minute = DEC_to_BCD(data_minute);
	st_time = I2C1_Write(Slave_Addrr, Minute_Reg, data_minute);
	if(st_time != I2C_STATUS_OK) return RTC_DS3231_REG_WRITE_FAIL;

	data_hour = DEC_to_BCD(data_hour);
	st_time = I2C1_Write(Slave_Addrr, Hour_Reg, data_hour);
	if(st_time != I2C_STATUS_OK) return RTC_DS3231_REG_WRITE_FAIL;

	// Tarih bilgilerini DEC'ten BCD'ye çevir ve ilgili register'a yaz.
	st_date = I2C1_Write(Slave_Addrr, Day_Name_Reg, day);
	if(st_date != I2C_STATUS_OK) return RTC_DS3231_REG_WRITE_FAIL;

	day_number = DEC_to_BCD(day_number);
	st_date = I2C1_Write(Slave_Addrr, Day_Number_Reg, day_number);
	if(st_date != I2C_STATUS_OK) return RTC_DS3231_REG_WRITE_FAIL;

	month = DEC_to_BCD(month);
	st_date = I2C1_Write(Slave_Addrr, Month_Reg, month);
	if(st_date != I2C_STATUS_OK) return RTC_DS3231_REG_WRITE_FAIL;

	year = DEC_to_BCD(year);
	st_date = I2C1_Write(Slave_Addrr, Year_Reg, year);
	if(st_date != I2C_STATUS_OK) return RTC_DS3231_REG_WRITE_FAIL;

	return RTC_DS3231_OK;
}

// RTC zaman bilgisi alma işlemi.
RTC_DS3231_Status RTC_DS3231_Get_Time(RTC_DS321_Time* time)
{
	// Zaman bilgilerini tutucak değişkenler.
	uint8_t data_second = 0;
	uint8_t data_minute = 0;
	uint8_t data_hour   = 0;
	I2C_Status_t st_time;

	// İlgili register'lardan zaman bilgilerini oku.
	st_time = I2C1_Read(Slave_Addrr, Second_Reg, &data_second);
	if(st_time != I2C_STATUS_OK) return RTC_DS3231_REG_READ_FAIL;

	st_time = I2C1_Read(Slave_Addrr, Minute_Reg, &data_minute);
	if(st_time != I2C_STATUS_OK) return RTC_DS3231_REG_READ_FAIL;

	st_time = I2C1_Read(Slave_Addrr, Hour_Reg, &data_hour);
	if(st_time != I2C_STATUS_OK) return RTC_DS3231_REG_READ_FAIL;

	// Okunan zaman bilgileri BCD'den DEC'e çevir ve structure'da ilgili değişkene bilgiyi yaz.
	time->second = BCD_to_DEC(data_second);
	time->minute = BCD_to_DEC(data_minute);
	data_hour &= 0x3F;
	time->hour = BCD_to_DEC(data_hour);

	return RTC_DS3231_OK;
}

// RTC tarih bilgisi alma işlemi.
RTC_DS3231_Status RTC_DS3231_Get_Date(RTC_DS321_Time* time)
{
	// Tarih bilgilerini tutucak değişkenler.
	uint8_t data_day_name = 0;
	uint8_t data_day = 0;
	uint8_t data_month = 0;
	uint8_t data_year   = 0;
	I2C_Status_t st_date;

	// İlgili register'lardan zaman bilgilerini oku.
	st_date = I2C1_Read(Slave_Addrr, Day_Name_Reg, &data_day_name);
	if(st_date != I2C_STATUS_OK) return RTC_DS3231_REG_READ_FAIL;

	st_date = I2C1_Read(Slave_Addrr, Day_Number_Reg, &data_day);
	if(st_date != I2C_STATUS_OK) return RTC_DS3231_REG_READ_FAIL;

	st_date = I2C1_Read(Slave_Addrr, Month_Reg, &data_month);
	if(st_date != I2C_STATUS_OK) return RTC_DS3231_REG_READ_FAIL;

	st_date = I2C1_Read(Slave_Addrr, Year_Reg, &data_year);
	if(st_date != I2C_STATUS_OK) return RTC_DS3231_REG_READ_FAIL;

	// Okunan zaman bilgileri BCD'den DEC'e çevir ve structure'da ilgili değişkene bilgiyi yaz.
	memcpy(time->day_name, Get_Day_Name(data_day_name), 10); // Gün isminin int değerine göre string karşılığını al ve ilgili struct değişkenine yaz.
	time->day_number = BCD_to_DEC(data_day);
	data_month &= 0x1F;                       // Ay bilgisi için maskeleme yap(bit0...bit5)
	time->month = BCD_to_DEC(data_month);
	time->year = BCD_to_DEC(data_year);

	return RTC_DS3231_OK;
}

//  Decimal'den BCD yapıya çeviren yardımcı fonksiyon.
uint8_t DEC_to_BCD(uint8_t value)
{
	return ((value / 10) << 4) | (value % 10);
}

// BCD'den decimal yapıya çeviren yardımcı fonksiyon.
uint8_t BCD_to_DEC(uint8_t value)
{
	return((value >> 4) * 10) + (value & 0x0F);
}

// Gün isminin int karşılığını alıp string karşılığını veren yardımcı fonksiyon.
char* Get_Day_Name(uint8_t day)
{
	switch (day) {
		case 1: return "Pazar";
		case 2: return "Pazartesi";
		case 3: return "Salı";
		case 4: return "Çarşamba";
		case 5: return "Perşembe";
		case 6: return "Cuma";
		case 7: return "Cumartesi";
		default: return "Invalid day number"; break;
	}
}


