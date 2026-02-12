/*
 * источники:
 * 1602А datasheet
 * https://cxem.net/mc/mc263.php
 * LiquidCrystal lib Arduino
 */

#include "main.h"
#include "lcd_1602a.h"

//---Переопределяем порты для подключения дисплея, для удобства---//
#define     LCD_OUT               GPIOB->ODR
#define     LCD_PIN_RS            LCD_RS_Pin          // PB9
#define     LCD_PIN_EN            LCD_EN_Pin          // PB8
#define     LCD_PIN_D7            LCD_D7_Pin          // PB7
#define     LCD_PIN_D6            LCD_D6_Pin          // PB6
#define     LCD_PIN_D5            LCD_D5_Pin          // PB5
#define     LCD_PIN_D4            LCD_D4_Pin          // PB4
#define     LCD_PIN_MASK  ((LCD_PIN_RS | LCD_PIN_EN | LCD_PIN_D7 | LCD_PIN_D6 | LCD_PIN_D5 | LCD_PIN_D4))

#define DWT_CONTROL *(volatile unsigned long *)0xE0001000
#define SCB_DEMCR   *(volatile unsigned long *)0xE000EDFC

void DWT_Init(void) // OK
{
    SCB_DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // разрешаем использовать счётчик
	DWT_CONTROL |= DWT_CTRL_CYCCNTENA_Msk;   // запускаем счётчик
}

//---Функция задержки---// OK
void delay_microseconds(uint32_t usec)
{
    uint32_t us_count_tic =  usec * (SystemCoreClock / 1000000);
    DWT->CYCCNT = 0U; // обнуляем счётчик
    while(DWT->CYCCNT < us_count_tic);
}

//---Функция для работы с дисплеем, по сути "дергаем ножкой" EN---// OK
void pelseEnable()
{
	HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_PIN_EN, GPIO_PIN_RESET);	// LCD_OUT &= ~LCD_PIN_EN;	//
    delay_microseconds(2); // Минимальная длительность импульса Enable по даташиту 0,45 микросекунд
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_PIN_EN, GPIO_PIN_SET); //LCD_OUT |= LCD_PIN_EN;
    delay_microseconds(2);
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_PIN_EN, GPIO_PIN_RESET); //LCD_OUT &= (~LCD_PIN_EN);
    delay_microseconds(100);
}

//---Отсылка байта в дисплей---// OK
void SendByte(uint8_t value, uint8_t thisData) //(char ByteToSend, int IsData)
{
	if(thisData==1) HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_PIN_RS, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_PIN_RS, GPIO_PIN_RESET);
	write4bits(value>>4);
	write4bits(value);

}


//---Установка позиции курсора---// OK
void setCursor(uint8_t Row, uint8_t Col)
{
	uint8_t address;
   if (Row == 0) address = 0;
   else address = 0x40;
   address |= Col;
   SendByte(0x80 | address, 0);
}

//---Очистка дисплея---// OK
void ClearLCDScreen()
{
    SendByte(0x01, 0);
    SendByte(0x02, 0);
}

//---Инициализация дисплея---// OK
void InitializeLCD(void)
{
	delay_microseconds(50000); // Wait time after VDD > 15ms (кое где 40ms)
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_PIN_RS, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_PIN_EN, GPIO_PIN_RESET);
	// тут же инициализироавть нулем пин R/W если буду читать сост дисплея

	write4bits(0x03);	// set [0 0 - 0 0 1 1] -> [RS RW - DB7 DB6 DB5 DB4]
	delay_microseconds(5000);	// Wait for more than 4,1ms
	write4bits(0x03);	// set [0 0 - 0 0 1 1]
	delay_microseconds(5000);	// Wait for more than 100 us
	write4bits(0x03);	// set [0 0 - 0 0 1 1]
	delay_microseconds(500);

	write4bits(0x02);	// set [0 0 - 0 0 1 0] => set interface to be 4 bits mode

	// set1 [0 0 - 0 0 1 0] // set2 [0 0 - N F * *] => set num display lines and font. After this that can not be changed
	write4bits(0x02);
	write4bits(0x08);	// set2 [0 0 - 1 0 0 0] ; 0b1000 = 0x08 N=1(2line; N=0 1line) F=0(5x8font)

	// set1 [0 0 - 0 0 0 0] // set2 [0 0 - 1 0 0 0] => displey off
	write4bits(0x00);
	write4bits(0x0E);		// set2 [0 0 - 1 1 1 0] => displey on, cur on, blink off

	// set1 [0 0 - 0 0 0 0] // set2 [0 0 - 0 0 0 1] => displey clear
	write4bits(0x00);
	write4bits(0x01);		// set2 [0 0 - 0 1 0 0] => increment mode

	// set1 [0 0 - 0 0 0 0] // set2 [0 0 - 0 0 I/D S] => Entry mode set (I/D=1 increment mode; S=0 no shift)
	write4bits(0x00);
	write4bits(0x02);

	HAL_Delay(50);
}

void write4bits(uint8_t value) 	// OK
{
	/*for (int i = 0; i < 4; i++) {
		digitalWrite(_data_pins[i], (value >> i) & 0x01);
		HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_PIN_RS, GPIO_PIN_RESET);
	}*/
	if((value >> 0) & 0x01) HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, GPIO_PIN_RESET);
	if((value >> 1) & 0x01) HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, GPIO_PIN_RESET);
	if((value >> 2) & 0x01) HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, GPIO_PIN_RESET);
	if((value >> 3) & 0x01) HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, GPIO_PIN_RESET);

	pelseEnable();
}

void PrintStr(uint8_t *Text)
{
	uint8_t *c;
    c = Text;
    while ((c != 0) && (*c != 0))
    {
        SendByte(*c, 1);
        c++;
    }
}

