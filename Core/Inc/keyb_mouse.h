#define ASCII		sizeof(uint8_t)

	/* Input keyboard report - 8 bytes
	 * --: reportID (if has it)
     * 0:  modifier keys          --- block 2
     * 1:  padding                --- block 3
     * 2:  keycode array [0]      --- block 6
     * 3:  keycode array [1]
     * 4:  keycode array [2]
     * 5:  keycode array [3]
     * 6:  keycode array [4]
     * 7:  keycode array [5]
     */

#define LEFT_CTRL		0x01	//LeftCtrl 		0
#define LEFT_SHIFT		0x02	//LeftShift		1
#define LEFT_ALT		0x04	//LeftAlt		2
#define LEFT_GUI		0x08	//LeftGUI		3
#define RIGHT_CTRL		0x10	//RightCtrl		4
#define RIGHT_SHIFT		0x20	//RightShift	5
#define RIGHT_ALT		0x40	//RightAlt		6
#define RIGHT_GUI		0x80	//RightGUI		7

const char keymatrix[ASCII]={
KEY_NONE,	KEY_NONE,	KEY_RIGHT,
};

