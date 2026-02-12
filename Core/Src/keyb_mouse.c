/*
 *
//функция для эксперимента по выяснению особенностей перадачи нажатий-отпусканий кнопок
void testPressRelease_1(void)
{
	keybrd_report[0] = 1; // reportID
	keybrd_report[1] = 0x00; // modifier, 2=LeftShift
	keybrd_report[3] = 0x0B; // send button press "C"
	keybrd_report[4] = 0x00; // send button press --
	USBD_HID_SendReport(&hUsbDeviceFS, keybrd_report, KEYBRD_REPORT_SIZE);
	HAL_Delay(2250);

	keybrd_report[0] = 1; // reportID
	keybrd_report[1] = 0x00; // modifier, 2=LeftShift
	keybrd_report[3] = 0x06; // send button press "H"
	keybrd_report[4] = 0x00; // send button press --
	USBD_HID_SendReport(&hUsbDeviceFS, keybrd_report, KEYBRD_REPORT_SIZE);
	HAL_Delay(2250);

	keybrd_report[0] = 1; // reportID
	keybrd_report[1] = 0x00; // modifier, 2=LeftShift
	keybrd_report[3] = 0x00; // send button press --
	keybrd_report[4] = 0x00; // send button press --
	USBD_HID_SendReport(&hUsbDeviceFS, keybrd_report, KEYBRD_REPORT_SIZE);
	HAL_Delay(2250);

}
 *
 *
 *
 */


/* алгоритм заполнения буфера отправки нажатий
 * всего 6 мест-байт для кода клавиши
 * проверяем первый, если пусто - вставляем туда и отправляем
 * если занято - ++, првоеряем если пусто вставляем туда и
 * отправляем
 * если пришло отпускание клавиши - ищем в буфере соотв. нажатую и вставляем туда ноль.
 *
 * в свисток по usart надо отправлять коды для репорта, старший бит в байте будет определять нажата кнопка или отпущена
 * 0 = нажать, 1 = отпустить;
 * (in_byte &= 0b01111111)(0x7F) - выделить символ
 * (in_byte &= 0b10000000)(0x80) - выделить нажатие-отпускание
 * 0x70-0x77 коды для Ctrl Shift Alt GUI
 */
