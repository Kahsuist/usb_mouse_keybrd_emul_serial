/* USER CODE BEGIN Header */
/**
 * индикотор 1602А(4бита) отображает полученыые по USART одиночные символы
 *
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd_1602a.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUFFER_LENGTH 5
#define CLICK_REPORT_SIZE 6
#define KEYBRD_REPORT_SIZE 9
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
extern int uart_recieve_message;
uint8_t click_report[CLICK_REPORT_SIZE] = {0};
uint8_t keybrd_report[KEYBRD_REPORT_SIZE] = {0};
extern USBD_HandleTypeDef hUsbDeviceFS;
uint8_t buff[BUFFER_LENGTH] = {0,};
int counter = 0;	// для отслеживания переполнения индикатора

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


//функция заполнения буфера отправки
void report_loader_keyboard(void)
{
	uint8_t b = buff[0];					// пришедший байт-код
	uint8_t btn_code = b & 0x7F;			// выделить код клавиши, соответствующий таблице Keyboard/Keypad Page (0x07) HID Usage Descriptions
	uint8_t press_release_flag = b & 0x80;	// выделить старший байт. 0 = кн нажатаб 1 = кн. отпущена

	// если флаг нажатия кнопки - добавить код в свободный байт репорта
	if(btn_code<0x70)					// символы до клавиш-модификаторов
	{
		if(press_release_flag==0)
		{
			uint8_t zero_byte = 0;			// первый нулевой байт
			uint8_t this_btn_present = 0;	// код нажатой клавиши есть
			for(uint8_t i=8; i>2; i--)
			{
				if(keybrd_report[i]==0) zero_byte = i;	// ищем нулевые байты, самый первый по счету в буфере будет в этой переменной
				if(keybrd_report[i]==btn_code) this_btn_present = 1; // код нажатой кнопки присутствует в буфере
			}
			// если код кнопки ещё не в буфере и пустой байт для кнопок в наличии, заносим в него код кнопки
			if(this_btn_present==0)
			{
				if(zero_byte>0) keybrd_report[zero_byte]=btn_code; // : (keybrd_report[zero_byte]=0);
			}
		}
		else	// если код отпускания кнопки - найти код этой кнопки и заменить нулем
		{
			for(uint8_t i=3; i<9; i++)
			{
				if(keybrd_report[i]==btn_code) keybrd_report[i] = 0;
			}
		}
	}
	else
	{
		// коды CtrlShiftAltGUI вместо Е0-Е7 кодируются как 0x70-0x77
		// 70 Keyboard LeftControl 0x01; 71 Keyboard LeftShift 0x02; 72 Keyboard LeftAlt 0x04; 73 Keyboard Left GUI 0x08
		// 74 Keyboard RightControl	0x10; 75 Keyboard RightShift 0x20; 76 Keyboard RightAlt	0x40; 77 Keyboard Right GUI	0x80

		for(uint8_t i=0; i<8; i++)
		{
			if(btn_code==0x70+i) press_release_flag==0 ? (keybrd_report[1] |= (0x01<<i)) : (keybrd_report[1] &= ~(0x01<<i));
		}
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	keybrd_report[0] = 1; // reportID
	click_report[0] = 2;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  DWT_Init();
  HAL_UART_Receive_IT(&huart2, (uint8_t*)buff, BUFFER_LENGTH);
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  InitializeLCD();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  ClearLCDScreen();
  setCursor(0,0);

  while (1)
  {
	  //testPressRelease_1();

	  HAL_UART_Receive_IT(&huart2, (uint8_t*)buff, BUFFER_LENGTH);

	  if(uart_recieve_message==1)
	  {
		  HAL_Delay(10); //прерывание приходит с 1 символом, подождать пока придут все символы


		  if(buff[0]==1)
		  {
			  for(uint8_t i=0; i<5; i++) keybrd_report[i] = buff[i];
			  USBD_HID_SendReport(&hUsbDeviceFS, keybrd_report, KEYBRD_REPORT_SIZE);
		  }

		  if(buff[0]==2)
		  {
			  for(uint8_t i=0; i<5; i++) click_report[i] = buff[i];
			  USBD_HID_SendReport(&hUsbDeviceFS, click_report, CLICK_REPORT_SIZE);
		  }

		  if(buff[0]==3)
		  {
			  // ========= очистка экрана 1602А при заполнении =========================
			  counter++;
			  if(counter==16) setCursor(1,0);
			  if(counter==32)
			  {
				  counter = 0;
				  setCursor(0,0);
				  ClearLCDScreen();
			  }
			  //==========================================================================
			  //PrintStr((uint8_t*)buff);	//PrintStr("Chundra");
			  SendByte(buff[1], 1);
		  }


		  // mouse experiment
		  /*
		  click_report[0] = 2; // ID репорта мыши

		  // нажать кнопку мыши
		  click_report[1] = 1; // send left button press
		  click_report[2] = 0; // X coord
		  click_report[3] = 0; // Y coord
		  USBD_HID_SendReport(&hUsbDeviceFS, click_report, CLICK_REPORT_SIZE);
		  HAL_Delay(50);

		  // провести линию вправо на 100 пикселей
		  click_report[2] = 100; // X coord
		  USBD_HID_SendReport(&hUsbDeviceFS, click_report, CLICK_REPORT_SIZE);
		  HAL_Delay(50);

		  // отпустить кнопку
		  click_report[1] = 0; // send left button press
		  click_report[2] = 0; // X coord
		  click_report[3] = 0; // Y coord
		  USBD_HID_SendReport(&hUsbDeviceFS, click_report, CLICK_REPORT_SIZE);
		  HAL_Delay(50);

		  // переместить мышь вниз на 50 пикселей
		  click_report[3] = 50; // Y coord
		  USBD_HID_SendReport(&hUsbDeviceFS, click_report, CLICK_REPORT_SIZE);
		  HAL_Delay(50);

		  // нажать кнопку мыши
		  click_report[1] = 1; // send left button press
		  click_report[3] = 0; // Y coord
		  USBD_HID_SendReport(&hUsbDeviceFS, click_report, CLICK_REPORT_SIZE);
		  HAL_Delay(50);
		  // провести линию влево на 75 пикселей
		  click_report[2] = -75; // X coord
		  USBD_HID_SendReport(&hUsbDeviceFS, click_report, CLICK_REPORT_SIZE);
		  HAL_Delay(50);

		  // отпустить кнопку мыши
		  click_report[1] = 0; // send left button press
		  click_report[2] = 0; // X coord
		  USBD_HID_SendReport(&hUsbDeviceFS, click_report, CLICK_REPORT_SIZE);

		  HAL_Delay(5000);
			*/
		  //==========================================================================
		  //HAL_Delay(10);
		  uart_recieve_message = 0;
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  //HAL_UART_Transmit_IT(&huart2, (uint8_t*)"ab", 2);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_D7_Pin|LCD_D6_Pin|LCD_D5_Pin|LCD_D4_Pin
                          |LCD_EN_Pin|LCD_RS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LCD_D7_Pin LCD_D6_Pin LCD_D5_Pin LCD_D4_Pin
                           LCD_EN_Pin LCD_RS_Pin */
  GPIO_InitStruct.Pin = LCD_D7_Pin|LCD_D6_Pin|LCD_D5_Pin|LCD_D4_Pin
                          |LCD_EN_Pin|LCD_RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
