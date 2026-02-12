void DWT_Init(void);
void delay_micros(uint32_t usec);
void pelseEnable(void);
void SendByte(uint8_t value, uint8_t thisData);
void setCursor(uint8_t Row, uint8_t Col);
void ClearLCDScreen(void);
void InitializeLCD(void);
void write4bits(uint8_t value);
void PrintStr(uint8_t *Text);
