#include "flash_driver.h"

// FLASH kilidi açmak.
void FLASH_Unlock(void)
{
	if(FLASH->CR & (1U << 31))
	{
		FLASH->KEYR = 0x45670123;
		FLASH->KEYR = 0xCDEF89AB;
	}
}

// FLASH kilitlemek.
void FLASH_Lock(void)
{
	if((FLASH->CR & (1U << 31)) == 0)
	{
		FLASH->CR |= FLASH_CR_LOCK;
	}
}


// FLASH Firmware güncelleme başlangıcını gerçekleştiren fonksiyon.
FLASH_Status_Typedef FLASH_Firmware_Update_Begin()
{
	FLASH_Status_Typedef st = FLASH_Start_Value;

	// Ana uygulama sector block'ları (sector3 - sector7) sil.
	st = FLASH_Erase_Application_Area();
	if(st != FLASH_Status_OK) return st;

	return FLASH_Status_OK;
}


// Firmware parçasını FLASH'a yazar ve yazma adresini günceller.
FLASH_Status_Typedef FLASH_Firmware_Update_Write_Chunk(uint32_t* start_address,
                                                        uint8_t *data,
                                                        size_t data_size,
                                                        uint32_t* received_size)
{
	FLASH_Status_Typedef st = FLASH_Start_Value;

	// Gelen firmware parçasını mevcut FLASH adresine yaz.
	st = FLASH_Write_Bytes(*start_address, data, data_size);
	if(st != FLASH_Status_OK) return st;

	// Bir sonraki yazma adresini 4 byte hizalı olacak şekilde güncelle.
	*start_address += ((data_size + 3) / 4) * 4;

	// Toplam yazılan firmware byte sayısını güncelle.
	*received_size += data_size;

	return FLASH_Status_OK;
}

// Yeni firmware FLASH'a yazıldıktan sonra yapılacak işlemleri kapsar.
FLASH_Status_Typedef FLASH_Firmware_Update_End(uint32_t received_size, uint32_t expected_size,
    uint32_t expected_crc32)
{
	// Alınan boyut beklenen boyuta eşit değilse durumu üst fonksiyona bildir.
    if(received_size != expected_size)
    {
        return FLASH_Status_Invalid_Length;
    }

    // FLASH'a yazılan veriler için CRC32 (bütünlük) hesabı yap.
    uint32_t calculated_crc32 =
        CRC32_Calculate_Flash(MAIN_APP_START_ADDRESS, expected_size);

    // Hesaplanan CRC API'den gelen CRC ila aynı değilse durumu üst fonksiyona bildir.
    if(calculated_crc32 != expected_crc32)
    {
        return FLASH_Status_Data_CRC_Error;
    }

    return FLASH_Status_OK;
}

// Ana uygulama sector blocks temizle.
FLASH_Status_Typedef FLASH_Erase_Application_Area(void)
{
	FLASH_Status_Typedef st = FLASH_Start_Value;

	// Sector block silmek için FLASH kilidi aç.
	FLASH_Unlock();

	// Sector 3'ten sector 7'ye kadar sil ve gerçekten silindi mi kontrol et.
	for(int i = 3; i < 8; i++)
	{
		st = FLASH_Erase_Sector(i);
		if(st != FLASH_Status_OK) goto exit;
		switch (i) {
			case 3:
				st = FLASH_Check_Sector_Value(FLASH_SECTOR_3, FLASH_SECTOR_3_SIZE);
				if(st != FLASH_Status_OK) goto exit;
				break;
			case 4:
				st = FLASH_Check_Sector_Value(FLASH_SECTOR_4, FLASH_SECTOR_4_SIZE);
				if(st != FLASH_Status_OK) goto exit;
				break;
			case 5:
				st = FLASH_Check_Sector_Value(FLASH_SECTOR_5, FLASH_SECTOR_5_SIZE);
				if(st != FLASH_Status_OK) goto exit;
				break;
			case 6:
				st = FLASH_Check_Sector_Value(FLASH_SECTOR_6, FLASH_SECTOR_6_SIZE);
				if(st != FLASH_Status_OK) goto exit;
				break;
			case 7:
				st = FLASH_Check_Sector_Value(FLASH_SECTOR_7, FLASH_SECTOR_7_SIZE);
				if(st != FLASH_Status_OK) goto exit;
				break;
			default:
				break;
		}
	}

	st = FLASH_Status_OK;

	// FLASH kilitle ve fonksiyon sonucu dön.
	exit:
	FLASH_Lock();
	return st;

}


// Belirtilen FLASH sector'ünü siler.
FLASH_Status_Typedef FLASH_Erase_Sector(uint8_t sector_number)
{
	// Bootloader'a ait sector'lerin silinmesini engelle.
	if(sector_number <= 1)
	{
		return FLASH_Status_Invalid_Sector_Number;
	}

	FLASH_Status_Typedef st = FLASH_Start_Value;

	// Önceki FLASH hata bayraklarını temizle.
	FLASH_Clear_Status_Flag();

	// FLASH meşgulse işlemin bitmesini bekle.
	while(FLASH->SR & FLASH_SR_BSY);

	// Silinecek sector numarasını ayarla.
	FLASH->CR &= ~FLASH_CR_SNB;

	// Sector erase modunu aktif et.
	FLASH->CR |= FLASH_CR_SER;

	// Hedef sector numarasını seç.
	FLASH->CR |= (sector_number << 3);

	// Silme işlemini başlat.
	FLASH->CR |= FLASH_CR_STRT;

	// Silme işlemi tamamlanana kadar bekle.
	while(FLASH->SR & FLASH_SR_BSY);

	// FLASH işlem sonucunu kontrol et.
	st = FLASH_Check_Status_Flag();
	if(st != FLASH_Status_OK) goto exit;

	st = FLASH_Status_OK;

	exit:
	// Sector erase modunu kapat.
	FLASH->CR &= ~FLASH_CR_SER;

	// End of operation bayrağını temizle.
	FLASH->SR |= FLASH_SR_EOP;

	// İşlem sonrası hata bayraklarını temizle.
	FLASH_Clear_Status_Flag();

	return st;
}


// Verilen byte dizisini FLASH'a word hizalı olarak yazar.
FLASH_Status_Typedef FLASH_Write_Bytes(uint32_t sector_address, uint8_t* data, size_t data_size)
{
	// Yazma adresinin ana uygulama FLASH alanı içinde olup olmadığını kontrol et.
	if(sector_address < MAIN_APP_START_ADDRESS || sector_address >= FLASH_END_ADDRESS)
	{
		return FLASH_Status_Invalid_Sector_Address;
	}

	// Yazılacak veri buffer'ı geçerli mi kontrol et.
	if(data == NULL) return FLASH_Status_Null_Buffer;

	// Yazılacak veri uzunluğu ana uygulama alanından büyük olamaz.
	if(data_size > MAIN_APP_SIZE) return FLASH_Status_Invalid_Length;

	// Yazılacak byte sayısını 4 byte word sayısına dönüştür.
	uint32_t word_count = data_size % 4 == 0 ? data_size / 4 : (data_size + 3) / 4;

	// Yazılacak FLASH alanının boş olup olmadığını kontrol et.
	for(int i = 0; i < word_count; i++)
	{
		uint32_t sector_value = *(uint32_t*)(sector_address + (i * 4));
		if(sector_value != 0xFFFFFFFF) return FLASH_Status_Sector_Not_Empty;
	}

	FLASH_Status_Typedef st = FLASH_Start_Value;

	// FLASH yazma işlemi için kilidi aç.
	FLASH_Unlock();

	// Byte verileri word formatına çevirmek için geçici dizi oluştur.
	uint32_t combined_arr[word_count];
	memset(combined_arr, 0, word_count * sizeof(uint32_t));

	// uint8_t veri dizisini uint32_t word dizisine dönüştür.
	FLASH_Helper_Uint8_to_Uint32(combined_arr, word_count, data, data_size);

	// Word formatına çevrilen verileri FLASH'a yaz.
	for(int i = 0; i < word_count; i++)
	{
		uint32_t current_address = sector_address + (i * 4);

		st = FLASH_Write_Word(current_address, combined_arr[i]);
		if(st != FLASH_Status_OK)
		{
		    goto exit;
		}
	}

	st = FLASH_Status_OK;

	exit:
	// FLASH kilidini tekrar aktif et.
	FLASH_Lock();

	return st;
}

// FLASH sector adresine veri yazmak.
FLASH_Status_Typedef FLASH_Write_Word(uint32_t sector_address, uint32_t data)
{
	FLASH_Status_Typedef st = FLASH_Start_Value;

	// FLASH'ta işlem varsa bitene kadar bekle.
	while(FLASH->SR & FLASH_SR_BSY);

	// FLASH Status bayrakları temizlemek.
	FLASH_Clear_Status_Flag();

	// Program size bitini 32 yap.
	FLASH->CR &= ~FLASH_CR_PSIZE;
	FLASH->CR |= FLASH_CR_PSIZE_x32;

	// FLASH Program bitini aktif et.
	FLASH->CR &= ~FLASH_CR_PG;
	FLASH->CR |= FLASH_CR_PG;

	// Sector adrese veriyi yaz.
	*(uint32_t*)sector_address = data;

	// Yazma işlemi bitene kadar bekle.
	while(FLASH->SR & FLASH_SR_BSY);

	// FLASH Program bitini kapat.
	FLASH->CR &= ~FLASH_CR_PG;

	// Status bayrakları kontrol et.
	st = FLASH_Check_Status_Flag();
	if(st != FLASH_Status_OK) goto exit;

	// Veri doğru şekilde ilgili adrese yazıldı mı kontrol et.
	uint32_t sector_bit_value = FLASH_Check_Sector_Bit_Value(sector_address);
	if(sector_bit_value != data)
	{
		st = FLASH_Status_Write_Not_Completed;
		goto exit;
	}

	st = FLASH_Status_OK;

	// Program size bitini sıfırla ve sonuç durumunu üst fonksiyona bildir.
	exit:
	FLASH->CR &= ~FLASH_CR_PSIZE;
	return st;

}

FLASH_Status_Typedef FLASH_Check_Status_Flag(void)
{
	uint32_t flash_status_register_value = FLASH->SR;

	if(flash_status_register_value & FLASH_SR_OPERR)
	{
		return FLASH_Status_Operation_Error;
	}

	else if(flash_status_register_value & FLASH_SR_WRPERR)
	{
		return FLASH_Status_Write_Protection_Error;
	}

	else if(flash_status_register_value & FLASH_SR_PGAERR)
	{
		return FLASH_Status_Programming_Alignment_Error;
	}

	else if(flash_status_register_value & FLASH_SR_PGPERR)
	{
		return FLASH_Status_Programming_Parallelism_Error;
	}

	else if(flash_status_register_value & FLASH_SR_PGSERR)
	{
		return FLASH_Status_Programming_Sequence_Error;
	}

	else if(flash_status_register_value & FLASH_SR_RDERR)
	{
		return FLASH_Status_RD_Error;
	}

	else
	{
		return FLASH_Status_OK;
	}
}

void FLASH_Clear_Status_Flag(void)
{
	//Status Bayrakları temizle.
	FLASH->SR |= FLASH_SR_EOP | FLASH_SR_OPERR | FLASH_SR_WRPERR | FLASH_SR_PGAERR
			| FLASH_SR_PGPERR | FLASH_SR_PGSERR | FLASH_SR_RDERR;
}

// FLASH Sector blocks kontrolü
FLASH_Status_Typedef FLASH_Check_Sector_Value(uint32_t sector_address, uint32_t sector_size)
{
	// Gelen sector büyüklüğüne kadar block'ları dolaş.
	for(int i = 0; i < sector_size; i+=4)
	{
		// Sector block'un değerini al.
		uint32_t sector_value = *(uint32_t*)(sector_address + i);

		// sector block değeri 0xFFFFFFFF değilse silme işlemi gerçekleşmemiştir, durumu dön.
		if(sector_value != 0xFFFFFFFF) return FLASH_Status_Erase_Not_Completed;
	}

	// Sector tüm block'ları dolaşılıp değerler istenen değerdeyse OK dön.
	return FLASH_Status_OK;
}

// FLASH sector içindeki 4byte'lık alan değer kontrolü.
uint32_t FLASH_Check_Sector_Bit_Value(uint32_t sector_address)
{
	return *(uint32_t*)sector_address;
}

// uint8_t veri dizisini FLASH yazımı için uint32_t word dizisine dönüştürür.
void FLASH_Helper_Uint8_to_Uint32(uint32_t* word_data,
                                  uint32_t word_count,
                                  uint8_t* data,
                                  size_t data_size)
{
	// Her 4 byte'lık veri grubu bir uint32_t word değerine dönüştürülür.
	for(int i = 0; i < word_count; i++)
	{
		// Eksik kalan son byte'lar için varsayılan değer 0xFF bırakılır.
		uint32_t word = 0xFFFFFFFF;

		// Dönüştürülecek byte grubunun başlangıç index'i hesaplanır.
		uint32_t byte_index = i * 4;

		// 1. byte word değerinin en düşük byte alanına yerleştirilir.
		if(byte_index < data_size)
		{
			word &= ~(0xFF << 0);
			word |= ((uint32_t)data[byte_index]) << 0;
		}

		// 2. byte word değerinin ikinci byte alanına yerleştirilir.
		if((byte_index + 1) < data_size)
		{
			word &= ~(0xFF << 8);
			word |= ((uint32_t)data[byte_index + 1]) << 8;
		}

		// 3. byte word değerinin üçüncü byte alanına yerleştirilir.
		if((byte_index + 2) < data_size)
		{
			word &= ~(0xFF << 16);
			word |= ((uint32_t)data[byte_index + 2]) << 16;
		}

		// 4. byte word değerinin en yüksek byte alanına yerleştirilir.
		if((byte_index + 3) < data_size)
		{
			word &= ~(0xFF << 24);
			word |= ((uint32_t)data[byte_index + 3]) << 24;
		}

		// Oluşturulan 32 bitlik word değeri çıkış dizisine yazılır.
		word_data[i] = word;
	}
}
