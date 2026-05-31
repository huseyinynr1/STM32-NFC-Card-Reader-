#include "bootloader_driver.h"

// Bootloader başlangıcı için buton okumak.
bool Bootloader_Is_Update_Button_Pressed(void)
{
	if(GPIO_ReadPin(GPIOA, 0))
	{
		while(GPIO_ReadPin(GPIOA, 0));
		Delay_ms(200);
		return true;
	}

	return false;
}

// Bootloader harici donanımları başlatmak.
Bootloader_Status_Typedef Bootloader_Hardware_Init(void)
{
	// USART3 başlangıç konfigürasyonlarını yap ve USART başlat.
	USART_Config();

	// Bootloader'e giriş bilgisi için led blink.
	Bootloader_Entry_Blink();

	SIM800C_Status_Type sim800c_st;

	// SIM800C başlatmayı ve GPRS bağlantısını yapmayı dene.
	for(int i = 0; i < 5; i++)
	{
		sim800c_st = SIM800C_Init();

		if(sim800c_st == SIM800C_Status_OK)
		{
			for(int j = 0 ; j < 5; j++)
			{
				sim800c_st = SIM800C_ConnectGPRS();
				if(sim800c_st == SIM800C_Status_OK) break;
			}
		}

		// SIM800C konfigüre olması için 1 saniye bekle ve SIM800C başlatılmış ve GPRS sağlanmış ise OK dön.
		Delay_ms(1000);
		if(sim800c_st == SIM800C_Status_OK) return Bootloader_Status_OK;
	}

	// Değilse Harici donanımlar başlatılamadı durumu dön.
	return Bootloader_Status_Hardware_Init_Error;
}

// Yeni firmware mevcut mu sorgusu yapar.
Bootloader_Status_Typedef Check_Bootloader_Firmware_Update(char* arr, uint16_t arr_size)
{
	SIM800C_Status_Type sim800c_st;

	// API'ye GET isteği yaparak yeni firmware varmı diye bilgi al.
	sim800c_st = HTTP_GET_Json(GET_CHECK_FIRMWARE_URL, arr, arr_size);

	// Eğer GET isteği başarı ile gerçekleşmediyse durum bildir.
	if(sim800c_st != SIM800C_Status_OK) return Bootloader_Status_HTTP_Error;

	// Değilse OK dön.
	return Bootloader_Status_OK;
}

// Bootloader yeni firmware'i karta yüklemek için başlangıç hazırlıkları yapan fonksiyon.
Bootloader_Status_Typedef Bootloader_Initialize_Preparation(void)
{
	Metadata_Status_Typedef meta_st = Metadata_Start_Value;
	FLASH_Status_Typedef flash_st = FLASH_Start_Value;

	// Metadata sil.
	meta_st = Metadata_Erase();
	if(meta_st != Metadata_Status_OK) return Bootloader_Status_Metadata_Erase_Error;

	// Ana uygulama FLASH sector block'larındaki(sector3 - sector7) verileri sil.
	flash_st = FLASH_Firmware_Update_Begin();
	if(flash_st != FLASH_Status_OK) return Bootloader_Status_Flash_Erase_Error;

	// Sector block'larında veriler başarı ile silindiyse OK dön.
	return Bootloader_Status_OK;
}

// API'den alınan firmware bilgilerine göre güncelleme işlemini gerçekleştirir.
Bootloader_Status_Typedef Bootloader_Perform_Update(http_check_firmware_typedef* firmware_check_object)
{
	// Yeni firmware yoksa güncelleme işlemi yapmadan fonksiyondan çık.
	if(!(firmware_check_object->success && firmware_check_object->isActive))
	    return Bootloader_Status_Firmware_Inactive_Error;

	Bootloader_Status_Typedef boot_st;

	// Bootloader yeni firmware'i karta yüklemek için başlangıç hazırlıkları yap
	boot_st = Bootloader_Initialize_Preparation();
	if(boot_st != Bootloader_Status_OK)
	    return boot_st;

	SIM800C_Status_Type sim800c_st;
	FLASH_Status_Typedef flash_st;
	Metadata_Status_Typedef meta_st;
	Firmware_Metadata_Typedef metadata;

	// Ana uygulama için başlangıç FLASH adresi.
	uint32_t start_address = MAIN_APP_START_ADDRESS;

	// GET işlemi URL'i oluşturmak için array.
	char build_url[128] = {0};

	// Yeni firmware değerlerini parça parça alacak array.
	uint8_t firmware_chunk[FIRMWARE_CHUNK_MAX_SIZE] = {0};

	// Şuana kadar alınan parça değerini tutucak değişken.
	uint32_t offset = 0;

	// Toplam alınan parça değerini tutucak değişken
	uint32_t received_total = 0;

	// Şuanki parça değeri yeni firmware büyüklüğüne ulaşıncaya dek devam et.
	while(offset < firmware_check_object->firmware_size)
	{
		// Kalan yeni firmware parça boyutunu tutucak değişken.
		uint32_t remaining = firmware_check_object->firmware_size - offset;

		// İstenilecek yeni parça boyutunu tutucak değişken.
		uint16_t request_size;

		// İstenicek yeni parça boyutu max parça boyutundan büyükse max parça boyutuna eşitle.
		if(remaining > FIRMWARE_CHUNK_MAX_SIZE)
		{
			request_size = FIRMWARE_CHUNK_MAX_SIZE;
		}

		// Değilse kalan parça boyutuna eşitle.
		else
		{
			request_size = (uint16_t)remaining;
		}


		// Yeni firmware dosya parçasını almak için GET URL oluştur.
		Firmware_Build_Chunk_URL(build_url,
								sizeof(build_url),
								GET_FIRMWARE_CHUNK_URL,
								firmware_check_object->firmware_id,
								offset,
								request_size);

		/*
		 * Yeni firmware dosya parçasını almak için GET isteği yap.
		 * Başarısız olursa 3 kere aynı denemeyi yap.
		 * 3 deneme sonunda GET isteği gerçekleştirilemez ise durum sonucunu üst fonksiyona dön.
		 */
		for(int i = 0; i < 2; i++)
		{
			sim800c_st = HTTP_GET_Raw_Binary(build_url, firmware_chunk, request_size);
			if(sim800c_st == SIM800C_Status_OK) break;
		}
		if(sim800c_st != SIM800C_Status_OK) return Bootloader_Status_HTTP_Error;

		/*
		 * GET isteği ile alınan yeni firmware dosya parçasını boyutu kadar FLASH sector block'a yaz.
		 * Yazma gerçekleşmez ise 3 tekrar defa dene.
		 * 3 deneme sonucunda işlem başarısız ise durum sonucunu üst fonksiyona bildir.
		 */
		for(int i = 0; i < 2; i++)
		{
			flash_st = FLASH_Firmware_Update_Write_Chunk(&start_address, firmware_chunk, request_size,  &received_total);
			if(flash_st == FLASH_Status_OK) break;
		}
		if(flash_st != FLASH_Status_OK) return Bootloader_Status_Flash_Write_Error;

		// Alınan(istenen) parça boyutunu şu ana kadar alınan parça boyutu üzerine ekle.
		offset += request_size;
	}

	// FLASH'a yazma işleminden sonra güncelleme işlemini doğru şekilde bitir.
	flash_st = FLASH_Firmware_Update_End(received_total,
										firmware_check_object->firmware_size,
										firmware_check_object->firmware_crc32);
	if(flash_st != FLASH_Status_OK) return Bootloader_Status_Flash_CRC_Error;

	// Yeni firmware bilgilerine göre metadata değişkenlerine atama yap.
	metadata.metadata_signature = METADATA_SIGNATURE;
	metadata.firmware_crc32 	= firmware_check_object->firmware_crc32;
	metadata.firmware_version 	= firmware_check_object->firmware_version;
	metadata.firmware_size 		= received_total;
	metadata.valid_flag 		= METADATA_VALID_FLAG;
	metadata.metadata_crc32 	= Metadata_Calculate_CRC(&metadata);

	// Metadata verilerini FLASH'a yaz.
	meta_st = Metadata_Write(&metadata);
	if(meta_st != Metadata_Status_OK) return Bootloader_Status_Metadata_Write_Error;

	// Yeni firmware dosyası MCU FLASH'a yazıldığını API'ye bildirmek için response ve POST URL hazırla
	char firmware_update_isActive[40] = {0};
	char firmware_update_isActive_response [100] = {0};
	firmware_check_object->isActive = false;
	Firmware_Update_IsActive_Convert_Json(firmware_update_isActive,
										  firmware_check_object->firmware_id,
										  firmware_check_object->isActive);

	// Yüklenen firmware'in başarı ile yüklendiğini POST işlemi ila bildir.
	sim800c_st = HTTP_POST(POST_FIRMWARE_ISACTIVE_UPDATE_URL,
						   firmware_update_isActive,
						   firmware_update_isActive_response,
						   sizeof(firmware_update_isActive_response),
						   20000);

		if(sim800c_st != SIM800C_Status_OK) return Bootloader_Status_HTTP_Error;
		bool success = false;

		// POST response'u kontrol et.
		Firmware_Update_IsActive_Response_Json_Convert(firmware_update_isActive_response, &success);
		if(!success) return Bootloader_Status_Firmware_Inactive_Error;


	return Bootloader_Status_OK;
}

// Ana uygulamaya geçiş öncesi bootloader'de kullanılan çevre birimlerini resetler.
void Bootloader_DeInit(void)
{
    //SysTick kapatılır.

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /*
     * USART3 kapatılır.
     * Bootloader SIM800C haberleşmesinde USART3 kullandığı için,
     * ana uygulama farklı baudrate veya farklı ayarlarla tekrar başlatabilsin.
     */
    USART3->CR1 &= ~USART_CR1_UE;
    USART3->CR1 = 0;
    USART3->CR2 = 0;
    USART3->CR3 = 0;
    USART3->BRR = 0;

    /*
     * USART3 clock hattı kapatılır.
     */
    RCC->APB1ENR &= ~RCC_APB1ENR_USART3EN;

    /*
     * USART3 pinleri olan PB10/PB11 reset benzeri giriş moduna alınır.
     * Böylece ana uygulama bu pinleri kendi ihtiyacına göre yeniden yapılandırır.
     */
    GPIOB->MODER &= ~((3U << 20) | (3U << 22));
    GPIOB->OTYPER &= ~((1U << 10) | (1U << 11));
    GPIOB->OSPEEDR &= ~((3U << 20) | (3U << 22));
    GPIOB->PUPDR &= ~((3U << 20) | (3U << 22));
    GPIOB->AFR[1] &= ~((0xFU << 8) | (0xFU << 12));


    //SIM800C PWRKEY pini olan PA15 temizlenir.
    GPIOA->MODER &= ~(3U << 30);
    GPIOA->OTYPER &= ~(1U << 15);
    GPIOA->OSPEEDR &= ~(3U << 30);
    GPIOA->PUPDR &= ~(3U << 30);


    // Bootloader LED pini PD12 reset benzeri giriş moduna alınır.
    GPIOD->MODER &= ~(3U << 24);
    GPIOD->OTYPER &= ~(1U << 12);
    GPIOD->OSPEEDR &= ~(3U << 24);
    GPIOD->PUPDR &= ~(3U << 24);

    /*
     * FLASH kilitlenir.
     * Bootloader FLASH yazma/silme yaptıysa application'a geçmeden önce kilitli bırakmak daha güvenlidir.
     */
    FLASH->CR |= FLASH_CR_LOCK;

    /*
     * Kullanılan GPIO clock'ları kapatılır.
     * Ana uygulama zaten kendi GPIO init fonksiyonlarında tekrar açacaktır.
     */
    RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOAEN;
    RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOBEN;
    RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIODEN;
}

// Bootloader'den ana uygulamaya geçmeyi gerçekleştirir.
Bootloader_Status_Typedef Bootloader_Try_Jump_To_Application(void)
{
	Metadata_Status_Typedef meta_st;
	Application_Status_Typedef app_st;
	Firmware_Metadata_Typedef metadata;

	// FLASH'tan firmware metadata bilgilerini oku.
	meta_st = Metadata_Read(&metadata);
	if(meta_st != Metadata_Status_OK)
	{
		Bootloader_Metadata_Invalid_Blink();
		return Bootloader_Status_Metadata_Read_Error;
	}

	// Okunan metadata bilgilerinin geçerliliğini kontrol et.
	meta_st = Metadata_Is_Valid(&metadata);
	if(meta_st != Metadata_Status_OK)
	{
		Bootloader_Metadata_Invalid_Blink();
		return Bootloader_Status_Metadata_Invalid;
	}

	// Ana uygulamanın FLASH üzerindeki CRC değerini hesapla.
	uint32_t app_crc = CRC32_Calculate_Flash(MAIN_APP_START_ADDRESS, metadata.firmware_size);

	// Hesaplanan CRC ile metadata içindeki CRC değerini karşılaştır.
	if(app_crc != metadata.firmware_crc32) return Bootloader_Status_Main_App_CRC_Error;

	// Ana uygulamanın başlangıç adreslerini kontrol et.
	app_st = Application_Control();
	if(app_st != Application_Valid)
	{
		Bootloader_Application_Invalid_Blink();
		return Bootloader_Status_Main_App_Invalid;
	}

	// Tüm kontroller başarılıysa ana uygulamaya geç.
	Jump_To_Application();
}
