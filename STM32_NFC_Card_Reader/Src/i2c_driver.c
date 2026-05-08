#include <i2c_driver.h>

// I2C1 için konfigürasyon ayarları fonksiyonu
void I2C1_Config(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;  //I2C clock aktif.

	I2C1->CR1 |= I2C_CR1_SWRST;
	Delay_Ms(1);
	I2C1->CR1 &= ~I2C_CR1_SWRST;
	Delay_Ms(10);

	//I2C1 çevresel birimi biti resetleme
	I2C1->CR1 &= ~I2C_CR1_PE;

	// I2C1 çevrebirimi saat frekans biti temizleme ve 42MHz'e ayarlama.
	I2C1->CR2 &= ~(0x1FU << 0);
	I2C1->CR2 |= (42U << 0);

	// Master mode biti temizleme ve fast mode yapma.
	I2C1->CCR &= ~I2C_CCR_FS;
	I2C1->CCR |= I2C_CCR_FS;

	// Fast mode duty cylce 2'ye ayarlama.
	I2C1->CCR &= ~I2C_CCR_DUTY;

	// Fast mode Duyt Cycle = Tlow / Thigh = 2 , Tscl = Tlow + Thigh = 3 ,  Fast mode CCR için CCR = fpclk1 / 3 x fscl , 42MHz / 3 x 400kHz =35 , CCR = 35
	// CCR Biti temizleme ve 35 yapma
	I2C1->CCR &= ~(0xFFFU << 0);
	I2C1->CCR |=  (0x23U << 0);

	// Fast mode için TRISE: TRISE = (Trisemax / Tpclk1) + 1  , (300ns / 23.8ns) + 1 = 12,6 + 1 = 13,6 14'e yuvarlanır
	// Trise bitlerini sıfırlama ve Trise bitinin değerini 14 yapma.
	I2C1->TRISE &= ~(0x1FU << 0);
	I2C1->TRISE |= (14U << 0);

	//I2C1 çevresel birimi biti aktifleştirme
	I2C1->CR1 |= I2C_CR1_PE;
}


I2C_Status_t I2C1_Write(uint8_t slave_addrr, uint8_t reg_addrr, uint8_t data)
{
	// Hat boşalana kadar bekle, hata durumu oluşursa hatayı dön.
	I2C_Status_t st_bsy = I2C_Wait_For_BSY_Flag();
	if(st_bsy != I2C_STATUS_OK) return st_bsy;

	I2C1->CR1 |= I2C_CR1_START;         // Start koşulu başlat.

	// Start koşulu başlayana kadar bekle, hata durumu oluşursa hata dön.
	I2C_Status_t st_sb = I2C_Wait_For_SB_Flag();
	if(st_sb != I2C_STATUS_OK) return st_sb;

	I2C1->DR = (slave_addrr << 1);      // DR register'a haberleşme yapılacak cihazın write adresini yaz.

	// ADDR flag kalkana kadar bekle, hata durumu oluşursa hata dön.
	I2C_Status_t st_addrr = I2C_Wait_For_Addrr_Flag();
	if(st_addrr != I2C_STATUS_OK) return st_addrr;

	(void)I2C1->SR2;                    // SR2 Registeri oku ADDR temizlensin.

	// TXE flag kalkana kadar bekle, hata durumu oluşursa hata dön.
	I2C_Status_t st_txe = I2C_Wait_For_TXE_Flag();
	if(st_txe != I2C_STATUS_OK) return st_txe;

	I2C1->DR = reg_addrr;               // DR register'a register adresini yaz.

	// TXE flag kalkana kadar bekle, hata durumu oluşursa hata dön.
	I2C_Status_t st_txe_2 = I2C_Wait_For_TXE_Flag();
	if(st_txe_2 != I2C_STATUS_OK) return st_txe_2;

	I2C1->DR = data;                    // DR register'a gönderilecek veriyi yaz.

	// BTF flag kalkana kadar bekle, hata durumu oluşursa hata dön.
	I2C_Status_t st_btf = I2C_Wait_For_BTF_Flag();
	if(st_btf != I2C_STATUS_OK) return st_btf;

	I2C1->CR1 |= I2C_CR1_STOP;          // Veri transferi bitince işlemi bitir.

	return I2C_STATUS_OK;
}

I2C_Status_t I2C1_Read(uint8_t slave_addrr, uint8_t reg_addrr, uint8_t* data)
{
	// Hat boşalana kadar bekle, hata durumu oluşursa hatayı dön.
	I2C_Status_t st_bsy = I2C_Wait_For_BSY_Flag();
	if(st_bsy != I2C_STATUS_OK) return st_bsy;

	I2C1->CR1 |= I2C_CR1_START;         // Start koşulu başlat.

	// Start koşulu başlayana kadar bekle, hata durumu oluşursa hata dön.
	I2C_Status_t st_sb = I2C_Wait_For_SB_Flag();
	if(st_sb != I2C_STATUS_OK) return st_sb;

	I2C1->DR = (slave_addrr << 1);      // DR register'a haberleşme yapılacak cihazın write adresini yaz.

	// ADDR flag kalkana kadar bekle, hata durumu oluşursa hata dön.
	I2C_Status_t st_addrr = I2C_Wait_For_Addrr_Flag();
	if(st_addrr != I2C_STATUS_OK) return st_addrr;

	(void)I2C1->SR2;                    // SR2 Registeri oku ADDR temizlensin.

	// TXE flag kalkana kadar bekle, hata durumu oluşursa hata dön.
	I2C_Status_t st_txe = I2C_Wait_For_TXE_Flag();
	if(st_txe != I2C_STATUS_OK) return st_txe;

	I2C1->DR = reg_addrr;               // DR register'a register adresini yaz.

	// TXE flag kalkana kadar bekle, hata durumu oluşursa hata dön.
	I2C_Status_t st_txe_2 = I2C_Wait_For_TXE_Flag();
	if(st_txe_2 != I2C_STATUS_OK) return st_txe_2;

	I2C1->CR1 |= I2C_CR1_START;         // Start koşulu başlat.

	// Start koşulu başlayana kadar bekle, hata durumu oluşursa hata dön.
	I2C_Status_t st_sb_2 = I2C_Wait_For_SB_Flag();
	if(st_sb_2 != I2C_STATUS_OK) return st_sb_2;

	I2C1->DR = (slave_addrr << 1) | 0x01;      // DR register'a haberleşme yapılacak cihazın read adresini yaz.

	// ADDR flag kalkana kadar bekle, hata durumu oluşursa hata dön.
	I2C_Status_t st_addr_2 = I2C_Wait_For_Addrr_Flag();
	if(st_addr_2 != I2C_STATUS_OK) return st_addr_2;

	I2C1->CR1 &= ~I2C_CR1_ACK;           // 1 byte alımı için ACK = 0

	(void)I2C1->SR2;                    // SR2 Registeri oku ADDR temizlensin.

	I2C1->CR1 |= I2C_CR1_STOP;          // Veri transferi bitince işlemi bitir.

	// RXNE flag kalkana kadar bekle, hata durumu oluşursa hata dön.
	I2C_Status_t st_rxne = I2C_Wait_For_RXNE_Flag();
	if(st_rxne != I2C_STATUS_OK) return st_rxne;

	*data = I2C1->DR;

	I2C1->CR1 |= I2C_CR1_ACK;           // ACK aktifleştir.

	return I2C_STATUS_OK;
}

I2C_Status_t I2C_Wait_For_BSY_Flag()
{
	uint32_t start = TIM6_Get_Millis();

	// BSY biti 0 olana kadar bekle ve bu sürede ilgili hata bayrakları kalkarsa, o hata durumunu dön.
	while(I2C1->SR2 & I2C_SR2_BUSY)
	{
		I2C_Status_t st = I2C_Check_Error_Flag();
		if(st != I2C_STATUS_OK) return st;

		// Eğer belirlenen sürede işlem bitmediyse timeout durumu dön
		if(TIM6_Get_Millis() - start > 10)
		{
			return I2C_STATUS_TIMEOUT;
		}
	}

	return I2C_STATUS_OK;
}

I2C_Status_t I2C_Wait_For_SB_Flag()
{
	uint32_t start = TIM6_Get_Millis();

	// SB biti 1 olana kadar bekle ve bu sürede ilgili hata bayrakları kalkarsa, o hata durumunu dön.
	while(!(I2C1->SR1 & I2C_SR1_SB))  // Start koşulu yaratma bayrağı 1 olana kadar bekle.
	{
		I2C_Status_t st = I2C_Check_Error_Flag();
		if(st != I2C_STATUS_OK) return st;

		// Eğer belirlenen sürede işlem bitmediyse timeout durumu dön
		if(TIM6_Get_Millis() - start > 5)
		{
			I2C1->CR1 |= I2C_CR1_STOP;
			return I2C_STATUS_TIMEOUT;
		}
	}

	return I2C_STATUS_OK;
}



I2C_Status_t I2C_Wait_For_Addrr_Flag()
{
	uint32_t start = TIM6_Get_Millis();

	// ADDR biti 1 olana kadar bekle ve bu sürede ilgili hata bayrakları kalkarsa, o hata durumunu dön.
	while(!(I2C1->SR1 & I2C_SR1_ADDR))
	{
		I2C_Status_t st = I2C_Check_Error_Flag();
		if(st != I2C_STATUS_OK) return st;

		// Eğer belirlenen sürede işlem bitmediyse timeout durumu dön
		if(TIM6_Get_Millis() - start > 5)
		{
			I2C1->CR1 |= I2C_CR1_STOP;
			return I2C_STATUS_TIMEOUT;
		}
	}

	return I2C_STATUS_OK;
}


I2C_Status_t I2C_Wait_For_TXE_Flag()
{
	uint32_t start = TIM6_Get_Millis();

	// TXE bayrağı 1 olana kadar bekle ve bu sürede ilgili hata bayrakları kalkarsa, o hata durumunu dön.
	while(!(I2C1->SR1 & I2C_SR1_TXE))
	{
		I2C_Status_t st = I2C_Check_Error_Flag();
		if(st != I2C_STATUS_OK) return st;

		// Eğer belirlenen sürede işlem bitmediyse timeout durumu dön
		if(TIM6_Get_Millis() - start > 5)
		{
			I2C1->CR1 |= I2C_CR1_STOP;
			return I2C_STATUS_TIMEOUT;
		}
	}

	return I2C_STATUS_OK;
}


I2C_Status_t I2C_Wait_For_RXNE_Flag()
{
	uint32_t start = TIM6_Get_Millis();

	// RXNE bayrağı 1 olana kadar bekle ve bu sürede ilgili hata bayrakları kalkarsa, o hata durumunu dön.
	while(!(I2C1->SR1 & I2C_SR1_RXNE))
	{
		I2C_Status_t st = I2C_Check_Error_Flag();
		if(st != I2C_STATUS_OK) return st;

		// Eğer belirlenen sürede işlem bitmediyse timeout durumu dön
		if(TIM6_Get_Millis() - start > 5)
		{
			I2C1->CR1 |= I2C_CR1_STOP;
			return I2C_STATUS_TIMEOUT;
		}
	}

	return I2C_STATUS_OK;
}

I2C_Status_t I2C_Wait_For_BTF_Flag()
{
	uint32_t start = TIM6_Get_Millis();

	// BTF bayrağı kalkana kadar bekle ve bu sürede ilgili hata bayrakları kalkarsa, o hata durumunu dön.
	while(!(I2C1->SR1 & I2C_SR1_BTF))
	{
		I2C_Status_t st = I2C_Check_Error_Flag();
		if(st != I2C_STATUS_OK) return st;

		// Eğer belirlenen sürede işlem bitmediyse timeout durumu dön
		if(TIM6_Get_Millis() - start > 5)
		{
			I2C1->CR1 |= I2C_CR1_STOP;
			return I2C_STATUS_TIMEOUT;
		}
	}

	return I2C_STATUS_OK;
}


// Hata bayrakları kontrol edip gönderim ve alım sırasında bayraklar kalkar ise durum bildirme.
I2C_Status_t I2C_Check_Error_Flag()
{
	// AF bayrağı kalkarsa ack başarısız olmuştur , AF bayrağını temizle ve işlemi durdur, durumu döndürüp fonksiyondan çık.
	if(I2C1->SR1 & I2C_SR1_AF)
	{
		I2C1->SR1 &= ~I2C_SR1_AF;
		I2C1->CR1 |= I2C_CR1_STOP;
		return I2C_STATUS_AF_ERROR;
	}

	// BERR bayrağı kalkarsa start veya stop condition'da hata olmuştur , BERR bayrağını temizle ve işlemi durdur, durumu döndürüp fonksiyondan çık.
	if(I2C1->SR1 & I2C_SR1_BERR)
	{
		I2C1->SR1 &= ~I2C_SR1_BERR;
		I2C1->CR1 |= I2C_CR1_STOP;
		return I2C_STATUS_BERR_ERROR;
	}

	// ARLO bayrağı kalkarsa iki cihaz aynı anda hatta veri basmaya çalışır , ARLO bayrağını temizle ve işlemi durdur, durumu döndürüp fonksiyondan çık.
	if(I2C1->SR1 & I2C_SR1_ARLO)
	{
		I2C1->SR1 &= ~I2C_SR1_ARLO;
		I2C1->CR1 |= I2C_CR1_STOP;
		return I2C_STATUS_ARLO_ERROR;
	}

	return I2C_STATUS_OK;  // Eğer hiçbir bayrak kalkmadıysa sorun yok , durumu döndür.
}
