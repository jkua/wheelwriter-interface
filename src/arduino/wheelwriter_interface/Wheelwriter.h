// Wheelwriter class to control an IBM Wheelwriter
// via the option interface
//
// Copyright (c) 2023 John Kua <john@kua.fm>
//
#pragma once

#include "Uart9bit.h"

namespace wheelwriter {

enum ww_model {
	UNKNOWN = 0x00,
	WHEELWRITER_3 = 0x06,
	WHEELWRITER_5 = 0x25,
	WHEELWRITER_6 = 0x26
};

enum ww_command {
	QUERY_MODEL = 0x00,
	RESET = 0x01,
	TYPE_CHARACTER_NO_ADVANCE = 0x02,
	TYPE_CHARACTER_AND_ADVANCE = 0x03,
	ERASE_CHARACTER_AND_ADVANCE = 0x04,
	MOVE_PLATEN = 0x05,
	MOVE_CARRIAGE = 0x06,
	SPIN_WHEEL = 0x07,
	QUERY_WHEEL = 0x08,
	SET_REPEAT_MODE = 0x09,
	UNKNOWN_0A = 0x0a,
	QUERY_STATUS = 0x0b,
	UNKNOWN_0C = 0x0c,
	QUERY_OPERATION = 0x0d,
	SEND_CODE = 0x0e,
};

enum ww_printwheel {
	PROPORTIONAL = 0x08,
	CPI_15 = 0x10,
	CPI_12 = 0x20,
	NO_WHEEL = 0x21,
	CPI_10 = 0x40
};

enum ww_repeat_mode {
	REPEAT_OFF = 0x00,
	SHIFT_RIGHT_HALF_SPACE = 0x02,
	UNDERLINE = 0x04,
	REPEAT_CHARACTER = 0x08,
	CENTER = 0x80,
};

enum ww_status {
	NO_STATUS = 0x00,
	CARRIAGE_MOTION_COMPLETE = 0x04,
	PAPER_DOWN = 0x10,
	CARRIAGE_MOTION_2 = 0x14,
	PRINTWHEEL_CHANGE = 0x40
};

enum ww_operation {
	BACKSPACE = 0x04,
	ERASE = 0x05,
	CARRIAGE_RETURN = 0x07,
	TAB = 0x0a,
	TAB_SET = 0x12
};

enum ww_code {
	LANG = 0x8e,
	CTR = 0x96,
	A_RTN = 0x9a,
	UNDLN = 0xa2,
	HALF_UP = 0xa3,
	HALF_DOWN = 0xa5,
	CAPS = 0xa7,
	INDENT = 0xca,
	BKSP1 = 0xd0,
	INDENT_CLEAR = 0xd6,
	CODE_ONLY = 0xe7 
};

static const uint16_t WW_MOTOR_CTRL_ADDR = 0x121;
static const uint8_t WW_PRINTWHEEL_MAX = 0x60;
static const uint8_t WW_CARRIAGE_ADVANCE_USTEP_MAX = 63;
static const uint8_t WW_PLATEN_ADVANCE_USTEP_MAX = 127;
static const uint8_t WW_CARRIAGE_DIRECTION_LEFT = 0x00;
static const uint8_t WW_CARRIAGE_DIRECTION_RIGHT = 0x80;
static const uint8_t WW_PLATEN_DIRECTION_UP = 0x80;
static const uint8_t WW_PLATEN_DIRECTION_DOWN = 0x00;

//------------------------------------------------------------------------------------------------
// ASCII to Wheelwriter printwheel translation table
// The Wheelwriter printwheel code indicates the position of the character on the printwheel. 
// “a” (code 01) is at the 12 o’clock position of the printwheel. Going counter clockwise, 
// “n” (code 02) is next character on the printwheel followed by “r” (code 03), “m” (code 04),
// “c” (code 05), “s” (code 06), “d” (code 07), “h” (code 08), and so on.
//------------------------------------------------------------------------------------------------
static const char ascii2Printwheel[160] =  
// col: 00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F    row:
//      sp     !     "     #     $     %     &     '     (     )     *     +     ,     -     .     /
      {0x00, 0x49, 0x4b, 0x38, 0x37, 0x39, 0x3f, 0x4c, 0x23, 0x16, 0x36, 0x3b, 0x0c, 0x0e, 0x57, 0x28, // 20
//       0     1     2     3     4     5     6     7     8     9     :     ;     <     =     >     ?
       0x30, 0x2e, 0x2f, 0x2c, 0x32, 0x31, 0x33, 0x35, 0x34, 0x2a ,0x4e, 0x50, 0x00, 0x4d, 0x00, 0x4a, // 30
//       @     A     B     C     D     E     F     G     H     I     J     K     L     M     N     O
       0x3d, 0x20, 0x12, 0x1b, 0x1d, 0x1e, 0x11, 0x0f, 0x14, 0x1F, 0x21, 0x2b, 0x18, 0x24, 0x1a, 0x22, // 40
//       P     Q     R     S     T     U     V     W     X     Y     Z     [     \     ]     ^     _   
       0x15, 0x3e, 0x17, 0x19, 0x1c, 0x10, 0x0d, 0x29, 0x2d, 0x26, 0x13, 0x41, 0x00, 0x40, 0x00, 0x4f, // 50
//       `     a     b     c     d     e     f     g     h     i     j     k     l     m     n     o
       0x00, 0x01, 0x59, 0x05, 0x07, 0x60, 0x0a, 0x5a, 0x08, 0x5d, 0x56, 0x0b, 0x09, 0x04, 0x02, 0x5f, // 60
//       p     q     r     s     t     u     v     w     x     y     z     {     |     }     ~    DEL  
       0x5c, 0x52, 0x03, 0x06, 0x5e, 0x5b, 0x53, 0x55, 0x51, 0x58, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, // 70
//     
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 80      
//     
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 90      
//                   ¢                             §                                                  
       0x00, 0x00, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // A0      
//       °     ±     ²     ³                 ¶                                   ¼     ½              
       0x44, 0x3C, 0x42, 0x43, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x47, 0x00, 0x00};// B0

class Wheelwriter {
public:
	Wheelwriter() {
		init_ = 0;
	}
	void init(Uart9Bit* uart) {
		uart_ = uart;
		buffer[0] = WW_MOTOR_CTRL_ADDR;
		init_ = 1;
	}
	uint8_t sendCommand(ww_command command);
	uint8_t sendCommand(ww_command command, uint8_t data);
	uint8_t sendCommand(ww_command command, uint8_t data1, uint8_t data2);

	ww_model queryModel();
	ww_printwheel reset();
	void typeAscii(uint8_t ascii);
	void typeAscii(uint8_t ascii, uint8_t advanceUsteps);
	void typeCharacter(uint8_t wheelPosition);
	void typeCharacter(uint8_t wheelPosition, uint8_t advanceUsteps);
	void eraseCharacter(uint8_t wheelPosition, uint8_t advanceUsteps);
	void movePlaten(int8_t usteps);
	void movePlaten(uint8_t usteps, uint8_t direction);
	void moveCarriage(int16_t usteps);
	void moveCarriage(uint16_t usteps, uint8_t direction);
	void spinWheel();
	ww_printwheel queryPrintwheel();
	void setRepeatMode(uint8_t repeatMode);
	ww_status queryStatus();
	ww_operation queryOperation();
	void sendCode(uint8_t code);

private:
	uint init_;
	Uart9Bit* uart_;
	uint16_t buffer[4];
};

}; // namespace wheelwriter
