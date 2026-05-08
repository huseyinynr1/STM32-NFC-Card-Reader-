#include "tft_ili9341.h"

// TFT başlangıç konfigürasyonları.
TFT_Status_t TFT_Init(void)
{
	TFT_Status_t st;
	// Donanım reseti,  ekran içindeki devre kendini sıfırlayana kadar bekle.
	GPIOB->BSRR |= GPIO_BSRR_BR3;    // RST LOW
	Delay_Ms(30);   // 30ms wait.
	GPIOB->BSRR |= GPIO_BSRR_BS3;    // RST High
	Delay_Ms(30);  // 200ms wait.

	TFT_CS_Low();

	// Power Control A register başlangıç konfigürasyonu.
	st = TFT_Config_Power_Control_A();
	if(st != TFT_STATUS_OK)
	{
		TFT_CS_High();
		return st;
	}

	// Power Control B register başlangıç konfigürasyonu.
	st = TFT_Config_Power_Control_B();
	if(st != TFT_STATUS_OK)
	{
		TFT_CS_High();
		return st;
	}

	// Driver Timing Control A register başlangıç konfigürasyonu.
	st = TFT_Config_Driver_Timing_Control_A();
	if(st != TFT_STATUS_OK)
	{
		TFT_CS_High();
		return st;
	}

	// Driver Timing Control B register başlangıç konfigürasyonu.
	st = TFT_Config_Driver_Timing_Control_B();
	if(st != TFT_STATUS_OK)
	{
		TFT_CS_High();
		return st;
	}

	// Power On Sequence Control register başlangıç konfigürasyonu.
	st = TFT_Config_Power_On_Sequence_Control();
	if(st != TFT_STATUS_OK)
	{
		TFT_CS_High();
		return st;
	}

	// Pump Ratio Control register başlangıç konfigürasyonu.
	st = TFT_Config_Pump_Ratio_Control();
	if(st != TFT_STATUS_OK)
	{
		TFT_CS_High();
		return st;
	}

	// Memory Access Control register başlangıç konfigürasyonu.
	st = TFT_Config_Memory_Access_Control();
	if(st != TFT_STATUS_OK)
	{
		TFT_CS_High();
		return st;
	}

	// COLMOD register başlangıç konfigürasyonu.
	st = TFT_Config_Pixel_Format();
	if(st != TFT_STATUS_OK)
	{
		TFT_CS_High();
		return st;
	}

	// TFT uykudan çıkarma.
	st = TFT_Config_Sleep_Out();
	if(st != TFT_STATUS_OK)
	{
		TFT_CS_High();
		return st;
	}

	// TFT ekranı açma.
	st = TFT_Config_Display_On();
	if(st != TFT_STATUS_OK)
	{
		TFT_CS_High();
		return st;
	}

	TFT_CS_High();
	return TFT_INIT_SUCCESS;    // TFT Başlatma başarılı.

}

// Power Control A register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Power_Control_A(void)
{
	TFT_Status_t st;

	// Güç devresi başlangıç ayarları.
	st = TFT_Write_Command(POWER_CONTROL_A);  // PCA registerına git.
	if(st != TFT_STATUS_OK) return st;

	// Vcore voltajını ayarla. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(PCA_Parameter_1);
	if(st != TFT_STATUS_OK) return st;

	// ekranın source driver bloğunun besleme voltajını (DDVDH) 5.6V ayarla.
	// İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(PCA_Parameter_2);
	if(st != TFT_STATUS_OK) return st;

	// Referans voltajını varsayılana ayarla. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(PCA_Parameter_3);
	if(st != TFT_STATUS_OK) return st;

	// GVDD gri ton voltaj üretecinin referansını 4.8V ayarla.
	// İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(PCA_Parameter_4);
	if(st != TFT_STATUS_OK) return st;

	// SAP (source amplifier) akımını orta seviye akım olarak ayarla.
	// İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(PCA_Parameter_5);
	if(st != TFT_STATUS_OK) return st;
	return TFT_STATUS_OK;         // İşlem başarılı.

}

// Power Control B register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Power_Control_B(void)
{
	TFT_Status_t st;

	// PCB registerına git. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Command(POWER_CONTROL_B);
	if(st != TFT_STATUS_OK) return st;

	// Reserved alanı, 0 gönder. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(PCB_Parameter_1);
	if(st != TFT_STATUS_OK) return st;

	// Güç tasarrufu ve VCOM güçlendirmesi aktif et. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(PCB_Parameter_2);
	if(st != TFT_STATUS_OK) return st;

	// ESD koruması aktif et. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(PCB_Parameter_3);
	if(st != TFT_STATUS_OK) return st;

	return TFT_STATUS_OK;         // İşlem başarılı.
}

// Driver Timing Control A register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Driver_Timing_Control_A(void)
{
	TFT_Status_t st;

	// DTCA registerına git. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Command(Driver_Timing_Control_A);
	if(st != TFT_STATUS_OK) return st;

	// Bir satır kapanmadan diğerini açmamak. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(DTCA_Parameter_1);
	if(st != TFT_STATUS_OK) return st;

	// EQ ve CR zamanlaması default değerde olsun. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(DTCA_Parameter_2);
	if(st != TFT_STATUS_OK) return st;

	// Ön şarj zamanlaması default değerde olsun. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(DTCA_Parameter_3);
	if(st != TFT_STATUS_OK) return st;

	return TFT_STATUS_OK;         // İşlem başarılı.
}

// Driver Timing Control B register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Driver_Timing_Control_B(void)
{
	TFT_Status_t st;

	// DTCB registerına git. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Command(Driver_Timing_Control_B);
	if(st != TFT_STATUS_OK) return st;

	 // Gate'in GND ve DDVDH arasındaki geçişi 1-2 unit süre. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(DTCB_Parameter_1);
	if(st != TFT_STATUS_OK) return st;

	// Anlamsız bitlere 0 değeri gönder. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(DTCB_Parameter_2);
	if(st != TFT_STATUS_OK) return st;

	return TFT_STATUS_OK;         // İşlem başarılı.
}

// Power On Sequence Control register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Power_On_Sequence_Control(void)
{
	TFT_Status_t st;

	 // POSC registerına git. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Command(Power_On_Sequence_Control);
	if(st != TFT_STATUS_OK) return st;

	// Tüm devreler 2 frame süre ile soft start başlasın. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(POSC_Parameter_1);
	if(st != TFT_STATUS_OK) return st;

	// Tüm devreler devreye girdikten sonra piksel sürücüsü açılsın.
	// İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(POSC_Parameter_2);
	if(st != TFT_STATUS_OK) return st;

	 // VGH 3.frame'de VGL 4.frame'de devreye girsin. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(POSC_Parameter_3);
	if(st != TFT_STATUS_OK) return st;

	// DDVDH güçlendirme modu aktif İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(POSC_Parameter_4);
	if(st != TFT_STATUS_OK) return st;

	return TFT_STATUS_OK;         // İşlem başarılı.
}

// Pump Ratio Control register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Pump_Ratio_Control(void)
{
	TFT_Status_t st;

	// Pump ratio control registerına git. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Command(Pump_Ratio_Control);
	if(st != TFT_STATUS_OK) return st;

	// DDVDH 6.6V ayarlama. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(PRC_Parameter_1);
	if(st != TFT_STATUS_OK) return st;

	return TFT_STATUS_OK;         // İşlem başarılı.
}

// Memory Access Control register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Memory_Access_Control(void)
{
	TFT_Status_t st;

	// Memory acces control register'a git. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Command(MADCTL_Reg);
	if(st != TFT_STATUS_OK) return st;

	// Dikey (portrait), yukarıdan aşağıya ve soldan sağa yazma ve yenileme ayarla.
	// İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(MADCTL_Parameter);
	if(st != TFT_STATUS_OK) return st;

	return TFT_STATUS_OK;         // İşlem başarılı.
}

// COLMOD register başlangıç konfigürasyonu.
TFT_Status_t TFT_Config_Pixel_Format(void)
{
	TFT_Status_t st;

	// Pixel format register'a git. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Command(COLMOD_Reg);
	if(st != TFT_STATUS_OK) return st;

	// RGB565 pixel formatı seç. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Data(COLMOD_Parameter);
	if(st != TFT_STATUS_OK) return st;

	return TFT_STATUS_OK;         // İşlem başarılı.
}

// TFT uykudan çıkarma.
TFT_Status_t TFT_Config_Sleep_Out(void)
{
	TFT_Status_t st;

	// Uyku modundan çık. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
	st = TFT_Write_Command(Sleep_Out_Reg);
	if(st != TFT_STATUS_OK) return st;
 	Delay_Ms(120);        // Güç devrelerinin stabilize olması için 120ms bekle

	return TFT_STATUS_OK;         // İşlem başarılı.
}

// TFT ekranı açma.
TFT_Status_t TFT_Config_Display_On(void)
{
	TFT_Status_t st;

 	// Ekranı aç. İşlemde hata çıkarsa hata durumunu dön ve fonksiyondan çık.
 	st = TFT_Write_Command(Display_On_Reg);
	if(st != TFT_STATUS_OK) return st;
	Delay_Ms(20);     // Güvenli başlangıç için 20ms wait.

	return TFT_STATUS_OK;         // İşlem başarılı.
}

TFT_Status_t TFT_Main_Screen(const char* time, const char* date)
{
	TFT_Status_t st;
	char bus_no[14] = "HAT NO : 429A";
	char t[10]  = "T:4000";
	char i[10]  = "I:2000";
	char i2[11] = "I2:2000";
	char ui_info [22] = "ISTANBUL UI VALIDATOR";
	char version [6]  = "H-001";

	if(!SPI1_Lock(100)) return TFT_STATUS_SPI_COMM_FAIL;

	TFT_CS_Low();
	st = TFT_Fill_Rect(0, 239, 0, 319, 0x05FD);  // Ana ekranın tamamını mavi renk yap.
	if(st != TFT_STATUS_OK) goto exit;

	st = TFT_Update_Date(date);
	if(st != TFT_STATUS_OK) goto exit;;

	st = TFT_Draw_String(120, 10, bus_no, 0xFFFF, 0x5DDF, 1.5);
	if(st != TFT_STATUS_OK) goto exit;

	st = TFT_Draw_String(90, 30, t, 0xFFFF, 0x5DDF, 1);
	if(st != TFT_STATUS_OK) goto exit;

	st = TFT_Draw_String(140, 30, i, 0xFFFF, 0x5DDF, 1);
	if(st != TFT_STATUS_OK) goto exit;

	st = TFT_Draw_String(190, 30, i2, 0xFFFF, 0x5DDF, 1);
	if(st != TFT_STATUS_OK) goto exit;

	st = TFT_Update_Time(time);
	if(st != TFT_STATUS_OK) goto exit;

	st = TFT_Draw_String(10, 280, ui_info, 0xFFFF, 0x5DDF, 1);
	if(st != TFT_STATUS_OK) goto exit;

	st = TFT_Draw_String(10, 300, version, 0xFFFF, 0x5DDF, 1);
	if(st != TFT_STATUS_OK) goto exit;

	st = TFT_Draw_Image(30, 60, 96 , 96, ibb_logo);
	if(st != TFT_STATUS_OK) goto exit;
	st = TFT_Draw_Image(30, 170, 192 , 96, card_logo);
	if(st != TFT_STATUS_OK) goto exit;
	st = TFT_Draw_Image(140, 75, 64 , 64, ist);
	if(st != TFT_STATUS_OK) goto exit;
	st = TFT_Draw_Image(200, 280, 32 , 32, two_g_logo);
	if(st != TFT_STATUS_OK) goto exit;

	st = TFT_STATUS_OK;  // İşlem başarılı.

	exit:
		TFT_CS_High();
		SPI1_Unlock();
		return st;
}

// Yeni kart kaydetme işlemi başarılı için ekran tasarımı.
TFT_Status_t TFT_New_Card_Saved_Screen(void)
{
	// Ekranın tamamını yeşil yap ve Kart kaydedildi yazısı çiz.
	TFT_Status_t st;
	char save_succes_arr[] = "Kart Kaydedildi!";

	if(!SPI1_Lock(100)) return TFT_STATUS_SPI_COMM_FAIL;

	TFT_CS_Low();
	st = TFT_Fill_Rect(0, 239, 0, 319, 0x07E0);
	if(st != TFT_STATUS_OK) goto exit;
	st = TFT_Draw_String(30, 40, save_succes_arr, 0xFFFF, 0x07E0, 2);
	if(st != TFT_STATUS_OK) goto exit;


	st = TFT_STATUS_OK;

	exit:
		TFT_CS_High();
		SPI1_Unlock();
		return st;
}

// Bakiye yükleme işlemi bildirim ekranı.
TFT_Status_t TFT_Upload_Balance_Screen(char* loaded_amount, char* new_balance)
{
	TFT_Status_t st;
	char succes_arr[] = {FONT_TR_UPPER_I_DOTTED, FONT_TR_LOWER_S_CEDILLA, 'l', 'e' , 'm', ' ',
			'B', 'a', FONT_TR_LOWER_S_CEDILLA, 'a', 'r', FONT_TR_LOWER_I_DOTLESS, 'l', FONT_TR_LOWER_I_DOTLESS, '\0'};

	if(!SPI1_Lock(100)) return TFT_STATUS_SPI_COMM_FAIL;
	// Bakiye yükleme işlemi başarılı ekranı çiz.
	TFT_CS_Low();
	st = TFT_Fill_Rect(0, 239, 0, 319, 0x5DDF);  // Ana ekranın tamamını mavi renk yap.
	if(st != TFT_STATUS_OK) goto exit;

	// Ekranda yüklenen miktar ve yeni bakiye bilgilerini çiz.
	st = TFT_Draw_String(40, 30, succes_arr, 0xFFFF, 0x5DDF, 2);
	if(st != TFT_STATUS_OK) goto exit;

	st = TFT_Draw_String(10, 70, loaded_amount, 0xFFFF, 0x5DDF, 1.75);
	if(st != TFT_STATUS_OK) goto exit;
	st = TFT_Draw_String(10, 110, new_balance, 0xFFFF, 0x5DDF, 1.75);
	if(st != TFT_STATUS_OK) goto exit;


	st = TFT_STATUS_OK;

	exit:
		TFT_CS_High();
		SPI1_Unlock();
		return st;
}

// İşlem başarılı ekranı tasarımı.
TFT_Status_t TFT_Process_Successfull_Screen(char* fare, char* balance)
{
	TFT_Status_t st;
	char succes_arr[] = {FONT_TR_UPPER_I_DOTTED, FONT_TR_LOWER_S_CEDILLA, 'l', 'e' , 'm', ' ',
			'B', 'a', FONT_TR_LOWER_S_CEDILLA, 'a', 'r', FONT_TR_LOWER_I_DOTLESS, 'l', FONT_TR_LOWER_I_DOTLESS, '\0'};

	if(!SPI1_Lock(100)) return TFT_STATUS_SPI_COMM_FAIL;
	// İşlem başarılı ekranı çiz.
	TFT_CS_Low();
	st = TFT_Fill_Rect(0, 239, 0, 319, 0x07E0);  // Ana ekranın tamamını yeşil renk yap.
	if(st != TFT_STATUS_OK) goto exit;

	// Ekranda çekilen ücret ve kalan bakiye bildirimlerini çiz.
	st = TFT_Draw_String(40, 30, succes_arr, 0xFFFF, 0x07E0, 2);
	if(st != TFT_STATUS_OK) goto exit;

	st = TFT_Draw_String(10, 70, fare, 0xFFFF, 0x07E0, 2);
	if(st != TFT_STATUS_OK) goto exit;
	st = TFT_Draw_String(10, 110, balance, 0xFFFF, 0x07E0, 2);
	if(st != TFT_STATUS_OK) goto exit;


	st = TFT_STATUS_OK;

	exit:
		TFT_CS_High();
		SPI1_Unlock();
		return st;
}

// İşlem başarısız ekranı tasarımı.
TFT_Status_t TFT_Process_Not_Successfull_Screen(uint8_t status)
{
	TFT_Status_t st;
	char not_succes_arr[20] = {0};
	char process_not_success_arr[] = {FONT_TR_UPPER_I_DOTTED, FONT_TR_LOWER_S_CEDILLA, 'l', 'e' , 'm', ' ',
			'B', 'a', FONT_TR_LOWER_S_CEDILLA, 'a', 'r', FONT_TR_LOWER_I_DOTLESS, 's', FONT_TR_LOWER_I_DOTLESS, 'z', '\0'};

	// Gelen durum değerlerine göre ekranda gösterilecek bildirimleri hazırla.
	switch (status) {
	    case 2:
	    	strcpy(not_succes_arr , "Gecersiz Kart");
	    	break;
	    case 29:
	    	strcpy(not_succes_arr , process_not_success_arr);
	    	break;
	    case 30:
	    	strcpy(not_succes_arr, "Yetersiz Bakiye");
	    	break;
	    case 31:
	    	 strcpy(not_succes_arr , process_not_success_arr);
	    	 break;
		case 32:
	    	 strcpy(not_succes_arr , process_not_success_arr);
			break;
		default:
			strcpy(not_succes_arr, "Bilinmeyen Hata");
			break;
	}

	if(!SPI1_Lock(100)) return TFT_STATUS_SPI_COMM_FAIL;

	// Hata bildirim ekranı çiz.
	TFT_CS_Low();
	st = TFT_Fill_Rect(0, 239, 0, 319, 0xF800);  // Ana ekranın tamamını kırmızı renk yap.
	if(st != TFT_STATUS_OK) goto exit;

	// Ekranda gösterilecek bildirimi çiz.
	st = TFT_Draw_String(30, 30, not_succes_arr, 0xFFFF, 0xF800, 2);
	if(st != TFT_STATUS_OK) goto exit;

	st = TFT_STATUS_OK;

	exit:
		TFT_CS_High();
		SPI1_Unlock();
		return st;
}

// Ekrandaki tarihi güncelleme fonksiyonu.
TFT_Status_t TFT_Update_Date(const char* date)
{
	TFT_Status_t st;

	st = TFT_Draw_String(10, 10, date, 0xFFFF, 0x5DDF, 1.5);  // Yeni tarih bilgisini ekrana yaz.
	if(st != TFT_STATUS_OK) return st;

	return TFT_STATUS_OK;
}

// Ekrandaki zamanı güncelleme fonksiyonu
TFT_Status_t TFT_Update_Time(const char* time)
{
	TFT_Status_t st;

	st = TFT_Draw_String(10, 30, time, 0xFFFF, 0x5DDF, 1.5); // Yeni zaman bilgisini ekrana yaz.
	if(st != TFT_STATUS_OK) return st;

	return TFT_STATUS_OK;
}


// Bir alan seçip o alanı belirtilen renkler ile doldurma
TFT_Status_t TFT_Fill_Rect(uint16_t start_col, uint16_t end_col, uint16_t start_row, uint16_t end_row , uint16_t color)
{
	TFT_Status_t st;

	// Alanı seç
	st = TFT_Set_Window(start_col, end_col, start_row, end_row);
	if(st != TFT_STATUS_OK) return st;

	// Verilen değerlerle şeklin alanını hesapla.
	uint32_t total_pixel;
	total_pixel = (uint32_t)(end_col - start_col + 1) * (end_row - start_row + 1);

	// Memory write komutunu gönder.
	st = TFT_Write_Command(Memory_Write_Reg);
	if(st != TFT_STATUS_OK) return st;

	// Alanı renkler ile doldur
	for(int i = 0; i < total_pixel; i++)
	{
		st = TFT_Send_Pixel(color);
		if(st != TFT_STATUS_OK) return st;

	}

	return TFT_STATUS_OK;  // İşlem başarılı.
}

// Ekrana birden çok karakteri çizme.
TFT_Status_t TFT_Draw_String(uint16_t x, uint16_t y, const char* str, uint16_t text_color, uint16_t background_color, float scale)
{
	while(*str)
	{
		TFT_Status_t st;
		st = TFT_Draw_Char(x, y, *str, text_color, background_color, scale);
		if(st != TFT_STATUS_OK) return st;
		x += 6 * scale;
		str++;
	}
	return TFT_STATUS_OK;  // İşlem başarılı.
}

TFT_Status_t TFT_Draw_Image(uint16_t x, uint16_t y, uint8_t width, uint16_t height, const uint16_t* image_array)
{
	TFT_Status_t st;

	st = TFT_Set_Window(x, x + width - 1, y, y + height - 1);
    if(st != TFT_STATUS_OK) return st;

    st = TFT_Write_Command(Memory_Write_Reg);
    if(st != TFT_STATUS_OK) return st;

    uint32_t pixel_count = width * height;

    for(int i = 0; i < pixel_count; i++)
    {
    	st = TFT_Send_Pixel(image_array[i]);
        if(st != TFT_STATUS_OK) return st;
    }

	return TFT_STATUS_OK;  // İşlem başarılı.
}

// Karakter yazma fonksiyonu.
TFT_Status_t TFT_Draw_Char(uint16_t x, uint16_t y, const char ch, uint16_t text_color, uint16_t background_color, float scale)
{
	TFT_Status_t st;

	// Font tablosundan karakteri bul, seçilen karakterin 5 byte'lık desenini al. (32: tablo space ile başladığı için)
	const uint8_t* bitmap = font5x7[ch-32];

	for(int col = 0; col < 5; col++)
	{
		uint8_t bits = bitmap[col];   // Sütunu (byte'ı al.)

		// Bitleri dolaş
		for(int row = 0; row < 8; row++)
		{
			uint16_t color = (bits & (1 << row)) ? text_color : background_color;

	        st = TFT_Fill_Rect(
	            x + col * scale,
	            x + col * scale + scale - 1,
	            y + row * scale,
	            y + row * scale + scale - 1,
	            color
	        );
	        if(st != TFT_STATUS_OK) return st;
		}
	}

    return TFT_STATUS_OK;
}

// Çizilecek karakterin yerini ve rengini gönderme.
TFT_Status_t TFT_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color)
{
    TFT_Status_t st;

    st = TFT_Set_Window(x, x, y, y);
    if(st != TFT_STATUS_OK) return st;

    st = TFT_Write_Command(Memory_Write_Reg);
    if(st != TFT_STATUS_OK) return st;

    st = TFT_Send_Pixel(color);
    if(st != TFT_STATUS_OK) return st;

    return TFT_STATUS_OK;
}


// TFT üzerinde belirli bir alan seçme.
TFT_Status_t TFT_Set_Window(uint16_t start_col, uint16_t end_col, uint16_t start_row, uint16_t end_row)
{
	TFT_Status_t st;

	// Sütun adreslerini belirle.
	st = TFT_Set_Column(start_col, end_col);
	if(st != TFT_STATUS_OK) return st;

	// Satır adreslerini belirle.
	st = TFT_Set_Row(start_row, end_row);
	if(st != TFT_STATUS_OK) return st;

	// Tüm işlemler başarılı. Başarılı durumu dön.
	return TFT_STATUS_OK;
}

// Ekranda gidilecek sütun koordinatlarını alıp o noktaya götürecek, sütun değerleri 16 bittir(0..239 dikey modda)
TFT_Status_t TFT_Set_Column(uint16_t start_col, uint16_t end_col)
{
	TFT_Status_t st;

	// Column set komutunu gönder.
	st = TFT_Write_Command(Column_Address_Set_Reg);
	if(st != TFT_STATUS_OK) return st;

	// SC (start column) üst 8 bit değerini yolla.
	st = TFT_Write_Data(start_col >> 8);
	if(st != TFT_STATUS_OK) return st;

	// SC (start column) alt 8 bit değerini yolla.
	st = TFT_Write_Data(start_col & 0xFF);
	if(st != TFT_STATUS_OK) return st;

	// EC (end column) üst 8 bit değerini yolla.
	st = TFT_Write_Data(end_col >> 8);
	if(st != TFT_STATUS_OK) return st;

	// EC (end column) alt 8 bit değerini yolla.
	st = TFT_Write_Data(end_col & 0xFF);
	if(st != TFT_STATUS_OK) return st;

	// Tüm işlemler başarılı. Başarılı durumu dön.
	return TFT_STATUS_OK;
}

// Ekranda gidilecek satır koordinatlarını alıp o noktaya götürecek, satır değerleri 16 bittir(0..319 dikey modda)
TFT_Status_t TFT_Set_Row(uint16_t start_row, uint16_t end_row)
{
	TFT_Status_t st;

	// Page address set komutunu gönder.
	st = TFT_Write_Command(Page_Address_Set_Reg);
	if(st != TFT_STATUS_OK) return st;

	// SP (start page) üst 8 bit değerini yolla.
	st = TFT_Write_Data(start_row >> 8);
	if(st != TFT_STATUS_OK) return st;

	// SP (start page) alt 8 bit değerini yolla.
	st = TFT_Write_Data(start_row & 0xFF);
	if(st != TFT_STATUS_OK) return st;

	// EP (end page) üst 8 bit değerini yolla.
	st = TFT_Write_Data(end_row >> 8);
	if(st != TFT_STATUS_OK) return st;

	// EP (end page) alt 8 bit değerini yolla.
	st = TFT_Write_Data(end_row & 0xFF);
	if(st != TFT_STATUS_OK) return st;

	// Tüm işlemler başarılı. Başarılı durumu dön.
	return TFT_STATUS_OK;
}


// Belirlenen alana işlenecek verileri gönderme.
TFT_Status_t TFT_Send_Pixel(uint16_t pixel)
{
	TFT_Status_t st;

	// Pixel verisinin üst 8 bit değerini yolla.
	st = TFT_Write_Data(pixel >> 8);
	if(st != TFT_STATUS_OK) return st;

	// Pixel verisinin alt 8 bit değerini yolla.
	st = TFT_Write_Data(pixel & 0xFF);
	if(st != TFT_STATUS_OK) return st;

	// Tüm işlemler başarılı. Başarılı durumu dön.
	return TFT_STATUS_OK;
}

// TFT CS pini LOW yapan fonksiyon.
void TFT_CS_Low()
{
	GPIOB->BSRR |= GPIO_BSRR_BR0; // CS LOW (Tft ile haberleşmeyi başlat.)
}

// TFT CS pini HIGH yapan fonksiyon.
void TFT_CS_High()
{
	GPIOB->BSRR |= GPIO_BSRR_BS0; // CS High (tft ile haberleşmeyi bitir.)
}

// TFT'ye SPI ile komut yolla.
TFT_Status_t TFT_Write_Command(uint8_t cmd)
{
	SPI_Status_t st;
	GPIOB->BSRR |= GPIO_BSRR_BR2; // DC LOW (komut gönderme.)

	st = SPI_Transfer(cmd, NULL);       // Register adresi yolla
	if(st != SPI_STATUS_OK) return TFT_STATUS_WRITE_COMMAND_FAIL;  // Komut yazılamadı hatası dön.

	return TFT_STATUS_OK;         // İşlem başarılı.
}

// TFT'ye SPI ile veri yolla.
TFT_Status_t TFT_Write_Data(uint8_t data)
{
	SPI_Status_t st;
	GPIOB->BSRR |= GPIO_BSRR_BS2; 		 // DC HIGH (veri gönderme.)

	st = SPI_Transfer(data, NULL);       // Register adresi yolla
	if(st != SPI_STATUS_OK) return TFT_STATUS_WRITE_DATA_FAIL;  // Veri yazılamadı hatası dön.

	return TFT_STATUS_OK;         // İşlem başarılı.
}
