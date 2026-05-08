#include "rc522.h"

// RC522 NFC başlangıç konfigürasyonları
RC522_Status_Type RC522_Init(void)
{
	RC522_Status_Type st;
	GPIOA->BSRR = GPIO_BSRR_BS4;     // CS HIGH

	GPIOA->BSRR = GPIO_BSRR_BR3;     // NRSTPDN LOW

	Delay_Ms(1);				     // wait.

	GPIOA->BSRR = GPIO_BSRR_BS3;     // NRST HIGH

	Delay_Ms(50);    				 // wait.

	// Read kontrolü
	uint8_t ver = 0;
	st = RC522_ReadReg(Version_Reg, &ver);
	if(st != RC522_Status_OK) return st;
    if (ver == 0x00 || ver == 0xFF) return RC522_SPI_Comm_Fail; // Dönen değerler sürekli 0x00 veya 0xFF  olursa SPI hattında sorun var döngüden çık.

	// SPI haberleşmesi çalışıyor mu testi. Çalışmıyorsa durumu dön çalışıyorsa eski değeri tekrar yaz.
    uint8_t test_old = 0;
	st = RC522_ReadReg(T_Reload_Reg_H, &test_old);
	if(st != RC522_Status_OK) return st;

	st = RC522_WriteReg(T_Reload_Reg_H, 0x5A);
	if(st != RC522_Status_OK) return st;

	uint8_t test_now = 0;
	st = RC522_ReadReg(T_Reload_Reg_H, &test_now);
	if(st != RC522_Status_OK) return st;
	if(test_now != 0x5A) return RC522_SPI_Comm_Fail;

	st = RC522_WriteReg(T_Reload_Reg_H, test_old);
	if(st != RC522_Status_OK) return st;

	// RC522'yi sıfırla
	st = RC522_WriteReg(Command_Reg, 0x0F);
	if(st != RC522_Status_OK) return st;

	// FIFO tamponu temizle
	st = RC522_WriteReg(FIFO_Level_Reg, 0x80);
	if(st != RC522_Status_OK) return st;

	Delay_Ms(50);

	// RC522 versiyonuna göre TPrescalEven 0 yapıp versiyon 1 olarak bildirme.
	uint8_t t_prescaler_even = 0;
	st = RC522_ReadReg(Demod_Reg, &t_prescaler_even);
	if(st != RC522_Status_OK) return st;

	if(t_prescaler_even & (1U << 4))
	{
		t_prescaler_even &= ~(1U << 4);
		st = RC522_WriteReg(Demod_Reg, t_prescaler_even);
		if(st != RC522_Status_OK) return st;
	}

	// TAuto = 1 , timer otomatik başlasın.
	uint8_t t_mode = 0;
	st = RC522_ReadReg(T_Mode_Reg, &t_mode);
	if(st != RC522_Status_OK) return st;
	t_mode |= (1U << 7);
	t_mode &= ~0x0F;       // TPrescaler_Hi = 0
	st = RC522_WriteReg(T_Mode_Reg, t_mode);
	if(st != RC522_Status_OK) return st;

	// Prescaler = 169
	st =RC522_WriteReg(T_Prescaler_Reg_L, 169);
	if(st != RC522_Status_OK) return st;

	// Reload = 1000
	st = RC522_WriteReg(T_Reload_Reg_H, 0x03);
	if(st != RC522_Status_OK) return st;
	st = RC522_WriteReg(T_Reload_Reg_L, 0xE8);
	if(st != RC522_Status_OK) return st;



	// Veri iletimi sırasında bit hızı = 106 kBd
	uint8_t tx_bit_speed = 0;
	st = RC522_ReadReg(Tx_Mode_Reg, &tx_bit_speed);
	if(st != RC522_Status_OK) return st;
	tx_bit_speed &= ~(7U << 4);
	st = RC522_WriteReg(Tx_Mode_Reg, tx_bit_speed);
	if(st != RC522_Status_OK) return st;

	// Veri alımı sırasında bit hızı = 106 kBd
	uint8_t rx_bit_speed = 0;
	st = RC522_ReadReg(Rx_Mode_Reg, &rx_bit_speed);
	if(st != RC522_Status_OK) return st;
	rx_bit_speed &= ~(7U << 4);
	st = RC522_WriteReg(Rx_Mode_Reg, rx_bit_speed);
	if(st != RC522_Status_OK) return st;

	// parity açık kalacak.
	uint8_t parity_disable = 0;
	st = RC522_ReadReg(Mf_Rx_Reg, &parity_disable);
	if(st != RC522_Status_OK) return st;
	parity_disable &= ~(1U << 4);
	st = RC522_WriteReg(Mf_Rx_Reg, parity_disable);
	if(st != RC522_Status_OK) return st;

    // 100% ASK zorla.
	uint8_t force_100_ask = 0;
	st = RC522_ReadReg(Tx_ASK_Reg, &force_100_ask);
	if(st != RC522_Status_OK) return st;
	force_100_ask |= (1 << 6);
	st = RC522_WriteReg(Tx_ASK_Reg, force_100_ask);
	if(st != RC522_Status_OK) return st;

	// TX1/TX2 13.56 MHz taşıyıcıyı (data ile modüle) sürer
	uint8_t tx1_tx2_rf_en = 0;
	st = RC522_ReadReg(Tx_Control_Reg, &tx1_tx2_rf_en);
	if(st != RC522_Status_OK) return st;
	tx1_tx2_rf_en |= (3U << 0);
	st = RC522_WriteReg(Tx_Control_Reg, tx1_tx2_rf_en);
	if(st != RC522_Status_OK) return st;


	return RC522_Status_OK;
}

// İlk çalışma testi yapılacak fonksiyon.
RC522_Status_Type RC522_Selftest(void)
{
	// Tx1RFEn/Tx2RFEn bitleri 1 mi? (Anten açıldı mı?)
	RC522_Status_Type st;
	uint8_t tx_control_reg_value = 0;
	st = RC522_ReadReg(Tx_Control_Reg, &tx_control_reg_value);
	if(st != RC522_Status_OK) return st;

	uint8_t tx1_tx2_rf_en = 0x03;
	if((tx_control_reg_value & tx1_tx2_rf_en) != 0x03) return RC522_Antenna_Enable_Fail;

	// FIFO’yu flush et (temizle)
	st = RC522_FIFO_Flush();
	if(st != RC522_Status_OK) return st;

	// IRQ bayraklarını temizle)
	st = RC522_IRQ_Clear();
	if(st != RC522_Status_OK) return st;


	// FIFO'ya 0x26 yaz
	uint8_t fifo_data_reg_value = 0x26;
	st = RC522_WriteReg(FIFO_Data_Reg, fifo_data_reg_value);
	if(st != RC522_Status_OK) return st;

	// TxLastBits = 7
	st = RC522_SetTxLastBits(7);
	if(st != RC522_Status_OK) return st;

	// Transceive ile bir işlem başlat
	st = RC522_CommandReg_CommandBitValue(Transceive);
	if(st != RC522_Status_OK) return st;

	// Start send=1, veri iletimini başlat
	st = RC522_StartSend(1);
	if(st != RC522_Status_OK) return st;

	for(int i = 0 ; i < 50 ; i++)
	{
		// Timer IRQ 1 ise self test doğru fonksiyondan çık ve doğru bildir.
		vTaskDelay(pdMS_TO_TICKS(2));
		uint8_t com_irq_value = 0;
		st = RC522_ReadReg(Com_Irq_Reg, &com_irq_value);
		if(st != RC522_Status_OK) return st;

		uint8_t timer_irq = (com_irq_value & (1U << 0));
		uint8_t rx_irq = (com_irq_value & (1U << 5));
		uint8_t idle_irq = (com_irq_value & (1U << 4));
		uint8_t err_irq = (com_irq_value & (1U << 1));

		if(timer_irq)
		{
		    RC522_StartSend(0);
		    return RC522_Selftest_Timeout;
		}

		if(rx_irq || idle_irq)
		{
			st = RC522_StartSend(0);
			if(st != RC522_Status_OK) return st;
			return RC522_Status_OK;
		}

		if(err_irq)
		{
			st = RC522_StartSend(0);
			if(st != RC522_Status_OK) return st;
			return RC522_Selftest_Error_IRQ;
		}
	}

	st = RC522_StartSend(0);
	if(st != RC522_Status_OK) return st;
	return RC522_Selftest_Timeout;
}

// Authenticate test etmek için fonksiyon.
RC522_Status_Type RC522_Authenticate_Test(void)
{
	RC522_Status_Type st;
	// Kart'ın id'si tutucak değişkenller
	uint8_t card_id[10]= {0};
	uint8_t card_id_length = 0;

	// Kart yoksa veya kart id alma başarısız ise hata dön.
	st = RC522_IsNewCardPresent();

	if(st != RC522_Card_Found) return st;

	st = RC522_ReadCardSerial(card_id, &card_id_length);
	if(st != RC522_Card_ID_Found) return st;

	// Önceki işlemi temizle ve hata oluşursa hata durumu dön.
	st = RC522_CommandReg_CommandBitValue(Idle);
	if(st != RC522_Status_OK) return st;

	// FIFO'yu temizle ve hata oluşursa hata durumu dön.
	st = RC522_FIFO_Flush();
	if(st != RC522_Status_OK) return st;

	// Interrupt bayraklarını temizle ve hata oluşursa hata durumu dön.
	st = RC522_IRQ_Clear();
	if(st != RC522_Status_OK) return st;

	// Tam byte gidecek ve hata oluşursa hata durumu dön.
	st = RC522_SetTxLastBits(0);
	if(st != RC522_Status_OK) return st;

	/* Authenticate başlatmak için gerekli komut dizisi oluşturma,
	KeyA adres, blok adres, KeyA değeri, kart id'in ilk 4 byte'ı hata oluşursa hata durumu dön.*/
	uint8_t block_adrr_4 = 0x04;
	uint8_t auth_cmd[12] = {KEYA, block_adrr_4, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, card_id[0], card_id[1], card_id[2], card_id[3]};
	st = RC522_Write_FIFO(auth_cmd, sizeof(auth_cmd));
	if(st != RC522_Status_OK) return st;

	// Komut bitini authenticate işlemi yap hata oluşursa hata durumu dön.
 	st = RC522_CommandReg_CommandBitValue(MFAuthent);
	if(st != RC522_Status_OK) return st;

 	// IRQ bayraklarının yeterli sürede kalkmasını bekle ve yeterli sürede idle_irq bayrağı kalkmadıysa fonksiyondan çık
 	st = RC522_WaitIRQForMFAuthent();
    if(st != RC522_Status_OK) return st;

    // Error register'inda herhangi bir hata bayrağı kalkmışmı kontrol et eğer kalkmışsa fonksiyondan çık
    uint8_t error_reg_value = 0;
 	st = RC522_ReadReg(Error_Reg, &error_reg_value);
    if(st != RC522_Status_OK) return st;
 	if(error_reg_value != 0) return RC522_Auth_Error_Reg_Set;

 	// MFCrypto1ON bayrağı kalkmış mı kontrol et , eğer kalkmış ise authenticate başarılıdır.
 	uint8_t mf_crypto1_on_value = 0;
 	st = RC522_ReadReg(Status2_Reg, &mf_crypto1_on_value);
    if(st != RC522_Status_OK) return st;
    mf_crypto1_on_value &= 0x08;
 	if(mf_crypto1_on_value != 0x08) return RC522_Auth_Crypto1_Not_Enabled;

 	// Test için okuma işlemi
	uint8_t test_read[2] = {MIFARE_Read, card_sector1_block0};
	uint8_t test_read_len = 2;
	uint8_t test_read_tr_last_bits = 0;
	uint8_t test_read_fifo[18] = {0};
	uint8_t test_read_fifo_len = 0;
	uint8_t test_read_last_bits = 0;
	bool test_read_crc_en = true;
	bool test_read_expect_response = true;

	st = RC522_Transceive(test_read, test_read_len, test_read_tr_last_bits, test_read_fifo, &test_read_fifo_len,
			sizeof(test_read_fifo), &test_read_last_bits, test_read_crc_en, test_read_expect_response);
	if(st != RC522_Status_OK) return st;

	if(test_read_fifo_len >=16 && test_read_last_bits == 0)
	{
		uint8_t error_reg_value_after_read_card = 0;
	 	st = RC522_ReadReg(Error_Reg, &error_reg_value_after_read_card);
		if(st != RC522_Status_OK) return st;
		if(error_reg_value_after_read_card != 0) return RC522_Test_Read_Fail;
		return RC522_Status_OK;
	}

	// Burada ilk test için test_read_fifo dizisi değerine bakacağız.
 	return RC522_Test_Read_Invalid_Response;
}

// HALT durumundaki bir kartı WUPA komutu ile tekrar cevap verebilir hale getirmek için kullanılır.
RC522_Status_Type RC522_WakeupCard(void)
{
	// WUPA(Wake up A) komutu hazırlama
	uint8_t wupa_adrr = WUPA;
	uint8_t wupa_adrr_len = 1;
	uint8_t wupa_adrr_tr_last_bits = 7;
	uint8_t wupa_adrr_response[2] = {0};
	uint8_t wupa_adrr_response_len = 0;
	uint8_t wupa_adrr_response_last_bits = 0;
	bool wupa_adrr_crc_en = false;
	bool wupa_adrr_expect_response = true;

	// Karta wupa komutu yollamak.
	RC522_Status_Type st = RC522_Transceive(&wupa_adrr, wupa_adrr_len, wupa_adrr_tr_last_bits, wupa_adrr_response, &wupa_adrr_response_len,
			sizeof(wupa_adrr_response), &wupa_adrr_response_last_bits, wupa_adrr_crc_en, wupa_adrr_expect_response);

	// Cevap formatını kontrol et.
	if(st == RC522_Status_OK)
	{
		// ATQA cevabı 2 byte olmalı
		if(wupa_adrr_response_len == 2)
		{
			// Son byte tam byte olmalı.
			if(wupa_adrr_response_last_bits == 0)
			{
				// Kart wakeup komutuna geçerli cevabı verdi.
				return RC522_Status_OK;
			}
		}
	}

	// Wake up işlemi başarısız oldu
	return RC522_Card_Wakeup_Fail;
}

// Kart var mı diye sorgulama fonksiyonu.
RC522_Status_Type RC522_IsNewCardPresent(void)
{
	for(int retry = 0 ; retry < 3; retry++)
	{
		// REQA (request A) komutu hazırlama
		uint8_t reqa_addrr = REQA;           // Kart var mı diye sorgulama komutu
		uint8_t reqa_addrr_len = 1;          // Gönderilecek veri uzunluğu
		uint8_t reqa_addrr_tr_last_bits = 7; // bu komut kısa çağrı komutu olduğu için last bits 7
		uint8_t reqa_addrr_response[2]={0};  // Geçici alıcı dizi (ATQA 0x0004 cevabı alınacak.)
		uint8_t reqa_addrr_response_len = 0; // Dizi uzunluk değişkeni
		uint8_t reqa_addrr_rx_last_bits = 0; // Cevap'taki byte'in kaç biti geçerli olacağını söylemek ( 0 == 8 bit)
		bool reqa_addrr_crc_en = false;
		bool reqa_addrr_expect_response = true;


		// Hazırlanan REQA komutu yollama.
		RC522_Status_Type st = RC522_Transceive(&reqa_addrr, reqa_addrr_len, reqa_addrr_tr_last_bits, reqa_addrr_response,
				&reqa_addrr_response_len, sizeof(reqa_addrr_response), &reqa_addrr_rx_last_bits,
				reqa_addrr_crc_en, reqa_addrr_expect_response);

		// Cevap formatını kontrol et.
		if(st != RC522_Status_OK)
		{
			vTaskDelay(pdMS_TO_TICKS(50));
			continue;
		}
		// ATQA cevabı 2 byte olmalı ve son byte tam byte olmalı.
		if(reqa_addrr_response_len == 2 && reqa_addrr_rx_last_bits == 0)
		// Kart REQA komutuna geçerli cevabı verdi ve kart bulundu.
		return RC522_Card_Found;

        vTaskDelay(pdMS_TO_TICKS(50));

	}
	// Kart bulunamadı.
	return RC522_No_Card_Present;
}

// Kart'ın UID'sini alma işlemini gerçekleştiren fonksiyon.
RC522_Status_Type RC522_ReadCardSerial(uint8_t *uid, uint8_t* len)
{

	for(int retry = 0; retry < 3; retry++)
	{
		RC522_StopCrypto(); // Eğer önceki işlemden açık kaldıysa kapat.
		RC522_CommandReg_CommandBitValue(Idle);
		RC522_FIFO_Flush();
		RC522_IRQ_Clear();
		RC522_SetTxLastBits(0);

		uint8_t cascade_level_1[2] = {0x93, 0x20};  // Kart ID sorgulama komutu. (0x93 = cascade level1, 0x20 = 32 bit UID iste).
		uint8_t cl_len = 2;                         // cascade level 1 dizisi uzunluğu
		uint8_t cl_tx_last_bits = 0;                // bu komut normal çağrı komutu olduğu için last bits 8
		uint8_t cl_rx_buffer[5] ={0};                    // Geçici alıcı dizi
		uint8_t cl_rx_len = 0;                      // Dizi uzunluk değişkeni
		uint8_t cl_rx_last_bits = 0;                // Cevap'taki byte'in kaç biti geçerli olacağını söylemek ( 0 == 8 bit)
		bool cl_crc_en = false;                     // Crc disable
		bool cl_expect_response = true;             // Gönderim cevabı bekleniyor.

		// Cascade Level 1 anti-collision komutunu gönder ve cevabı al.
		RC522_Status_Type st = RC522_Transceive(cascade_level_1, cl_len, cl_tx_last_bits, cl_rx_buffer,
				&cl_rx_len, sizeof(cl_rx_buffer), &cl_rx_last_bits, cl_crc_en, cl_expect_response);
		if(st != RC522_Status_OK)
		{
			vTaskDelay(pdMS_TO_TICKS(50));
			continue;
		}
		// Gelen cevap 5 byte ve tam byte hizalıysa BCC doğrulaması yap.
		if(cl_rx_len == 5 && cl_rx_last_bits == 0)
		{
			// İlk 4 byte XOR sonucu, 5. byte'taki BCC değeriyle eşleşmelidir.
			uint8_t bcc_calc = cl_rx_buffer[0]^cl_rx_buffer[1]^cl_rx_buffer[2]^cl_rx_buffer[3];

			// Hesaplanan BCC doğruysa kartı select komutuyla seçme aşamasına geç.
			if(bcc_calc == cl_rx_buffer[4])
			{
				// Cascade Level 1 select komutu oluştur.
				// 0x93 = Collision1, 0x70 = Select, ardından UID + BCC gönderilir.
				uint8_t cl_select_cmd[7] = {0x93, 0x70, cl_rx_buffer[0], cl_rx_buffer[1], cl_rx_buffer[2], cl_rx_buffer[3], cl_rx_buffer[4]};
				uint8_t clsc_tx_len = 7;
				uint8_t clsc_tx_last_bits = 0;
				uint8_t clsc_rx_buffer[3] = {0};
				uint8_t clsc_rx_len = 0;
				uint8_t clsc_rx_last_bits = 0;
				bool clsc_crc_en = true;
				bool clsc_expect_response = true;

				// Select komutunu gönder ve SAK cevabını al.
				 st = RC522_Transceive(cl_select_cmd, clsc_tx_len, clsc_tx_last_bits, clsc_rx_buffer,
						&clsc_rx_len, sizeof(clsc_rx_buffer), &clsc_rx_last_bits, clsc_crc_en, clsc_expect_response);

				 if(st != RC522_Status_OK)
				 {
				 	vTaskDelay(pdMS_TO_TICKS(50));
				 	continue;
				 }
				// SAK cevabı geldiyse ve tam byte hizalıysa UID'nin devam edip etmediğini kontrol et.
				 // Bit 2 (cascade bit) = 1 ise UID bir sonraki cascade level'da devam ediyor demektir.
				if(clsc_rx_len >= 1 && clsc_rx_last_bits == 0)
				{
					if((clsc_rx_buffer[0] & 0x04) != 0)
					{
						// UID tek seviyede tamamlanmadığı için Cascade Level 2 anti-collision komutunu hazırla
						uint8_t cascade_level_2[2] = {0x95, 0x20};
						uint8_t cl2_len = 2;
						uint8_t cl2_tx_last_bits = 0;
						uint8_t cl2_rx_buffer[5] = {0};
						uint8_t cl2_rx_len = 0;
						uint8_t cl2_rx_last_bits = 0;
						bool cl2_crc_en = false;
						bool cl2_expect_response = true;

						// Cascade Level 2 anti-collision cevabını al.
						st = RC522_Transceive(cascade_level_2, cl2_len, cl2_tx_last_bits, cl2_rx_buffer,
								&cl2_rx_len, sizeof(cl2_rx_buffer), &cl2_rx_last_bits, cl2_crc_en, cl2_expect_response);

						if(st != RC522_Status_OK)
						{
							vTaskDelay(pdMS_TO_TICKS(50));
							continue;
						}

						// 5 byte cevap geldiyse BCC doğrulaması yap.
						if(cl2_rx_len == 5 && cl2_rx_last_bits == 0)
						{
							uint8_t bcc2_calc = cl2_rx_buffer[0]^cl2_rx_buffer[1]^cl2_rx_buffer[2]^cl2_rx_buffer[3];
							if(bcc2_calc == cl2_rx_buffer[4])
							{
								// BCC doğruysa Cascade Level 2 select komutunu oluştur ve gönder.
								uint8_t cl2_select_cmd[7] = {0x95, 0x70, cl2_rx_buffer[0], cl2_rx_buffer[1], cl2_rx_buffer[2], cl2_rx_buffer[3], cl2_rx_buffer[4]};
								uint8_t clsc2_tx_len = 7;
								uint8_t clsc2_tx_last_bits = 0;
								uint8_t clsc2_rx_buffer[5] = {0};
								uint8_t clsc2_rx_len = 0;
								uint8_t clsc2_rx_last_bits = 0;
								bool clsc2_crc_en = true;
								bool clsc2_expect_response = true;

								// Select komutunu gönder ve SAK cevabını al.
								st = RC522_Transceive(cl2_select_cmd, clsc2_tx_len, clsc2_tx_last_bits, clsc2_rx_buffer,
										&clsc2_rx_len, sizeof(clsc2_rx_buffer), &clsc2_rx_last_bits, clsc2_crc_en, clsc2_expect_response);

								if(st != RC522_Status_OK)
								{
									vTaskDelay(pdMS_TO_TICKS(50));
									continue;
								};

								// Cascade Level 2 select cevabındaki cascade bitini kontrol et.
								// Eğer bit 2 = 1 ise UID Cascade Level 3'te devam eder.
								if(clsc2_rx_len >= 1 && clsc2_rx_last_bits == 0)
								{
									if((clsc2_rx_buffer[0] & 0x04) != 0)
									{
										// UID hâlâ tamamlanmadığı için Cascade Level 3 anti-collision komutunu hazırla.
										uint8_t cascade_level_3[2] = {0x97, 0x20};
										uint8_t cl3_len = 2;
										uint8_t cl3_tx_last_bits = 0;
										uint8_t cl3_rx_buffer[5] = {0};
										uint8_t cl3_rx_len = 0;
										uint8_t cl3_rx_last_bits = 0;
										bool cl3_crc_en = false;
										bool cl3_expect_response = true;

										// Cascade Level 3 anti-collision cevabını al.
										st = RC522_Transceive(cascade_level_3, cl3_len, cl3_tx_last_bits, cl3_rx_buffer,
												&cl3_rx_len, sizeof(cl3_rx_buffer), &cl3_rx_last_bits, cl3_crc_en, cl3_expect_response);
										if(st != RC522_Status_OK)
										{
											vTaskDelay(pdMS_TO_TICKS(50));
											continue;
										}

										// 5 byte cevap geldiyse BCC doğrulaması yap.
										if(cl3_rx_len == 5 && cl3_rx_last_bits == 0)
										{
											uint8_t bcc3_calc = cl3_rx_buffer[0]^cl3_rx_buffer[1]^cl3_rx_buffer[2]^cl3_rx_buffer[3];

											if(bcc3_calc == cl3_rx_buffer[4])
											{
												// BCC doğrulaması başarılıysa Cascade Level 3 select komutunu oluştur.
												uint8_t cl3_select_cmd[7] = {0x97, 0x70, cl3_rx_buffer[0], cl3_rx_buffer[1], cl3_rx_buffer[2], cl3_rx_buffer[3], cl3_rx_buffer[4]};
												uint8_t clsc3_tx_len = 7;
												uint8_t clsc3_tx_last_bits = 0;
												uint8_t clsc3_rx_buffer[5]= {0};
												uint8_t clsc3_rx_len = 0;
												uint8_t clsc3_rx_last_bits = 0;
												bool clsc3_crc_en = true;
												bool clsc3_expect_response = true;

												// Select komutunu gönder ve SAK cevabını al.
												st = RC522_Transceive(cl3_select_cmd, clsc3_tx_len, clsc3_tx_last_bits, clsc3_rx_buffer,
														&clsc3_rx_len, sizeof(clsc3_rx_buffer), &clsc3_rx_last_bits, clsc3_crc_en, clsc3_expect_response);

												if(st != RC522_Status_OK)
												{
													vTaskDelay(pdMS_TO_TICKS(50));
													continue;
												}

												// Cascade Level 3 select işlemi tamamlandıktan sonra artık UID tamamen elde edilmiştir.
												if(clsc3_rx_len >= 1 && clsc3_rx_last_bits == 0)
												{
													// 10 byte uid toparla döndür
													uint8_t temp_uid3[10] = {cl_rx_buffer[1], cl_rx_buffer[2], cl_rx_buffer[3], cl2_rx_buffer[1], cl2_rx_buffer[2], cl2_rx_buffer[3], cl3_rx_buffer[0], cl3_rx_buffer[1], cl3_rx_buffer[2], cl3_rx_buffer[3]};
													*len = 10;
													memcpy(uid, temp_uid3, sizeof(temp_uid3));

													return RC522_Card_ID_Found;
												}
											}
										}
									}


									else
									{
										// 7 byte uid toparla döndür
										uint8_t temp_uid2[7] = {cl_rx_buffer[1], cl_rx_buffer[2], cl_rx_buffer[3], cl2_rx_buffer[0], cl2_rx_buffer[1], cl2_rx_buffer[2], cl2_rx_buffer[3]};
										*len = 7;
										memcpy(uid, temp_uid2, sizeof(temp_uid2));

										return RC522_Card_ID_Found;
									}
								}
							}
						}
					}

					else
					{
						// 4 byte UID toparla döndür
						uint8_t temp_uid[4] = {cl_rx_buffer[0], cl_rx_buffer[1], cl_rx_buffer[2], cl_rx_buffer[3]};
						*len = 4;
						memcpy(uid, temp_uid, sizeof(temp_uid));
						return RC522_Card_ID_Found;
					}
				}
			}
		}
	}

	// Kart id bulunamadı durumunu dön.
	return RC522_Card_ID_Not_Found;
}



RC522_Status_Type RC522_Authenticate(uint8_t block_addrr, uint8_t auth_command, uint8_t* key, uint8_t* card_id)
{
	RC522_Status_Type st = RC522_Status_Start_Value;

	// Authenticate için mutex boşta ise hemen al , değilse 100ms almak için bekle.
	if(!SPI1_Lock(100)) return RC522_SPI_Comm_Fail;

	for(int retry = 0; retry < 5; retry++)
	{
		st = RC522_CommandReg_CommandBitValue(Idle);   // Önceki işlemi temizle
		if(st != RC522_Status_OK) goto exit;

		st = RC522_StartSend(false);
		if(st != RC522_Status_OK) goto exit;

		st = RC522_FIFO_Flush();  // FIFO'yu temizle
		if(st != RC522_Status_OK) goto exit;

		st = RC522_IRQ_Clear();   // Interrupt bayraklarını temizle
		if(st != RC522_Status_OK) goto exit;

		st = RC522_SetTxLastBits(0);  // Tam byte gidecek.
		if(st != RC522_Status_OK) goto exit;

		uint8_t auth_cmd[12] = {auth_command, block_addrr, key[0], key[1], key[2], key[3],key[4], key[5], card_id[0], card_id[1], card_id[2], card_id[3]};

		st = RC522_Write_FIFO(auth_cmd, sizeof(auth_cmd));
		if(st != RC522_Status_OK) goto exit;

		// Komut bitini authenticate işlemi yap.
	 	st = RC522_CommandReg_CommandBitValue(MFAuthent);
		if(st != RC522_Status_OK) goto exit;

	 	// IRQ bayraklarının yeterli sürede kalkmasını bekle ve yeterli sürede idle_irq bayrağı kalkmadıysa fonksiyondan çık
	 	st = RC522_WaitIRQForMFAuthent();
	    if(st != RC522_Status_OK) goto exit;

	    // Error register'inda herhangi bir hata bayrağı kalkmışmı kontrol et eğer kalkmışsa fonksiyondan çık
	 	uint8_t error_reg_value = 0;
	    st = RC522_ReadReg(Error_Reg, &error_reg_value);
	    if(st != RC522_Status_OK) goto exit;
	 	if(error_reg_value != 0)
	 	{
	 		st = RC522_Auth_Error_Reg_Set;
	 		goto exit;
	 	}

	 	// MFCrypto1ON bayrağı kalkmış mı kontrol et , eğer kalkmış ise authenticate başarılıdır.
		uint8_t mf_crypto1_on_value = 0;
	    st = RC522_ReadReg(Status2_Reg, &mf_crypto1_on_value);
	    if(st != RC522_Status_OK) goto exit;
	    mf_crypto1_on_value &= 0x08;
	 	if(mf_crypto1_on_value != 0x08)
	 	{
	 		st = RC522_Auth_Not_Active;
	 		goto exit;
	 	}

	 	// authenticate akışı tamamlandı gibi göründü ama authentication aktif hale gelmedi
	 	st = RC522_Status_OK;
	 	break;
	}



 	exit:
 	    if(st != RC522_Status_OK)
 	    {
 	        (void)RC522_StartSend(false);
 	        (void)RC522_CommandReg_CommandBitValue(Idle);
 	    }

 	    SPI1_Unlock();
 	    return st;
}

// Yeni kart kişiselleştirme fonksiyonu.
RC522_Status_Type RC522_CardPersonalization(new_card_response_helper* new_card_info, uint8_t* card_id, uint8_t block_addrr_header, uint8_t block_addrr_balance)
{
	// Kartın sektör 1 blok 0'a proje ve kartla ilgili bilgileri girmek için card_header nesnesi oluştur ve bilgileri nesnenin değişkenlerine tanımla.
	RC522_Status_Type st = RC522_Status_Start_Value;

	RC522_Card_Header card_header_init = {0};
	memcpy(card_header_init.magic_number, new_card_info->magic_number, sizeof(new_card_info->magic_number));
	card_header_init.version = new_card_info->version;
	card_header_init.card_type = new_card_info->card_type;
	memcpy(card_header_init.uid, card_id, 4);
	card_header_init.operation_counter = 0;
	card_header_init.expiry_date = new_card_info->expiry_date;
	card_header_init.crc = Helper_Calculate_Crc16((uint8_t*)&card_header_init, 14);

	// Kartın sektör 1 blok 1'e kart bakiyesi ve vize bilgileri girmek için card_balance nesnesi oluştur ve bilgileri nesnenin değişkenlerine tanımla.
	RC522_Card_Balance card_balance_init = {0};
	card_balance_init.balance = new_card_info->balance;
	card_balance_init.operation_counter = 0;
	card_balance_init.max_balance = MAX_ALLOWED_BALANCE;
	card_balance_init.visa_date = new_card_info->visa_date;
	card_balance_init.crc = Helper_Calculate_Crc16((uint8_t*)&card_balance_init, 14);

	// Mutex al, meşgulse 300ms alana kadar bekle
    if(!SPI1_Lock(300)) return RC522_SPI_Comm_Fail;


	// Nesneye bilgiler tanımlandıktan sonra kartın sektör1 blok0'a bilgileri yaz, işlemde hata çıkarsa hata durumunu dön.
	st = RC522_MIFARE_Write(block_addrr_header, (uint8_t*)&card_header_init, sizeof(card_header_init));
	if(st != RC522_Status_OK) goto exit;

	// Nesneye bilgiler tanımlandıktan sonra kartın sektör1 blok1'e bilgileri yaz, işlemde hata çıkarsa hata durumunu dön.
	st = RC522_MIFARE_Write(block_addrr_balance, (uint8_t*)&card_balance_init, sizeof(card_balance_init));
	if(st != RC522_Status_OK) goto exit;

	st = RC522_Status_OK;

	exit:
		SPI1_Unlock();
		return st;
	/*
	// Karttaki KEYA, KEYB ve AccessBits'e erişmek için struct nesnesi ve geçici dizi oluşturma.
	card_security card_security_init;
	uint8_t security_block_temp_buff[16] = {0};

	// Burada keyA, keyB ve accessbits değerlerine bakılacak.
	RC522_MIFARE_READ(card_sector1_block3, security_block_temp_buff);
	memcpy(&card_security_init, security_block_temp_buff, 16);

	// KEYA, KEYB ve AccessBits için yeni değerlerini atayıp karttaki ilgili(trailer block = sector 1 block 3) bloğa yazma. (KEYB write/read, KEYA read.)
	const uint8_t keyA_private_value_temp[6] = KEYA_Private_Value;
	const uint8_t access_bits_private_value_temp[3] = Access_Bits_Private_Value;
	card_security_init.general_purpose = General_Purpose_Default_Value;
	const uint8_t keyB_private_value_temp[6] = KEYB_Private_Value;
	memcpy(card_security_init.key_A, keyA_private_value_temp, 6);
	memcpy(card_security_init.access_bits,access_bits_private_value_temp ,3);
	memcpy(card_security_init.key_B, keyB_private_value_temp, 6);

	RC522_MIFARE_Write(card_sector1_block3, (uint8_t*)&card_security_init , sizeof(card_security_init));   // Yeni değerlerin trailer block'a yazılması.*/
}


RC522_Status_Type RC522_MIFARE_READ(uint8_t block_addrr, uint8_t* read_buffer)
{
	// Mutex al, kullanıyorsa almak için 100ms bekle.
	if(!SPI1_Lock(300)) return RC522_SPI_Comm_Fail;

	RC522_Status_Type st = RC522_Status_Start_Value;

	uint8_t read_cmd[2] = {MIFARE_Read, block_addrr};
	uint8_t read_cmd_len = 2;
	uint8_t read_cmd_last_bits = 0;
	uint8_t read_cmd_response[18] = {0};
	uint8_t read_cmd_response_len = 0;
	uint8_t read_cmd_response_last_bits = 0;
	bool read_cmd_crc_en = true;
	bool read_cmd_expect_response = true;
	st = RC522_Transceive(read_cmd, read_cmd_len, read_cmd_last_bits, read_cmd_response,
			&read_cmd_response_len, sizeof(read_cmd_response),&read_cmd_response_last_bits,
			read_cmd_crc_en, read_cmd_expect_response);

	if(st != RC522_Status_OK) goto exit;

	if(read_cmd_response_len >= 16 && read_cmd_response_last_bits == 0)
	{
		uint8_t error_reg_value_after_read = 0;
		st = RC522_ReadReg(Error_Reg, &error_reg_value_after_read);
		if(st != RC522_Status_OK) goto exit;
		if(error_reg_value_after_read != 0)
		{
			st = RC522_Mifare_Read_Error_Reg_Set;
			goto exit;
		}

		memcpy(read_buffer, read_cmd_response, 16);
		st = RC522_Status_OK;
		goto exit;
	}

	st = RC522_Mifare_Read_Invalid_Response;

	exit:
		SPI1_Unlock();   //	Mutex ver. (serbest bırak)
		return st;       // Durum sonucu dön.
}

RC522_Status_Type RC522_MIFARE_Write(uint8_t block_addrr, uint8_t* data, uint8_t data_len)
{
    RC522_Status_Type st = RC522_Status_Start_Value;

    if(!SPI1_Lock(300)) return RC522_SPI_Comm_Fail;

    uint8_t write_cmd[2] = {MIFARE_Write, block_addrr};
    uint8_t write_cmd_len = 2;
    uint8_t write_cmd_last_bits = 0;
    uint8_t write_cmd_response[1] = {0};
    uint8_t write_cmd_response_len = 0;
    uint8_t write_cmd_response_last_bits = 0;
    bool write_cmd_crc_en = true;
    bool write_cmd_expect_response = true;

    st = RC522_Transceive(write_cmd,
                          write_cmd_len,
                          write_cmd_last_bits,
                          write_cmd_response,
                          &write_cmd_response_len,
                          sizeof(write_cmd_response),
                          &write_cmd_response_last_bits,
                          write_cmd_crc_en,
                          write_cmd_expect_response);

    if(st != RC522_Status_OK)
        goto exit;

    uint8_t write_cmd_ack = write_cmd_response[0] & 0x0F;

    if(!(write_cmd_ack == 0x0A && write_cmd_response_last_bits == 4))
    {
        st = RC522_Mifare_Write_CMD_ACK_Fail;
        goto exit;
    }

    uint8_t error_reg_value_after_write_cmd = 0;
    st = RC522_ReadReg(Error_Reg, &error_reg_value_after_write_cmd);
    if(st != RC522_Status_OK)
        goto exit;

    if(error_reg_value_after_write_cmd != 0)
    {
        st = RC522_Mifare_Write_Error_Reg_Set;
        goto exit;
    }

    uint8_t write_data[16] = {0};
    memcpy(write_data, data, data_len);

    uint8_t write_data_response[1] = {0};
    uint8_t write_data_response_len = 0;
    uint8_t write_data_response_last_bits = 0;

    st = RC522_Transceive(write_data,
                          16,
                          0,
                          write_data_response,
                          &write_data_response_len,
                          sizeof(write_data_response),
                          &write_data_response_last_bits,
                          true,
                          true);

    if(st != RC522_Status_OK)
        goto exit;

    uint8_t write_data_ack = write_data_response[0] & 0x0F;

    if(!(write_data_ack == 0x0A && write_data_response_last_bits == 4))
    {
        st = RC522_Mifare_Write_Data_ACK_Fail;
        goto exit;
    }

    uint8_t error_reg_value_after_write_data = 0;
    st = RC522_ReadReg(Error_Reg, &error_reg_value_after_write_data);
    if(st != RC522_Status_OK)
        goto exit;

    if(error_reg_value_after_write_data != 0)
    {
        st = RC522_Mifare_Write_Error_Reg_Set;
        goto exit;
    }

    st = RC522_Status_OK;

    exit:
    	SPI1_Unlock();
    	return st;
}

RC522_Status_Type RC522_Get_Card_Info(uint8_t* card_id, RC522_Card_Header *card_header_temp, RC522_Card_Balance* card_balance_temp)
{
	if(!SPI1_Lock(100)) return RC522_SPI_Comm_Fail;

	RC522_Status_Type st = RC522_Status_Start_Value;

	// Okunan kartın şuanki verilerini almak için geçici diziler
	uint8_t card_header_temp_raw[16] = {0};
	uint8_t card_balance_temp_raw[16] = {0};

	// Okunan kartın bloklarındaki verileri al.
	st = RC522_MIFARE_READ(card_sector1_block0, card_header_temp_raw);
	if(st != RC522_Status_OK) goto exit;
	st = RC522_MIFARE_READ(card_sector1_block1, card_balance_temp_raw);
	if(st != RC522_Status_OK) goto exit;

	// Geçici dizilere alınan verileri struct'lara kopyala
	memcpy(card_header_temp, card_header_temp_raw, 16);
	memcpy(card_balance_temp, card_balance_temp_raw, 16);

	// Eğer okunan ID ile kart id'si uyuşmuyorsa işlem başarısız dön.
	if(memcmp(card_header_temp->uid, card_id, sizeof(card_header_temp->uid)) != 0)
	{
		st = RC522_Card_ID_Mismatch;
		goto exit;
	}


	// CRC(bütünlük) yolda bozulmuş olabilir, tekrar hesaplayıp eşleşmiyorsa başarısız dön.
	uint16_t header_crc_temp = Helper_Calculate_Crc16((uint8_t*)card_header_temp, 14);
	uint16_t balance_crc_temp = Helper_Calculate_Crc16((uint8_t*)card_balance_temp, 14);
	if(header_crc_temp != card_header_temp->crc || balance_crc_temp != card_balance_temp->crc)
	{
		st = RC522_Card_Data_CRC_Fail;
		goto exit;
	}

	st = RC522_Status_OK;

	exit:
		SPI1_Unlock();
		return st;
}



RC522_Status_Type RC522_Passanger_Card_Balance_Transaction(RC522_Card_Header* card_header_temp,
		RC522_Card_Balance *card_balance_temp,
		card_balance_info_helper *card_balance_info_helper_temp,
		RTC_DS321_Time* time_temp)
{
	RC522_Status_Type st;
	// Son okunan kart id ve son okuma zamanı tutucak değişkenler.
	static uint8_t last_id[4]= {0};
	static TickType_t last_read_time = 0;

	// Aynı kart tekrar gelirse kontrol bayrakları.
	bool elapsed_time_flag = false;
	bool same_card_id = false;

	// Eğer aynı kart belirlenen zaman (5dk) içinde tekrar okutulmaya çalışırsa bayrak kaldır
	elapsed_time_flag = Helper_GetElapsedTime(last_read_time, Required_Time);
	if(memcmp(card_header_temp->uid, last_id, sizeof(card_header_temp->uid)) == 0 && elapsed_time_flag == false)
	{
		same_card_id = true;
	}

	else
	{
		same_card_id = false;
	}


	// Şuanki zamanı kart bloğundaki zaman gibi formatla ve ondan sonra son kullanma tarihi ve vize tarihi ile karşılaştır, eğer geçmiş ise bayrak kaldır.
	uint16_t current_date = Helper_Date_Packing(time_temp->year, time_temp->month, time_temp->day_number);
	bool is_expired   = (current_date > card_header_temp->expiry_date);
	bool visa_expired = (current_date > card_balance_temp->visa_date);

	// Eğer okunan kart tipi tam kart tipi ise veya herhangi tipteki bir kart tekrar okutulmuş ise tam ücret tarifesi uygula.
	// Diğer else if şartları da okutulan kart tipine göre bakiye düşümü yapar.
	if(card_header_temp->card_type == Full_Fare_Card || same_card_id == true)
	{
		// Son kullanma ve vize tarihini kontrol et. Kart bakiyesini, işlem sayacını ve crc güncelleyip bloğa tekrar yaz
		if(is_expired) return Is_Expired;
		if(visa_expired) return Visa_Expired;
		if(card_balance_temp->balance < Full_Fare_Card_Fare) return Balance_Insufficient;
		card_balance_temp->balance -= Full_Fare_Card_Fare;
		card_balance_temp->operation_counter += 1;
		card_balance_temp->crc = Helper_Calculate_Crc16((uint8_t*)card_balance_temp, 14);
		card_header_temp->operation_counter +=1;
		card_header_temp->crc = Helper_Calculate_Crc16((uint8_t*)card_header_temp, 14);

		// TFT'ye gönderilecek alanlar.
		card_balance_info_helper_temp->balance = card_balance_temp->balance;
		card_balance_info_helper_temp->fare = Full_Fare_Card_Fare;

		st = RC522_MIFARE_Write(card_sector1_block0, (uint8_t*)card_header_temp, sizeof(*card_header_temp));
		if(st != RC522_Status_OK) return st;
		st= RC522_MIFARE_Write(card_sector1_block1, (uint8_t*)card_balance_temp, sizeof(*card_balance_temp));
		if(st != RC522_Status_OK) return st;

		memcpy(last_id, card_header_temp->uid, 4);  // Son ID'yi tut
		last_read_time = xTaskGetTickCount();
		return Process_Successfull;
	}

	else if(card_header_temp->card_type == Student_Card)
	{
		// Son kullanma ve vize tarihini kontrol et. Kart bakiyesini, işlem sayacını ve crc güncelleyip bloğa tekrar yaz
		if(is_expired) return Is_Expired;
		if(visa_expired) return Visa_Expired;
		if(card_balance_temp->balance < Student_Card_Fare) return Balance_Insufficient;
		card_balance_temp->balance -= Student_Card_Fare;
		card_balance_temp->operation_counter += 1;
		card_balance_temp->crc = Helper_Calculate_Crc16((uint8_t*)card_balance_temp, 14);
		card_header_temp->operation_counter +=1;
		card_header_temp->crc = Helper_Calculate_Crc16((uint8_t*)card_header_temp, 14);

		// TFT'ye gönderilecek alanlar.
		card_balance_info_helper_temp->balance = card_balance_temp->balance;
		card_balance_info_helper_temp->fare = Student_Card_Fare;

		st = RC522_MIFARE_Write(card_sector1_block0, (uint8_t*)card_header_temp, sizeof(*card_header_temp	));
		if(st != RC522_Status_OK) return st;
		st = RC522_MIFARE_Write(card_sector1_block1, (uint8_t*)card_balance_temp, sizeof(*card_balance_temp));
		if(st != RC522_Status_OK) return st;

		memcpy(last_id, card_header_temp->uid, 4);  // Son ID'yi tut
		last_read_time = xTaskGetTickCount();
		return Process_Successfull;
	}

	else if(card_header_temp->card_type == Teacher_Card)
	{
		// Son kullanma ve vize tarihini kontrol et. Kart bakiyesini, işlem sayacını ve crc güncelleyip bloğa tekrar yaz
		if(is_expired) return Is_Expired;
		if(visa_expired) return Visa_Expired;
		if(card_balance_temp->balance < Teacher_Card_Fare) return Balance_Insufficient;
		card_balance_temp->balance -= Teacher_Card_Fare;
		card_balance_temp->operation_counter += 1;
		card_balance_temp->crc = Helper_Calculate_Crc16((uint8_t*)card_balance_temp, 14);
		card_header_temp->operation_counter +=1;
		card_header_temp->crc = Helper_Calculate_Crc16((uint8_t*)card_header_temp, 14);

		// TFT'ye gönderilecek alanlar.
		card_balance_info_helper_temp->balance = card_balance_temp->balance;
		card_balance_info_helper_temp->fare = Teacher_Card_Fare;

		st = RC522_MIFARE_Write(card_sector1_block0, (uint8_t*)card_header_temp, sizeof(*card_header_temp));
		if(st != RC522_Status_OK) return st;
		st = RC522_MIFARE_Write(card_sector1_block1, (uint8_t*)card_balance_temp, sizeof(*card_balance_temp));
		if(st != RC522_Status_OK) return st;

		memcpy(last_id, card_header_temp->uid, 4);  // Son ID'yi tut
		last_read_time = xTaskGetTickCount();
		return Process_Successfull;
	}

	else if(card_header_temp->card_type == Senior_Citizen_Card)
	{
		// Son kullanma ve vize tarihini kontrol et. Kart bakiyesini, işlem sayacını ve crc güncelleyip bloğa tekrar yaz
		if(is_expired) return Is_Expired;
		if(visa_expired) return Visa_Expired;
		if(card_balance_temp->balance < Senior_Citizen_Card_Fare) return Balance_Insufficient;
		card_balance_temp->balance -= Senior_Citizen_Card_Fare;
		card_balance_temp->operation_counter += 1;
		card_balance_temp->crc = Helper_Calculate_Crc16((uint8_t*)card_balance_temp, 14);
		card_header_temp->operation_counter +=1;
		card_header_temp->crc = Helper_Calculate_Crc16((uint8_t*)card_header_temp, 14);

		// TFT'ye gönderilecek alanlar.
		card_balance_info_helper_temp->balance = card_balance_temp->balance;
		card_balance_info_helper_temp->fare = Senior_Citizen_Card_Fare;

		st = RC522_MIFARE_Write(card_sector1_block0, (uint8_t*)card_header_temp, sizeof(*card_header_temp));
		if(st != RC522_Status_OK) return st;
		st = RC522_MIFARE_Write(card_sector1_block1, (uint8_t*)card_balance_temp, sizeof(*card_balance_temp));
		if(st != RC522_Status_OK) return st;

		memcpy(last_id, card_header_temp->uid, 4);  // Son ID'yi tut
		last_read_time = xTaskGetTickCount();
		return Process_Successfull;
	}
	else if(card_header_temp->card_type == Disabled_Person_Card)
	{
		// Son kullanma ve vize tarihini kontrol et. Kart bakiyesini, işlem sayacını ve crc güncelleyip bloğa tekrar yaz
		if(is_expired) return Is_Expired;
		if(visa_expired) return Visa_Expired;
		if(card_balance_temp->balance < Disabled_Person_Card_Fare) return Balance_Insufficient;
		card_balance_temp->balance -= Disabled_Person_Card_Fare;
		card_balance_temp->operation_counter += 1;
		card_balance_temp->crc = Helper_Calculate_Crc16((uint8_t*)card_balance_temp, 14);
		card_header_temp->operation_counter +=1;
		card_header_temp->crc = Helper_Calculate_Crc16((uint8_t*)card_header_temp, 14);

		// TFT'ye gönderilecek alanlar.
		card_balance_info_helper_temp->balance = card_balance_temp->balance;
		card_balance_info_helper_temp->fare = Disabled_Person_Card_Fare;

		st = RC522_MIFARE_Write(card_sector1_block0, (uint8_t*)card_header_temp, sizeof(*card_header_temp));
		if(st != RC522_Status_OK) return st;
		st = RC522_MIFARE_Write(card_sector1_block1, (uint8_t*)card_balance_temp, sizeof(*card_balance_temp));
		if(st != RC522_Status_OK) return st;

		memcpy(last_id, card_header_temp->uid, 4);  // Son ID'yi tut
		last_read_time = xTaskGetTickCount();
		return Process_Successfull;
	}

	else
	{
		return Process_NotSuccessfull;
	}
}

// Yeni bakiye yükleme isteği olursa bakiye güncelleme.
RC522_Status_Type RC522_Balance_Top_Up(RC522_Card_Header* card_header_temp, RC522_Card_Balance* card_balance_temp, uint32_t new_balance)
{
	if(new_balance == 0)
	{
		return Process_NotSuccessfull;
	}


	if(!SPI1_Lock(300)) return RC522_SPI_Comm_Fail;

	RC522_Status_Type st = RC522_Status_Start_Value;


	card_header_temp->operation_counter += 1;
	card_header_temp->crc = Helper_Calculate_Crc16((uint8_t*)card_header_temp, 14);

	card_balance_temp->balance += new_balance;
	card_balance_temp->crc = Helper_Calculate_Crc16((uint8_t*)card_balance_temp, 14);

	st = RC522_MIFARE_Write(card_sector1_block0, (uint8_t*)card_header_temp, sizeof(*card_header_temp));
	if(st != RC522_Status_OK) goto exit;

	st = RC522_MIFARE_Write(card_sector1_block1, (uint8_t*)card_balance_temp, sizeof(*card_balance_temp));
	if(st != RC522_Status_OK) goto exit;

	st = Balance_Upload_Successfull;

	exit:
		SPI1_Unlock();
		return st;


}


// Crypto durdurma işlemini gerçekleştiren fonksiyon.
RC522_Status_Type RC522_StopCrypto(void)
{
	if(!SPI1_Lock(100)) return RC522_SPI_Comm_Fail;

	RC522_Status_Type st = RC522_Status_Start_Value;

	uint8_t mf_crypto1_on_value = 0;

	st = RC522_ReadReg(Status2_Reg, &mf_crypto1_on_value);
	if(st != RC522_Status_OK) goto exit;
	mf_crypto1_on_value &= ~(1U << 3);
	st = RC522_WriteReg(Status2_Reg, mf_crypto1_on_value);
	if(st != RC522_Status_OK) goto exit;

	st = RC522_Status_OK;

	exit:
		SPI1_Unlock();
		return st;
}

// Kartı HALT durumuna alma işlemini gerçekleştiren fonksiyon.
RC522_Status_Type RC522_HALT_Card()
{
	// SPI kullanmak için Mutex al ve karta HALT komutu gönder.
	if(!SPI1_Lock(100)) return RC522_SPI_Comm_Fail;

	uint8_t halt_cmd[2]= MIFARE_Halt;
	uint8_t halt_cmd_len = 2;
	uint8_t halt_cmd_last_bits = 0;
	uint8_t halt_cmd_response[1] = {0};
	uint8_t halt_cmd_response_len = 0;
	uint8_t halt_cmd_response_last_bits = 0;
	bool halt_cmd_crc_en = true;
	bool halt_cmd_expect_response = false;

	RC522_Status_Type st = RC522_Transceive(halt_cmd, halt_cmd_len, halt_cmd_last_bits, halt_cmd_response,
			&halt_cmd_response_len, sizeof(halt_cmd_response), &halt_cmd_response_last_bits, halt_cmd_crc_en, halt_cmd_expect_response);

	if(st != RC522_Status_OK) goto exit;

	st= RC522_Status_OK;

	exit:
		SPI1_Unlock();
		return st;
}


// RC522 ile veri alışverişini gerçekleştiren fonksiyon.
RC522_Status_Type RC522_Transceive(uint8_t* transceive_data, uint8_t txLen, uint8_t tx_last_bits, uint8_t* rxBuffer, uint8_t* rxLen,
		uint8_t rxMaxLen, uint8_t* rxLastBits, bool crc_enable, bool expect_response)
{
	RC522_Status_Type st = RC522_Status_Start_Value;

	// Mutex al, kullanıyorsa almak için 100ms bekle.
	if(!SPI1_Lock(100)) return RC522_SPI_Comm_Fail;

	st = RC522_StartSend(0);
	if(st != RC522_Status_OK) goto exit;

	st = RC522_CommandReg_CommandBitValue(Idle);
	if(st != RC522_Status_OK) goto exit;

	// FIFO ve bayrakları temizle
	st = RC522_FIFO_Flush();
	if(st != RC522_Status_OK) goto exit;
	st = RC522_IRQ_Clear();
	if(st != RC522_Status_OK) goto exit;

	if(crc_enable)
	{
		st = RC522_TxCRCEnDis(1);
		if(st != RC522_Status_OK) goto exit;
	}

	else
	{
		st = RC522_TxCRCEnDis(0);
		if(st != RC522_Status_OK) goto exit;
	}

	// Dışarıdan gelen Tx last bits'i (0bit veya 7bit) gönder, Komutu gönderme işlemi yap ve başlat.
	st = RC522_SetTxLastBits(tx_last_bits);
	if(st != RC522_Status_OK) goto exit;

	// Gönderilecek veriyi FIFO'ya yaz.
	st = RC522_Write_FIFO(transceive_data, txLen);
	if(st != RC522_Status_OK) goto exit;

	st = RC522_CommandReg_CommandBitValue(Transceive);
	if(st != RC522_Status_OK) goto exit;

	st = RC522_StartSend(true);
	if(st != RC522_Status_OK) goto exit;



	//Cevap bekleniyorsa, durum kontrolü yap eğer hata varsa gönderme işlemini durdur ve boşa al
	if(expect_response)
	{
		st = RC522_WaitIRQForTransceive();

		if(st != RC522_Status_OK)
		{
		    RC522_StartSend(0);
		    RC522_CommandReg_CommandBitValue(Idle);
		    RC522_TxCRCEnDis(0);

		    goto exit;
		}
	}


	if(crc_enable)
	{
		// CRC kapat.
		st = RC522_TxCRCEnDis(0);
		if(st != RC522_Status_OK) goto exit;
	}


	if(expect_response)
	{
		// Alınacak verinin uzunluğunu al ve dışarıdan gelen değişkene ata, Veriyi oku ve dışarıdan gelen diziye yaz.
		uint8_t fifo_level_value = 0;

		st = RC522_ReadReg(FIFO_Level_Reg, &fifo_level_value);

		if(st != RC522_Status_OK) goto exit;

		fifo_level_value &= 0x7F;

		if(fifo_level_value == 0)
		{
		    RC522_StartSend(0);
		    RC522_CommandReg_CommandBitValue(Idle);
		    st = RC522_No_Card_Present;
		    goto exit;
		}


		if(fifo_level_value > rxMaxLen)
		{
		    RC522_StartSend(0);
		    RC522_CommandReg_CommandBitValue(Idle);
            st = RC522_Wait_Error;
            goto exit;
		}


		*rxLen = fifo_level_value;
		st = RC522_Read_FIFO(rxBuffer, *rxLen);
		if(st != RC522_Status_OK) goto exit;

		// rx Last bits değerini al
		st = RC522_ReadRxLastBits(rxLastBits);
		if(st != RC522_Status_OK) goto exit;

	}

	// İşlemi bitir ve tamamlandı olarak dön
	st = RC522_StartSend(0);
	if(st != RC522_Status_OK) goto exit;
	st = RC522_CommandReg_CommandBitValue(Idle);
	if(st != RC522_Status_OK) goto exit;

	st= RC522_Status_OK;

	exit:
		SPI1_Unlock();
		return st;
 }

// RC522 ile veri alışverişi başlatma işlemini gerçekleştiren fonksiyon.
RC522_Status_Type RC522_StartSend(bool start_sent_value)
{
	RC522_Status_Type st;
	uint8_t bit_framing_reg_value = 0;
	st = RC522_ReadReg(Bit_Framing_Reg, &bit_framing_reg_value);
	if(st != RC522_Status_OK) return st;

	if(start_sent_value)
	{
		// Transceive komutu ile birlikte veri iletimini başlat.
		bit_framing_reg_value |= (1U << 7);
		st = RC522_WriteReg(Bit_Framing_Reg, bit_framing_reg_value);
		if(st != RC522_Status_OK) return st;

	}

	else
	{
		// Start send biti temizleme ve veri iletimini durdur.
		bit_framing_reg_value &= ~(1U << 7);
		st = RC522_WriteReg(Bit_Framing_Reg, bit_framing_reg_value);
		if(st != RC522_Status_OK) return st;

	}

	return RC522_Status_OK;
}

// RC522 TxLastBits değeri belirleme.
RC522_Status_Type RC522_SetTxLastBits(uint8_t tx_last_bits)
{
	uint8_t bit_framing_reg_value = 0;
	RC522_Status_Type st;
	st = RC522_ReadReg(Bit_Framing_Reg, &bit_framing_reg_value);
	if(st != RC522_Status_OK) return st;
	// Son byte tam byte gönderilecek.
	if(tx_last_bits == 0)
	{
		bit_framing_reg_value &= ~(7U << 0);
		st = RC522_WriteReg(Bit_Framing_Reg, bit_framing_reg_value);
		if(st != RC522_Status_OK) return st;
	}

	// Son byte 7 bit gönderilecek.
	if(tx_last_bits == 7)
	{
		bit_framing_reg_value &= ~(7U << 0);
		bit_framing_reg_value |= (7U << 0);
		st = RC522_WriteReg(Bit_Framing_Reg, bit_framing_reg_value);
		if(st != RC522_Status_OK) return st;
	}

	return RC522_Status_OK;

}

// RC522 FIFO temizleme.
RC522_Status_Type RC522_FIFO_Flush(void)
{
	// FIFO’yu flush et (temizle)
	RC522_Status_Type st;
	uint8_t fifo_level_reg_value = 0;
	st = RC522_ReadReg(FIFO_Level_Reg, &fifo_level_reg_value);
	if(st != RC522_Status_OK) return st;
	fifo_level_reg_value |= (1U << 7);
	st = RC522_WriteReg(FIFO_Level_Reg, fifo_level_reg_value);
	if(st != RC522_Status_OK) return st;

	return RC522_Status_OK;
}

// RC522 IRQ bayraklarını temizleme.
RC522_Status_Type RC522_IRQ_Clear(void)
{
	// IRQ bayraklarını temizle
	RC522_Status_Type st;

	uint8_t com_irq_reg_value = 0x7F;
	st = RC522_WriteReg(Com_Irq_Reg, com_irq_reg_value);
	if(st != RC522_Status_OK) return st;


	// IRQ bayraklarını temizle
	uint8_t div_irq_reg_value = 0x14;
	st = RC522_WriteReg(Div_Irq_Reg, div_irq_reg_value);
	if(st != RC522_Status_OK) return st;


	// İşlem başarılı durumunu dön.
	return RC522_Status_OK;
}

// RC522 RxLastBits değeri okuma.
RC522_Status_Type RC522_ReadRxLastBits(uint8_t* rx_last_bits)
{
	// Son byte'ı oku ve başarılı durumu döndür, hata oluştuysa hata durumunu dön.
	RC522_Status_Type st;
	uint8_t rx_last_bits_value = 0;
	st = RC522_ReadReg(Control_Reg, &rx_last_bits_value);
	if(st != RC522_Status_OK) return st;
	rx_last_bits_value &= 0x07;
	*rx_last_bits = rx_last_bits_value;
	return RC522_Status_OK;
}

// Tx_Mode_Reg CRC etkinleştir veya devre dışı bırak.
RC522_Status_Type RC522_TxCRCEnDis(uint8_t value)
{
	RC522_Status_Type st;
	// value = 1 ise hem gönderme (TX) hem alma (RX) tarafında CRC bitini aktif et.
	if(value == 1)
	{
		// Tx_Mode_Reg register'ını oku eğer hata varsa hata durumunu dön.
		uint8_t tx_crc_en_value = 0;
		st = RC522_ReadReg(Tx_Mode_Reg, &tx_crc_en_value);
		if(st != RC522_Status_OK) return st;

		// Bit 7'yi 1 yap -> TX CRC aktif
		tx_crc_en_value |= (1U << 7);

		// Güncellenmiş değeri Tx_Mode_Reg register'ına yaz.
		st = RC522_WriteReg(Tx_Mode_Reg, tx_crc_en_value);
		if(st != RC522_Status_OK) return st;
	}

	// value = 0 ise hem gönderme (TX) hem alma (RX) tarafında CRC bitini kapat
	if(value == 0)
	{
		// Tx_Mode_Reg register'ını oku
		uint8_t tx_crc_en_value = 0;
		st = RC522_ReadReg(Tx_Mode_Reg, &tx_crc_en_value);
		if(st != RC522_Status_OK) return st;

		// Bit 7'yi 0 yap -> TX CRC pasif
		tx_crc_en_value &= ~(1U << 7);

		// Güncellenmiş değeri Tx_Mode_Reg register'ına yaz
		st = RC522_WriteReg(Tx_Mode_Reg, tx_crc_en_value);
		if(st != RC522_Status_OK) return st;

		// Rx_Mode_Reg register'ını oku
		uint8_t rx_crc_en_value = 0;
		st = RC522_ReadReg(Rx_Mode_Reg, &rx_crc_en_value);
		if(st != RC522_Status_OK) return st;

		// Bit 7'yi 0 yap -> RX CRC pasif
		rx_crc_en_value &= ~(1U << 7);

		// Güncellenmiş değeri Rx_Mode_Reg register'ına yaz
		st = RC522_WriteReg(Rx_Mode_Reg, rx_crc_en_value);
		if(st != RC522_Status_OK) return st;
	}

	// Tüm işlemler başarıyla tamamlandı
	return RC522_Status_OK;
}

// RC522 CommandReg ile yapılacak komut işlemi değerini gönderme.
RC522_Status_Type RC522_CommandReg_CommandBitValue(uint8_t cmd)
{
	// Dışarıdan gelecek komuta göre , Command biti temizle ve komutu yaz.
	RC522_Status_Type st;
	uint8_t command_reg_value = 0;
	st = RC522_ReadReg(Command_Reg, &command_reg_value);
	if(st != RC522_Status_OK) return st;
	command_reg_value &= ~(15U << 0);
	command_reg_value = (cmd << 0);
	st = RC522_WriteReg(Command_Reg, command_reg_value);
	if(st != RC522_Status_OK) return st;

	// Tüm işlemler başarıyla tamamlandı
	return RC522_Status_OK;
}

// RC522 FIFO'ya veri yazma
RC522_Status_Type RC522_Write_FIFO(uint8_t* data, uint8_t len)
{
	RC522_Status_Type st;
	// Gönderilecek veriyi FIFO'ya yaz.
	for(int i = 0 ; i < len; i++)
	{
		st = RC522_WriteReg(FIFO_Data_Reg, data[i]);
		if(st != RC522_Status_OK) return st;
	}

	// Tüm işlemler başarıyla tamamlandı
	return RC522_Status_OK;
}

// RC522 FIFO'dan veri okuma
RC522_Status_Type RC522_Read_FIFO(uint8_t* buffer, uint8_t len)
{
	RC522_Status_Type st;

	// Alınacak verileri dışarıdan gelen diziye yaz.
	for(int i = 0; i < len; i++)
	{
		st = RC522_ReadReg(FIFO_Data_Reg, &buffer[i]);

		if(st != RC522_Status_OK) return st;

	}

	// Tüm işlemler başarıyla tamamlandı
	return RC522_Status_OK;
}


// RC522 Transceive işleminde IRQ bayraklarını kontrol etme.
RC522_Status_Type RC522_WaitIRQForTransceive(void)
{
	RC522_Status_Type st = RC522_Status_Start_Value;
    uint32_t start = TIM6_Get_Millis();

	// Belirli bir süre bayrak okumak için bekle.
    while((TIM6_Get_Millis() - start) < 200)
	{
		// Bayrak bitlerini maskele
		uint8_t com_irq_value = 0;
		st = RC522_ReadReg(Com_Irq_Reg, &com_irq_value);

		if(st != RC522_Status_OK) return st;

		uint8_t rx_irq    = (com_irq_value & 0x20);
		uint8_t idle_irq  = (com_irq_value & 0x10);
		uint8_t err_irq   = (com_irq_value & 0x02);
		uint8_t timer_irq = (com_irq_value & 0x01);


		// Hata varsa durum bildir.
		if(err_irq)
		{
			uint8_t error_code = 0;
			st = RC522_ReadReg(Error_Reg, &error_code);
			if(st != RC522_Status_OK)
			{
				RC522_IRQ_Clear();
				return st;
			}
			if(error_code != 0)
			{
				RC522_IRQ_Clear();
				return RC522_Wait_Error;
			}
		}


		// Eğer boşa düştüyse ve FIFO doluysa başarılı döndür.
		if(rx_irq || idle_irq)
		{
			uint8_t fifo_level_reg_value = 0;
			st = RC522_ReadReg(FIFO_Level_Reg, &fifo_level_reg_value);  // bit[6:0]
			fifo_level_reg_value &= 0x7F;
			if(fifo_level_reg_value > 0)
			{
				RC522_IRQ_Clear();
				return RC522_Status_OK;
			}
			//else return RC522_Wait_Timeout;
		}



		if(timer_irq)
		{
			uint8_t fifo_level_reg_value = 0;
			st = RC522_ReadReg(FIFO_Level_Reg, &fifo_level_reg_value);  // bit[6:0]
			fifo_level_reg_value &= 0x7F;
			if(fifo_level_reg_value > 0)
			{
				RC522_IRQ_Clear();
				return RC522_Status_OK;
			}
			else
			{
				RC522_IRQ_Clear();
				return RC522_Wait_Timeout;  // Veriln zaman dolduysa timeout olmuştur durum bildir.

			}
		}
	}

	RC522_IRQ_Clear();
	return RC522_Wait_Timeout;
}

// Authenticate işleminde bayrak kontrol fonksiyonu.
RC522_Status_Type RC522_WaitIRQForMFAuthent(void)
{
	RC522_Status_Type st = RC522_Status_Start_Value;
    uint32_t start = TIM6_Get_Millis();

	// Belirli bir süre bayrak okumak için bekle.
    	while((TIM6_Get_Millis() - start) < 200)
		{

			// Bayrak bitlerini maskele
			uint8_t com_irq_value = 0;
			st =  RC522_ReadReg(Com_Irq_Reg, &com_irq_value);
			if(st != RC522_Status_OK) return st;

			uint8_t idle_irq  = (com_irq_value & (1U << 4));
			uint8_t err_irq   = (com_irq_value & (1U << 1));
			uint8_t timer_irq = (com_irq_value & (1U << 0));

			if(timer_irq) return RC522_Wait_Timeout;  // Veriln zaman dolduysa timeout olmuştur durum bildir.

			// Hata varsa durum bildir.
			if(err_irq)
			{
				uint8_t error_code = 0;
				st = RC522_ReadReg(Error_Reg, &error_code);
				if(st != RC522_Status_OK) return st;
				if(error_code != 0) return RC522_Wait_Error;
			}

			// Eğer boşa düştüyse ve FIFO doluysa başarılı döndür.
			if(idle_irq) return RC522_Status_OK;
			//vTaskDelay(pdMS_TO_TICKS(1));

		}

		return RC522_Wait_Timeout;
}


// RC522 Register'lara veri yazma.
RC522_Status_Type RC522_WriteReg(uint8_t reg, uint8_t data)
{
	RC522_Status_Type rc522_st = RC522_Status_Start_Value;
	SPI_Status_t st = SPI_STATUS_START_VALUE;

    if(!SPI1_Lock(100)) return RC522_Reg_Write_Fail;

	uint8_t address = (((reg << 1) & 0x7E));   // Yazma işlemi için 0 ve 7.biti maskele (ayır) ve register adresini bit1...6 arasına yaz.

	GPIOA->BSRR = GPIO_BSRR_BR4;          // CS LOW

	st = SPI_Transfer(address, NULL);           // Adresi gönder, gelen değeri kullanma. (RC522 ilk byte gönderildikten sonra gönderdiği veri anlamsızdır.)

    if(st != SPI_STATUS_OK)
    {
        rc522_st = RC522_Reg_Write_Fail;
        goto exit;
    }

    st = SPI_Transfer(data, NULL);              // Değeri gönder ve gelen değeri al.
    if(st != SPI_STATUS_OK)
    {
        rc522_st = RC522_Reg_Write_Fail;
        goto exit;;
    }

    rc522_st = RC522_Status_OK;

    exit:
		GPIOA->BSRR = GPIO_BSRR_BS4;      // CS HIGH, iletişimi sonlandır.
		SPI1_Unlock();          		  // Mutex'i ver.
		return rc522_st;				  // Durum sonucu döndür.
}

// RC522 Register'lardan veri okuma.
RC522_Status_Type RC522_ReadReg(uint8_t reg, uint8_t* value)
{
	RC522_Status_Type rc522_st = RC522_Status_Start_Value;
	SPI_Status_t st = SPI_STATUS_START_VALUE;

    if(!SPI1_Lock(100)) return RC522_Reg_Read_Fail;

	uint8_t address = ((((reg << 1) & 0x7E) | 0x80));   // Okuma işlemi için trgister maskeleme (bit1 ve bit7 maskele ve bit7'i 1 yap

	GPIOA->BSRR = GPIO_BSRR_BR4;           // CS LOW

	st = SPI_Transfer(address, NULL);     // Adresi gönder, gelen değeri kullanma.

	if(st != SPI_STATUS_OK)
    {
    	rc522_st = RC522_Reg_Read_Fail;
    	goto exit;
    }

	st = SPI_Transfer(0x00, value);     // Sahte(dummy) byte gönder ve gelen değeri oku. (RC522 2 byte'lik veri alımından sonra değer döndürür.)
    if(st != SPI_STATUS_OK)
    {
    	rc522_st = RC522_Reg_Read_Fail;
    	goto exit;
    }

    rc522_st = RC522_Status_OK;  // Başarılı sonucunu bildir.

	exit:
		GPIOA->BSRR = GPIO_BSRR_BS4;      // CS HIGH
	    SPI1_Unlock();          		  // Mutex'i ver.
		return rc522_st;        		  // Durum sonucu döndür.
}


// Test için kart sector block'lardan veri silme.
RC522_Status_Type RC522_Clear_Card_Blocks(uint8_t block0_addr, uint8_t block1_addr)
{
    RC522_Status_Type st;

    uint8_t empty_block[16] = {0};

    /*
     * Sector trailer bloklarına kesinlikle yazılmamalı.
     * MIFARE Classic 1K'da her sektörün 4. bloğu trailer'dır.
     * Örnek:
     * Sector 1: block 4,5,6 data; block 7 trailer.
     */
    if((block0_addr % 4) == 3 || (block1_addr % 4) == 3)
    {
        return RC522_Status_Error;
    }

    // Sector 1 Block 0 temizle
    st = RC522_MIFARE_Write(block0_addr, empty_block, sizeof(empty_block));
    if(st != RC522_Status_OK)
    {
        return st;
    }

    vTaskDelay(pdMS_TO_TICKS(10));

    // Sector 1 Block 1 temizle
    st = RC522_MIFARE_Write(block1_addr, empty_block, sizeof(empty_block));
    if(st != RC522_Status_OK)
    {
        return st;
    }

    return RC522_Status_OK;
}
