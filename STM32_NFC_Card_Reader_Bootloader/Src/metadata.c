#include "metadata.h"

// Metadata alanını silme.
Metadata_Status_Typedef Metadata_Erase()
{
	Metadata_Status_Typedef metadata_status = Metadata_Start_Value;
	FLASH_Status_Typedef fl_st = FLASH_Start_Value;

	// FLASH kilidi aç.
	FLASH_Unlock();

	// Metadata'nın FLASH'ta yazılı olduğu Sector 2 'yi sil.
	fl_st = FLASH_Erase_Sector(2);
	if(fl_st != FLASH_Status_OK)
	{
		metadata_status = Metada_Status_Erase_Not_Successfull;
		goto exit;
	}

	// Silinen sector gerçekten silindi mi kontrol et.
	fl_st = FLASH_Check_Sector_Value(FLASH_SECTOR_2, FLASH_SECTOR_2_SIZE);
	if(fl_st != FLASH_Status_OK)
	{
		metadata_status = Metada_Status_Erase_Not_Successfull;
		goto exit;
	}

	metadata_status = Metadata_Status_OK;

	exit:
	FLASH_Lock(); 			// FLASH kilitle.
	return metadata_status;
}

// Metadata struct nesnesini FLASH'a yazma.
Metadata_Status_Typedef Metadata_Write(Firmware_Metadata_Typedef* metadata)
{
	Metadata_Status_Typedef metadata_status = Metadata_Start_Value;
	// Gelen struct nesnesi başlangıç adresine git. (uint32_t tipinde dizi gibi ele al.)
	uint32_t* metadata_variables = (uint32_t*)metadata;

	// Metadata struct nesnesi boyutunu hesapla. Toplam boyut / uint32_t boyutu.
	uint32_t metadata_size = sizeof(Firmware_Metadata_Typedef) / sizeof(uint32_t);

	// FLASH'a yazmak için FLASH kilidi aç.
	FLASH_Unlock();

	// Metadata değişken değerlerini FLASH'a yaz.
	for(int i = 0; i < metadata_size; i++)
	{
		FLASH_Status_Typedef fl_st = FLASH_Start_Value;

		fl_st = FLASH_Write_Word(METADATA_START_ADDRESS + (i * 4), metadata_variables[i]);
		if(fl_st != FLASH_Status_OK)
		{
			metadata_status =  Metadata_Status_Write_Not_Successfull;
			goto exit;
		}
	}

	metadata_status =  Metadata_Status_OK;

	exit:
	FLASH_Lock();
	return metadata_status;
}

// Metadata değerlerini FLASH'tan okuma işlemi
Metadata_Status_Typedef Metadata_Read(Firmware_Metadata_Typedef* metadata)
{
	uint32_t* metadata_variables = (uint32_t*)metadata;

	uint32_t metadata_size = sizeof(Firmware_Metadata_Typedef) / sizeof(uint32_t);

	for(int i = 0; i < metadata_size; i++)
	{
		metadata_variables[i] = *(uint32_t*)(METADATA_START_ADDRESS + (i* 4));
	}

	return Metadata_Status_OK;
}

// FLASH'tan okunan metadata bilgilerinin geçerliliğini kontrol eder.
Metadata_Status_Typedef Metadata_Is_Valid(Firmware_Metadata_Typedef* metadata)
{
	// Metadata imza değeri beklenen değerle eşleşmeli.
	if(metadata->metadata_signature != METADATA_SIGNATURE)
	{
		return Metadata_Status_Signature_Error;
	}

	// Firmware geçerlilik bayrağı doğru olmalı.
	if(metadata->valid_flag != METADATA_VALID_FLAG)
	{
		return Metadata_Status_Metadata_Invalid;
	}

	// Firmware boyutu geçerli uygulama alanı sınırları içinde olmalı.
	if(metadata->firmware_size == 0 ||
	   metadata->firmware_size == 0xFFFFFFFF ||
	   metadata->firmware_size > MAIN_APP_SIZE)
	{
		return Metadata_Status_Invalid_Firmware_Size;
	}

	// Metadata bütünlük kontrolü için CRC32 hesapla.
	uint32_t calculated_metadata_crc = Metadata_Calculate_CRC(metadata);

	// Hesaplanan CRC ile kayıtlı metadata CRC değeri eşleşmeli.
	if(metadata->metadata_crc32 != calculated_metadata_crc)
	{
		return Metadata_Status_CRC_Error;
	}

	return Metadata_Status_OK;
}

// Metadata yapısının CRC32 bütünlük değerini hesaplar.
uint32_t Metadata_Calculate_CRC(Firmware_Metadata_Typedef* metadata)
{
	// Metadata yapısını byte dizisi olarak ele al.
	uint8_t* metadata_variables = (uint8_t*)metadata;

	// CRC alanı hariç metadata boyutunu hesapla.
	uint32_t metadata_size = sizeof(Firmware_Metadata_Typedef) - sizeof(uint32_t);

	// Metadata alanları için CRC32 hesapla.
	return CRC32_Calculate_Buffer(metadata_variables, metadata_size);
}


