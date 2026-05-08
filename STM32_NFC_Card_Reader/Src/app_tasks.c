#include "app_tasks.h"

// Kart güncel bakiye ve tutar bilgilerini TFT task'a taşıyan kuyruk.
QueueHandle_t card_balance_info_queue_for_screen = NULL;

// SIM800C Task'A HTTP GET ve POST işlemi için kart bilgilerini ve metod bilgisini taşıyan kuyruk.
QueueHandle_t http_request_queue = NULL;

// RFID Task'a API'den gelen yeni kart bilgilerini taşıyan kuyruk.
QueueHandle_t get_new_card_info_queue = NULL;

// Yeni kart bilgileri kart block'lara başarıyla yazıldığı bilgisini SIM800C'ye API'ye bildirmesi için kuyruk.
QueueHandle_t new_card_post_response_queue;

// RFID Task'a kart bakiye yükleme isteği ve bilgilerini taşıyan kuyruk.
QueueHandle_t topup_response_queue = NULL;

// Kart bloğuna yeni bakiye bilgisinin yazıldığı bilgisini API'ye bildirilmesi için SIM800C'ye taşıyan kuyruk.
QueueHandle_t topup_post_response_queue = NULL;

// RFID Task'a güncel zaman bilgisi taşıyan kuyruk.
QueueHandle_t time_info_queue_for_rfid = NULL;

// TFT Task'a güncel zaman bilgisini taşıyan kuyruk.
QueueHandle_t time_info_queue_for_screen = NULL;

// Task , Queue ve Mutex başlatmak.
void Task_Init()
{
	// Kuyrukları yaratmak.
	card_balance_info_queue_for_screen = xQueueCreate(2, sizeof(card_balance_info_helper));
	http_request_queue = xQueueCreate(2, sizeof(http_request_helper));
	get_new_card_info_queue = xQueueCreate(2, sizeof(new_card_response_helper));
	new_card_post_response_queue = xQueueCreate(2, sizeof(post_response_helper));
	topup_response_queue = xQueueCreate(2, sizeof(topup_response_helper));
	topup_post_response_queue = xQueueCreate(2, sizeof(post_response_helper));
	time_info_queue_for_rfid = xQueueCreate(1, sizeof(RTC_DS321_Time));
	time_info_queue_for_screen = xQueueCreate(1, sizeof(RTC_DS321_Time));

	// SPI1 için mutex başlat.
	SPI1_Mutex_Init();

	// Task'ları yaratmak.
	xTaskCreate(vRFID_TASK, "RFID_Task", 256, NULL, 4, NULL);
	xTaskCreate(vSIM800C_Task, "SIM800C_Task", 1024, NULL, 2, NULL);
	xTaskCreate(vTimer_Service_Task, "Timer_Service_Task", 256, NULL, 3, NULL);
	xTaskCreate(vTFT_LCD_Task, "TFT_LCD_Task", 256, NULL, 1, NULL);
	vTaskStartScheduler();

}

void vRFID_TASK(void *pvParameters)
{
	for(;;)
	{
		// Kart daha önceden HALT'a alınmış olabilir(bu durumda kart bulunamayabilir.) uyandırmayı dene.
		RC522_WakeupCard();

		RC522_Status_Type st;
		bool crypto_started = false;
		bool halt_card = false;

		// Kart ara
		st = RC522_IsNewCardPresent();

		if(st != RC522_Card_Found)
		{
			vTaskDelay(pdMS_TO_TICKS(250));
			continue;
		}

		uint8_t new_card_id[10] = {0};
		uint8_t new_card_id_len = 0;
		// Kart ID al
		st = RC522_ReadCardSerial(new_card_id, &new_card_id_len);

		if(st != RC522_Card_ID_Found)
		{
		    RC522_CommandReg_CommandBitValue(Idle);
		    RC522_FIFO_Flush();
		    RC522_IRQ_Clear();
		    vTaskDelay(pdMS_TO_TICKS(250));
		    continue;
		}

		// Karta veri yazılacak bloğu ve key'i tanımla
		uint8_t block_addrr = card_sector1_block0;
		uint8_t auth_command = KEYA;
		uint8_t key[6] = KEYA_Default_Value;

		// Kart doğrulamak için bir bloğundan veri oku.
		st = RC522_Authenticate(block_addrr, auth_command, key, new_card_id);

		if(st != RC522_Status_OK)
		{
		    RC522_CommandReg_CommandBitValue(Idle);
			vTaskDelay(pdMS_TO_TICKS(250));
			continue;
		}

		crypto_started = true;

		/*st = RC522_Clear_Card_Blocks(card_sector1_block0, card_sector1_block1);

		if(st != RC522_Status_OK)
		{
		    goto cleanup_card_session;
		}

		goto cleanup_card_session;*/


		uint8_t card_header_temp_raw[16] = {0};
		RC522_Card_Status_Type cs;
		st = RC522_MIFARE_READ(block_addrr, card_header_temp_raw);
		if(st != RC522_Status_OK)
		{
		    goto cleanup_card_session;
		}

		RC522_Card_Header card_header_temp_buff;
		memcpy(&card_header_temp_buff, card_header_temp_raw, 16);
		card_balance_info_helper card_balance_info_helper_temp = {0};
		uint16_t temp_id = (card_header_temp_buff.magic_number[0] << 8) | card_header_temp_buff.magic_number[1];

		// Bloktaki veri bu proje için belirlenen magic number'a eşit değilse ve o blokta veri varsa geçersiz(başka bir yere ait)karttır.
		if(temp_id != Magic_Number)
		{
		// Magic number eşleşmeyip fakat kartın içindeki belirlenen bloktaki veri yok (veya boş) ise kart yeni karttır.
			if(Helper_IsAllZero(card_header_temp_raw, sizeof(card_header_temp_raw)) || Helper_IsAllFF(card_header_temp_raw, sizeof(card_header_temp_raw)))
			{
				cs = New_Card;
			}

			else
			{
				cs = Invalid_Card;
			}
		}

		// Yukardaki şartları sağlamamışsa (magic number eşleşti ve veri var) o zaman sisteme kayıtlı karttır
		else
		{
			cs = Registered_Card;
		}


		if(cs == New_Card)
		{
			// Yeni kart durumunda kart id alınıp http'ye kart bilgileri için get isteği atılır.
			http_request_helper request_new_card = {0};       // Yeni kart id'sini sim800c task'a göndermek için kuyruk taşıyıcı değişkeni.
			new_card_response_helper receive_new_card_info = {0}; // Yeni kart bilgilerini sim800c task'tan almak için kuyruk taşıyıcı değişkeni.

			memcpy(request_new_card.uid, new_card_id, 4);
			request_new_card.request_type = HTTP_REQ_NEW_CARD_INFO;

			if(xQueueSend(http_request_queue, &request_new_card, pdMS_TO_TICKS(100))!= pdPASS)
			{
				st = RC522_StopCrypto();
				if(st != RC522_Status_OK) goto cleanup_card_session;;// Kriptolama durdur.
				vTaskDelay(pdMS_TO_TICKS(600));
			}

			if(xQueueReceive(get_new_card_info_queue, &receive_new_card_info, pdMS_TO_TICKS(30000)) == pdPASS)
			{
				st = RC522_CardPersonalization(&receive_new_card_info, new_card_id, card_sector1_block0, card_sector1_block1);
				if(st != RC522_Status_OK)
				{
					card_balance_info_helper_temp.status = st;
				}

				request_new_card.request_id = receive_new_card_info.request_id;
				request_new_card.status_type = Completed;
				request_new_card.request_type = HTTP_REQ_PERSONALIZATION_RESULT_POST;

				// Serverdan gelen isteğin başarılı ile tamamlandığını kuyruğa yazılır.
				if(xQueueSend(http_request_queue, &request_new_card, pdMS_TO_TICKS(1000)) != pdPASS) goto cleanup_card_session;

				post_response_helper post_response = {0};

				if(xQueueReceive(new_card_post_response_queue, &post_response, pdMS_TO_TICKS(30000)) == pdPASS)
				{
					// Yeni kart işlem sonucunu al ve kuyruğa yaz. Ekranda yeni kart okutulduktan sonra sonuç gösterilecek.
					card_balance_info_helper_temp.status = st;
					if(xQueueSend(card_balance_info_queue_for_screen, &card_balance_info_helper_temp, pdMS_TO_TICKS(100))!= pdPASS) goto cleanup_card_session;

				    halt_card = true;
				    goto cleanup_card_session;

				}
			}

			else
			{
				goto cleanup_card_session;;// Kriptolama durdur.
			}
		}

		else if(cs == Registered_Card)
		{
			RC522_Card_Header  	 card_header_temp  = {0};
			RC522_Card_Balance 	 card_balance_temp = {0};
			card_info_helper     card_info_helper_temp = {0};
			topup_response_helper new_topup_response = {0};
			RTC_DS321_Time       receive_time = {0};

			st = RC522_Get_Card_Info(new_card_id, &card_header_temp, &card_balance_temp);
			if(st != RC522_Status_OK) goto cleanup_card_session;

			http_request_helper request_new_card = {0};

			// Kart için  bakiye yükleme isteği varmı diye sorgulanır.
			memcpy(request_new_card.uid, new_card_id, 4);
			request_new_card.request_type = HTTP_REQ_TOPUP_CHECK;
			if(xQueueSend(http_request_queue, &request_new_card, pdMS_TO_TICKS(15000)) != pdPASS) goto cleanup_card_session;


			// Eğer bakiye yükleme isteği varsa bakiye yükleme fonksiyonu çağırılıp, bakiye kart bloğunda güncellenir.
			if(xQueueReceive(topup_response_queue, &new_topup_response, pdMS_TO_TICKS(15000)) == pdPASS)
			{
				if(new_topup_response.has_topup && new_topup_response.http_process_status)
				{
					st = RC522_Balance_Top_Up(&card_header_temp, &card_balance_temp, new_topup_response.amount);
					if(st == Balance_Upload_Successfull){

						http_request_helper balance_request_response = {0};
						balance_request_response.request_id = new_topup_response.request_id;
						balance_request_response.status_type = Completed;
						balance_request_response.request_type = HTTP_REQ_TOPUP_RESULT_POST;

						// Serverdan gelen isteğin başarılı ile tamamlandığını kuyruğa yazılır.
						if(xQueueSend(http_request_queue, &balance_request_response, pdMS_TO_TICKS(1000)) != pdPASS) goto cleanup_card_session;

						post_response_helper post_response = {0};

						if(xQueueReceive(topup_post_response_queue, &post_response, pdMS_TO_TICKS(30000)) == pdPASS)
						{
							// Yüklenen ve yeni bakiyeyi struct nesnesine ve kuyruğa yaz,  TFT task'a gönder.
							card_balance_info_helper_temp.loaded_amount = new_topup_response.amount;
							card_balance_info_helper_temp.balance = card_balance_temp.balance;
							card_balance_info_helper_temp.status = st;

							if(xQueueSend(card_balance_info_queue_for_screen, &card_balance_info_helper_temp, pdMS_TO_TICKS(1000)) != pdPASS) goto cleanup_card_session;
							halt_card = true;
							goto cleanup_card_session;

						}
					}

					else
					{
						request_new_card.request_id = new_topup_response.request_id;
						request_new_card.status_type = Failed;
						request_new_card.request_type = HTTP_REQ_TOPUP_RESULT_POST;

						card_balance_info_helper_temp.status = st;

						// Serverdan gelen isteğin başarılı ile tamamlandığını kuyruğa yazılır.
						if(xQueueSend(http_request_queue, &request_new_card, pdMS_TO_TICKS(100)) != pdPASS) goto cleanup_card_session;
						if(xQueueSend(card_balance_info_queue_for_screen, &card_balance_info_helper_temp, pdMS_TO_TICKS(100)) != pdPASS) goto cleanup_card_session;
					}

				}

				else if(!new_topup_response.http_process_status)
				{
					card_balance_info_helper_temp.status = Process_NotSuccessfull;
					if(xQueueSend(card_balance_info_queue_for_screen, &card_balance_info_helper_temp, pdMS_TO_TICKS(100)) != pdPASS) goto cleanup_card_session;
				}
			}

			if(xQueueReceive(time_info_queue_for_rfid, &receive_time, pdMS_TO_TICKS(1000)) == pdPASS)
			{
				st = RC522_Passanger_Card_Balance_Transaction(&card_header_temp, &card_balance_temp, &card_balance_info_helper_temp, &receive_time);
				if(st != Process_Successfull)
				{
					card_balance_info_helper_temp.status = st;
				}

				else
				{
					card_balance_info_helper_temp.status = Process_Successfull;
				}

				card_info_helper_temp.balance = card_balance_temp.balance;
				if(xQueueSend(card_balance_info_queue_for_screen, &card_balance_info_helper_temp, pdMS_TO_TICKS(15000)) != pdPASS) goto cleanup_card_session;

				halt_card = true;
				goto cleanup_card_session;
			}

		}

		else if(cs == Invalid_Card)
		{
			card_balance_info_helper_temp.status = cs;
			if(xQueueSend(card_balance_info_queue_for_screen, &card_balance_info_helper_temp, pdMS_TO_TICKS(15000)) != pdPASS) goto cleanup_card_session;

		    halt_card = true;
		    goto cleanup_card_session;
		}

		cleanup_card_session:
			if(crypto_started)
		    {
		        RC522_StopCrypto();
		    }

		    RC522_CommandReg_CommandBitValue(Idle);
		    RC522_FIFO_Flush();
		    RC522_IRQ_Clear();
		    RC522_SetTxLastBits(0);

		    if(halt_card)
		    {
		        RC522_HALT_Card();
		    }

		    vTaskDelay(pdMS_TO_TICKS(200));
	}
}




// SIM800C uygulama fonksiyonu.
void vSIM800C_Task(void *pvParameters)
{
	SIM800C_Status_Type st = SIM800C_Status_Start;           // Fonksiyon durum ve sonuç bilgisi için nesne.
	http_post_result http_st = Post_Result_Start_Value;      // HTTP POST işlemi sonucu bilgisi için nesne.
	char result_arr[64] = {0};                               // HTTP POST işlem sonucunu tutucak dizi.
	char http_result_post_response[128] = {0};           	 // POST işlemi yapıldıktan sonra POST işleminin sonucunu almak için oluşturulan değişken
	char http_get_new_card_info_response_arr[512] = {0};   	 // GET işlemi ile dönen cevabı tutacak dizi.
	char http_get_new_card_balance_response_arr[256] = {0};  // GET işlemi ile bakiye yükleme isteği sorgusu sonucunu tutucak dizi.
	char http_get_card_info_url[256] = {0};       			 // Kart id'ye göre kart bilgilerini almak için GET işlemi ile gönderilecek url.
	char http_get_card_balance_url[256] = {0};    			 // Bakiye yükleme kontrolü için oluşturulacak GET url.
	char arr_card_id[32] = {0};                   			 // Kart id'nin string'e çevirilip tutulacak değişken.
	http_request_helper request_new_card = {0}; 			 // Yeni kart'ın id'sini taşıyan kuyruktan alınan veriyi tutucak struct.
	new_card_response_helper new_card_info = {0};   		 // GET işlemi ile dönen yeni kart bilgileri tutucak struct.
	topup_response_helper new_topup_response = {0}; 		 // GET işlemi ile dönen yeni kart bakiye bilgilerini tutucak struct.
	post_response_helper post_response = {0};

	for(;;)
	{
		if(xQueueReceive(http_request_queue, &request_new_card, pdMS_TO_TICKS(30000)) == pdPASS)
		{
			// http_request_queue'den gelen işlem tipine göre yapılacak GET veya POST işlemleri.
			switch (request_new_card.request_type) {

			    // Yeni kart bilgisi adımı.
 				case HTTP_REQ_NEW_CARD_INFO:

					// Kuyruktan gelen kart id'yi al ve string'e çevir.
					Helper_Get_Card_ID_String(arr_card_id, sizeof(arr_card_id), request_new_card.uid, sizeof(request_new_card.uid));

					// Get işlemi yapılacak url'i oluştur.
					Helper_Build_Get_Url(http_get_card_info_url, sizeof(http_get_card_info_url), GET_NEW_CARD_INFO_URL, "cardUid", arr_card_id);

					// Yeni kart bilgilerini GET işlemi ile al.
					st = HTTP_GET(http_get_card_info_url, http_get_new_card_info_response_arr, sizeof(http_get_new_card_info_response_arr));
					if(st != SIM800C_Status_OK) continue;

					// Dizinin ilk elemanı string sonu elemanı değilse cevap geldi.
					if(http_get_new_card_info_response_arr[0] != '\0')
					{
						Helper_Json_Convert_to_New_Card_Info(http_get_new_card_info_response_arr, &new_card_info);
						xQueueSend(get_new_card_info_queue, &new_card_info, pdMS_TO_TICKS(100));
					}
					break;

			  // Bakiye yükleme isteği kontrol adımı.
			  case HTTP_REQ_TOPUP_CHECK:
					// Kuyruktan gelen kart id'yi al ve string'e çevir.
					Helper_Get_Card_ID_String(arr_card_id, sizeof(arr_card_id), request_new_card.uid, sizeof(request_new_card.uid));

					// Get işlemi yapılacak url'i oluştur.
					Helper_Build_Get_Url(http_get_card_balance_url, sizeof(http_get_card_balance_url), GET_TOPUP_REQUEST_INFO_URL, "cardUid", arr_card_id);

					st = HTTP_GET(http_get_card_balance_url, http_get_new_card_balance_response_arr, sizeof(http_get_new_card_balance_response_arr));
					if(st != SIM800C_Status_OK)
					{
						new_topup_response.http_process_status = false;
						xQueueSend(topup_response_queue, &new_topup_response, pdMS_TO_TICKS(1000));
					}

					// Dizinin ilk elemanı string sonu elemanı değilse cevap geldi.
					if(http_get_new_card_balance_response_arr[0] != '\0')
					{
						Helper_Json_Convert_to_New_Balance(http_get_new_card_balance_response_arr, &new_topup_response);
						xQueueSend(topup_response_queue, &new_topup_response, pdMS_TO_TICKS(1000));
					}
					break;

			 // Yeni kart bilgileri ilgili kart block'lara yazıldı bilgisini API'ye bildirme adımı.
			 case HTTP_REQ_PERSONALIZATION_RESULT_POST:
				 Helper_Conver_to_Json_Request_Result(&request_new_card, result_arr, sizeof(result_arr));
				 st = HTTP_POST(POST_PERSONALIZATION_REQUEST_RESPONSE_URL, result_arr, http_result_post_response, sizeof(http_result_post_response), 3000);
				 if(st != SIM800C_Status_OK)
				{
					 post_response.http_process_status = false;
				}

				 http_st = Helper_Post_Response_Code(http_result_post_response);  // POST işlemi sonucunu al

				 // POST işlemi başarılıys kuyruğu boşalt.
				 if(http_st == Post_Result_Successfull)
				 {
					 post_response.http_process_status = true;
				 	 post_response.result = Post_Result_Successfull;

				 	 xQueueSend(new_card_post_response_queue, &post_response, pdMS_TO_TICKS(100));
				 }

				 else if(http_st == Post_Result_Invalid_Json)
				 {
						post_response.http_process_status = true;
						post_response.result = Post_Result_Invalid_Json;

						xQueueSend(new_card_post_response_queue, &post_response, pdMS_TO_TICKS(100));;
				 }

				// Database, server vb. hatalarda tekrar kuyruğu okuyup POST işlemini 3 defa dene..
					else if(http_st == Post_Result_DB_Error ||
							http_st == Post_Result_Server_Error||
							http_st == Post_Result_Unknown_Error)
					 {
						for(int retry_post = 0; retry_post < 3; retry_post++)
						{
							st = HTTP_POST(POST_PERSONALIZATION_REQUEST_RESPONSE_URL, result_arr, http_result_post_response, sizeof(http_result_post_response), 3000);
							if(st == SIM800C_Status_OK) break;
						}

						http_st = Helper_Post_Response_Code(http_result_post_response);  // POST işlemi sonucunu al

						// POST işlemi başarılıys kuyruğu boşalt.
						if(http_st != Post_Result_Successfull)
						{
							post_response.http_process_status = false;
							post_response.result = http_st;

							xQueueSend(new_card_post_response_queue, &post_response, pdMS_TO_TICKS(100));
						}

						else
						{
							post_response.http_process_status = true;
							post_response.result = Post_Result_Successfull;

							xQueueSend(new_card_post_response_queue, &post_response, pdMS_TO_TICKS(100));
						}

					 }
				break;

			 // Karta yeni bakiyenin yüklendiğini bildirmek için adım.
			 case HTTP_REQ_TOPUP_RESULT_POST:

				 Helper_Conver_to_Json_Request_Result(&request_new_card, result_arr, sizeof(result_arr));
				 st = HTTP_POST(POST_TOPUP_REQUEST_RESPONSE_URL, result_arr, http_result_post_response, sizeof(http_result_post_response), 3000);
				 if(st != SIM800C_Status_OK)
				 {
					 post_response.http_process_status = false;
				 }

				 http_st = Helper_Post_Response_Code(http_result_post_response);  // POST işlemi sonucunu al

				// POST işlemi başarılıys kuyruğu boşalt.
				 if(http_st == Post_Result_Successfull)
				 {
					post_response.http_process_status = true;
					post_response.result = Post_Result_Successfull;

					xQueueSend(topup_post_response_queue, &post_response, pdMS_TO_TICKS(100));
				 }

				 else if(http_st == Post_Result_Invalid_Json)
				 {
						post_response.http_process_status = true;
						post_response.result = Post_Result_Invalid_Json;

						xQueueSend(topup_post_response_queue, &post_response, pdMS_TO_TICKS(100));;
				 }

				// Database, server vb. hatalarda tekrar kuyruğu okuyup POST işlemini 3 defa dene
				else if(http_st == Post_Result_DB_Error ||
						http_st == Post_Result_Server_Error||
						http_st == Post_Result_Unknown_Error)
				 {
					for(int retry_post = 0; retry_post < 3; retry_post++)
					{
						st = HTTP_POST(POST_TOPUP_REQUEST_RESPONSE_URL, result_arr, http_result_post_response, sizeof(http_result_post_response), 3000);
						if(st == SIM800C_Status_OK) break;
					}

					http_st = Helper_Post_Response_Code(http_result_post_response);  // POST işlemi sonucunu al

					// POST işlemi başarılıys kuyruğu boşalt.
					if(http_st != Post_Result_Successfull)
					{
						post_response.http_process_status = false;
						post_response.result = http_st;

						xQueueSend(topup_post_response_queue, &post_response, pdMS_TO_TICKS(100));
					}

					else
					{
						post_response.http_process_status = true;
						post_response.result = Post_Result_Successfull;

						xQueueSend(topup_post_response_queue, &post_response, pdMS_TO_TICKS(100));
					}

				 }
				break;
				default:
					break;
			}
		}

		//vTaskDelay(pdMS_TO_TICKS(1000));
	}
}



void vTimer_Service_Task(void *pvParameters)
{
	RTC_DS321_Time temp_time = {0};       // Zaman ve tarih verilerini tutucak struct nesnesi.

	for(;;)
	{
		RTC_DS3231_Status st;
		// Saati ve tarihi donanımdan al ve struct nesnesine yaz.
		st = RTC_DS3231_Get_Time(&temp_time);
		if(st != RTC_DS3231_OK) return;
		st = RTC_DS3231_Get_Date(&temp_time);
		if(st != RTC_DS3231_OK) return;

		// Zaman ve tarih verilerini kuyruklara yaz.
		xQueueOverwrite(time_info_queue_for_rfid, &temp_time);
		xQueueOverwrite(time_info_queue_for_screen, &temp_time);

		vTaskDelay(pdMS_TO_TICKS(1000));     // Görevi 1 saniye beklemeye al.
	}
}


void vTFT_LCD_Task(void *pvParameters)
{
	TFT_Status_t st = TFT_STATUS_ERROR;
	TFT_Status_Screen current_screen;
	RTC_DS321_Time receive_time = {0};
	card_balance_info_helper receive_balance_and_fare = {0};

	char time_buffer[6]     = "00:00";
	char date_buffer[11]    = "01/01/2001";
	char balance_buffer[21] = {0};
	char fare_buffer[15]     = {0};
	char amount_buffer[25]  = {0};

	for(int i = 0; i < 3; i++)
	{
		// Kart okutulmadığı sürece her döngüde ana ekranı ve zamanı tazele.
		st = TFT_Main_Screen(time_buffer, date_buffer);
		if(st != TFT_STATUS_OK) continue;

		current_screen = MAIN_SCREEN;    // Şuanki ekran bilgisini ana ekran olarak güncelle.
		break;
	}

	for(;;)
	{
		// TFT başarılı şekilde başlamışsa zaman kuyruğundan zaman verisini kart kuyruğundan kart verisini al.
		if(current_screen == MAIN_SCREEN)
		{
			// Zaman kuyruğundan 10ms veri bekle, al ve string diziye dönüştür.
			if(xQueueReceive(time_info_queue_for_screen, &receive_time, pdMS_TO_TICKS(100)) == pdPASS)
			{
				Helper_Convert_Date_To_String(date_buffer, sizeof(date_buffer), &receive_time);
				Helper_Convert_Time_To_String(time_buffer, sizeof(time_buffer), &receive_time);

				if(!SPI1_Lock(100)) continue;
				// Ana ekrandaki eski zaman ve tarih bilgisini sil ve yenisini yaz.
				TFT_CS_Low();       // TFT CS pinini düşük seviyeye çek.

				// Ekranda tarihin yazılı olduğu yeri temizle.
				st = TFT_Fill_Rect(10, 60, 10, 17, 0x5DDF);
				if(st != TFT_STATUS_OK)
				{
				    TFT_CS_High();
					SPI1_Unlock();
				    continue;
				}

				// Yeni tarihi ekranda yaz.
				st = TFT_Update_Date(date_buffer);
				if(st != TFT_STATUS_OK)
				{
				    TFT_CS_High();
					SPI1_Unlock();
				    continue;
				}

				// Ekranda saatin yazılı olduğu yeri temizle.
				st = TFT_Fill_Rect(10, 35, 30, 37, 0x5DDF);
				if(st != TFT_STATUS_OK)
				{
				    TFT_CS_High();
					SPI1_Unlock();
				    continue;
				}

				// Yeni saati ekranda yaz.
				st = TFT_Update_Time(time_buffer);
				TFT_CS_High();
				SPI1_Unlock();

				if(st != TFT_STATUS_OK) continue;


			}


			// Okutulan kart için bakiye ve tutar bilgisini kuyruktan oku.
			if(xQueueReceive(card_balance_info_queue_for_screen, &receive_balance_and_fare, pdMS_TO_TICKS(20000)) == pdPASS)
			{
				// Yeni kart okutulduysa yeni kart bildirim ekranı çiz.
				if(receive_balance_and_fare.status == RC522_Status_OK)
				{
					TFT_New_Card_Saved_Screen();
					current_screen = NEW_CARD_SCREEN;

					vTaskDelay(pdMS_TO_TICKS(2000));

					TFT_Main_Screen(time_buffer, date_buffer);
					current_screen = MAIN_SCREEN;
				}

				// Karttan çekim işlemi başarılıysa işlem başarılı ekranına geç.
				else if(receive_balance_and_fare.status == Process_Successfull)
				{
					Helper_Convert_Fare_To_String(fare_buffer, sizeof(fare_buffer), receive_balance_and_fare.fare);
					Helper_Convert_Balance_To_String(balance_buffer, sizeof(balance_buffer), receive_balance_and_fare.balance);
					TFT_Process_Successfull_Screen(fare_buffer, balance_buffer);
					current_screen = SUCCESS_SCREEN;
					// Kart sonucu ekranda 2 saniye görünsün, sonra ana ekrana dönsün
					vTaskDelay(pdMS_TO_TICKS(2000));
					TFT_Main_Screen(time_buffer, date_buffer);
					current_screen = MAIN_SCREEN;
				}

				// Yeni bakiye yüklenmiş ise miktarı ve bakiyeyi ekranda göster.
				else if(receive_balance_and_fare.status == Balance_Upload_Successfull)
				{
					Helper_Convert_Loaded_Amount_To_String(amount_buffer, sizeof(amount_buffer), receive_balance_and_fare.loaded_amount);
					Helper_Convert_Balance_To_String(balance_buffer, sizeof(balance_buffer), receive_balance_and_fare.balance);
					TFT_Upload_Balance_Screen(amount_buffer, balance_buffer);
					current_screen = UPLOAD_BALANCE_SCREEN;
					// Kart sonucu ekranda 2 saniye görünsün, sonra ana ekrana dönsün
					vTaskDelay(pdMS_TO_TICKS(2000));
					TFT_Main_Screen(time_buffer, date_buffer);
					current_screen = MAIN_SCREEN;
				}

				// Karttan çekim işlemi başarılı değilse işlem başarısız ekranına geç, durum sonucunu ekranda göster.
				else
				{
					TFT_Process_Not_Successfull_Screen(receive_balance_and_fare.status);
					current_screen = FAIL_SCREEN;
					// Kart sonucu ekranda 2 saniye görünsün, sonra ana ekrana dönsün
					vTaskDelay(pdMS_TO_TICKS(2000));
					TFT_Main_Screen(time_buffer, date_buffer);
					current_screen = MAIN_SCREEN;
				}
			}
		}
		// 200ms de bir uykuya al.
		vTaskDelay(pdMS_TO_TICKS(200));;
	}
}


