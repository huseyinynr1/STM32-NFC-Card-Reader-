#include "crc32.h"

// 8 bitlik array için CRC32 bütünlük değeri hesaplar.
// Verilen byte dizisinin CRC32 bütünlük değerini hesaplar.
uint32_t CRC32_Calculate_Buffer(uint8_t* data, size_t length)
{
	// CRC32 başlangıç değeri atanır.
	uint32_t crc = CRC32_INITIAL;

	// Buffer içindeki veriler byte byte işlenir.
	for(int i = 0; i < length; i++)
	{
		// Okunan byte CRC hesabına dahil edilir.
		crc ^= data[i];

		// Her byte için 8 bitlik CRC işlem adımı uygulanır.
		for(uint8_t bit = 0; bit < 8; bit++)
		{
			// LSB 1 ise polinom ile XOR işlemi uygulanır.
			if(crc & 1U)
			{
				crc = (crc >> 1U) ^ CRC32_POLYNOMIAL;
			}

			// LSB 0 ise sadece sağa kaydırma yapılır.
			else
			{
				crc = (crc >> 1U);
			}
		}
	}

	// Final XOR uygulanarak nihai CRC32 değeri döndürülür.
	return crc ^ CRC32_FINAL_XOR;
}


// FLASH bellekteki veriler için CRC32 bütünlük değeri hesaplar.
uint32_t CRC32_Calculate_Flash(uint32_t start_address, size_t length)
{
	uint32_t crc = CRC32_INITIAL;

	// Belirtilen FLASH adres aralığını byte byte dolaş.
	for(int i = 0; i < length; i++)
	{
		// FLASH'tan okunan byte değerini CRC değerine dahil et.
		crc ^= *(uint8_t*)(start_address + i);

		 // Her byte için 8 bitlik CRC işlem adımını uygula.
		for(uint8_t bit = 0; bit < 8; bit++)
		{
			// En düşük anlamlı(LSB) bit 1 ise
			if(crc & 1U)
			{
				/*
				 * crc değerini 1 bit sağa kaydır
				 * 32 bitlik FLASH Sector adresteki terslenmiş değeri CRC32 polinom değeri ile tekrar tersle.
				 */
				crc = (crc >> 1U) ^ CRC32_POLYNOMIAL;
			}

			// En düşük anlamlı(LSB) bit 0 ise
			else
			{
				// 32 bitlik FLASH Sector adresteki terslenmiş değeri 1 bit sağa kaydır. (LSB = 1 elde etmek için.)
				crc = (crc >> 1U);
			}
		}
	}

	// Oluşan terslenmiş CRC değerini tekrar FINAL CRC32 değeri ile tersleyip dön.
	return crc ^ CRC32_FINAL_XOR;
}


