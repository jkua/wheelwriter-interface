// Wheelwriter class to control an IBM Wheelwriter
// via the option interface
//
// Copyright (c) 2023 John Kua <john@kua.fm>
//
#pragma once

#include "uart_9bit/Uart9bit.h"
#include <string>

namespace wheelwriter {

enum ww_model {
	UNKNOWN_MODEL = 0x00,
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

// Index is command value
// 0: Unknown command
// 1: Just the command is sent
// 2: Command + Data 1
// 3: Command + Data 2
static const uint8_t ww_command_length[16] = {1, 1, 3, 3, 
																				3, 2, 3, 1, 
																				1, 2, 2, 1,
																				3, 2, 2, 0};

static const char ww_command_part_strings[][8] = {
	"address",
	"command",
	"data1",
	"data2"
};
enum ww_platen_direction {
	PLATEN_DIRECTION_DOWN = 0x00,
	PLATEN_DIRECTION_UP = 0x80
};

enum ww_carriage_direction {
	CARRIAGE_DIRECTION_LEFT = 0x00,
	CARRIAGE_DIRECTION_RIGHT = 0x80
};

enum ww_printwheel {
	PROPORTIONAL = 0x08,
	CPI_15 = 0x10,
	CPI_12 = 0x20,
	NO_WHEEL = 0x21,
	CPI_10 = 0x40
};

enum ww_keyboard {
	KEYBOARD_US = 1,
	KEYBOARD_ASCII = 103,
	KEYBOARD_USSR = 231
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
	NO_OPERATION = 0x00,
	BACKSPACE = 0x04,
	ERASE = 0x05,
	CARRIAGE_RETURN = 0x07,
	TAB = 0x0a,
	TAB_SET = 0x12
};

enum ww_keycode {	// Mapped with a Wheelwriter 3 keyboard
  KEY_A = 0x04,
	KEY_B = 0x1f,
	KEY_C = 0x16,	// CTR
	KEY_D = 0x14,
	KEY_E = 0x12,
	KEY_F = 0x1c,
	KEY_G = 0x1d,
	KEY_H = 0x25,
	KEY_I = 0x2a,
	KEY_J = 0x24,
	KEY_K = 0x2c,
	KEY_L = 0x34,
	KEY_M = 0x26,
	KEY_N = 0x72,	// CAPS
	KEY_O = 0x32,
	KEY_P = 0x3a,
	KEY_Q = 0x02,	// Impression control?
	KEY_R = 0x1a,
	KEY_S = 0x0c,
	KEY_T = 0x1b,
	KEY_U = 0x22,  // UNDLN
	KEY_V = 0x1e,
	KEY_W = 0x0a,
	KEY_X = 0x0e,
	KEY_Y = 0x23,
	KEY_Z = 0x06,
	KEY_0 = 0x39,
	KEY_1 = 0x01,	// Spell check on/off
	KEY_2 = 0x09,	// Spell check - add word to dictionary
	KEY_3 = 0x11,  // Spell check - delete word from dictionary
	KEY_4 = 0x19,	// Spell check - adjust beep volume
	KEY_5 = 0x18,	// Printer on/off
	KEY_6 = 0x20,	// Printer - online
	KEY_7 = 0x21,	// Printer - feed
	KEY_8 = 0x29,	// Printer - set
	KEY_9 = 0x31,

	KEY_PLUS_MINUS = 0x00,
	KEY_HYPHEN = 0x38,
	KEY_EQUAL = 0x28,
	KEY_BACKSPACE = 0x50,			// BKSP 1/2
	KEY_TAB = 0x4a,
	KEY_HALF = 0x3b,		  			// ^2
	KEY_RIGHT_BRACKET = 0x2b,	// ^3
	KEY_RETURN = 0x56,
	KEY_SEMICOLON = 0x3c,			// Section mark
	KEY_APOSTROPHE = 0x3d,			// Paragraph mark
	KEY_COMMA = 0x2e,
	KEY_PERIOD = 0x36,
	KEY_SLASH = 0x3f,
	KEY_SPACE = 0x47,
	KEY_ERASE = 0x4f,

	KEY_MARGIN_RELEASE = 0x48,
	KEY_LEFT_MARGIN = 0x42,
	KEY_RIGHT_MARGIN = 0x4b,
	KEY_TAB_SET = 0x4c,
	KEY_TAB_CLEAR = 0x45,

	KEY_PAPER_UP = 0x51,
	KEY_PAPER_DOWN = 0x52,
	KEY_MICRO_UP = 0x54,
	KEY_MICRO_DOWN = 0x46,
	KEY_LINE_SPACE = 0x57,

	PAPER_FEED_LEVER = 0x49,

	SHIFT_MASK = 0x80,	  // High bit indicates Shift pressed or Shift locked
	CODE_KEY_RELEASED = 0x67 // Emitted when the Code key is released
};

enum ww_typestyle {
	TYPESTYLE_NORMAL = 0x00,
	TYPESTYLE_BOLD = 0x01,
	TYPESTYLE_UNDERLINE = 0x10,
	TYPESTYLE_BOLD_UNDERLINE = 0x11
};

enum ww_linespacing {
	LINESPACING_ONE = 0x00,
	LINESPACING_ONE_POINT_FIVE = 0x01,
	LINESPACING_TWO = 0x02,
	LINESPACING_THREE = 0x03
};

enum ww_keypress_type {
	NO_KEYPRESS = 0x00,
	CHARACTER_KEYPRESS = 0x01,
	SPACE_KEYPRESS = 0x02,
	RETURN_KEYPRESS = 0x03,
	CODE_KEYPRESS = 0x04,
	NO_COMMAND = 0xff
};

static const uint16_t WW_ADDRESS_BIT = 0x100;
static const uint8_t WW_MOTOR_CTRL_ADDR = 0x21;
static const uint8_t WW_PRINTWHEEL_MAX = 0x60;
static const uint8_t WW_CARRIAGE_ADVANCE_USTEP_MAX = 63;
static const uint8_t WW_PLATEN_ADVANCE_USTEP_MAX = 127;
static const uint8_t WW_MAX_VALID_COMMAND = SEND_CODE;

//------------------------------------------------------------------------------------------------
// ASCII (ISO 8859-1) to Wheelwriter US printwheel translation table
// The Wheelwriter printwheel code indicates the position of the character on the printwheel. 
// “a” (code 01) is at the 12 o’clock position of the printwheel. Going counter clockwise, 
// “n” (code 02) is next character on the printwheel followed by “r” (code 03), “m” (code 04),
// “c” (code 05), “s” (code 06), “d” (code 07), “h” (code 08), and so on.
//------------------------------------------------------------------------------------------------
static const char ascii2UsPrintwheelTable[2][256] = { 
// US 001-008
// col: 00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F    row:
//      NUL   SOH   STX   ETX   EOT   ENQ   ACK   BEL   BS    HT    LF    VT    FF    CR    SO    SI
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 00
//      DLE   DC1   DC2   DC3   DC4   NAK   SYN   ETB   CAN   EM    SUB   ESC   FS    GS    RS    US
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 10
//      SP     !     "     #     $     %     &     '     (     )     *     +     ,     -     .     /
			 0x00, 0x49, 0x4b, 0x38, 0x37, 0x39, 0x3f, 0x4c, 0x23, 0x16, 0x36, 0x3b, 0x0c, 0x0e, 0x57, 0x28, // 20
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
			 0x44, 0x3C, 0x43, 0x42, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x47, 0x00, 0x00, // B0
//     
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // C0 
//     
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // E0 
//     
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // D0 
//     
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},// F0 

// ASCII 103
// col: 00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F    row:
//      NUL   SOH   STX   ETX   EOT   ENQ   ACK   BEL   BS    HT    LF    VT    FF    CR    SO    SI
			{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 00
//      DLE   DC1   DC2   DC3   DC4   NAK   SYN   ETB   CAN   EM    SUB   ESC   FS    GS    RS    US
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 10
//      SP     !     "     #     $     %     &     '     (     )     *     +     ,     -     .     /
			 0x00, 0x49, 0x4b, 0x38, 0x37, 0x39, 0x3f, 0x4c, 0x23, 0x16, 0x36, 0x3b, 0x0c, 0x0e, 0x57, 0x28, // 20
//       0     1     2     3     4     5     6     7     8     9     :     ;     <     =     >     ?
			 0x30, 0x2e, 0x2f, 0x2c, 0x32, 0x31, 0x33, 0x35, 0x34, 0x2a ,0x4e, 0x50, 0x45, 0x4d, 0x46, 0x4a, // 30
//       @     A     B     C     D     E     F     G     H     I     J     K     L     M     N     O
			 0x3d, 0x20, 0x12, 0x1b, 0x1d, 0x1e, 0x11, 0x0f, 0x14, 0x1F, 0x21, 0x2b, 0x18, 0x24, 0x1a, 0x22, // 40
//       P     Q     R     S     T     U     V     W     X     Y     Z     [     \     ]     ^     _   
			 0x15, 0x3e, 0x17, 0x19, 0x1c, 0x10, 0x0d, 0x29, 0x2d, 0x26, 0x13, 0x41, 0x42, 0x40, 0x3a, 0x4f, // 50
//       `     a     b     c     d     e     f     g     h     i     j     k     l     m     n     o
			 0x3c, 0x01, 0x59, 0x05, 0x07, 0x60, 0x0a, 0x5a, 0x08, 0x5d, 0x56, 0x0b, 0x09, 0x04, 0x02, 0x5f, // 60
//       p     q     r     s     t     u     v     w     x     y     z     {     |     }     ~    DEL  
			 0x5c, 0x52, 0x03, 0x06, 0x5e, 0x5b, 0x53, 0x55, 0x51, 0x58, 0x54, 0x48, 0x43, 0x47, 0x44, 0x00, // 70
//     
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 80      
//     
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 90      
//                   ¢                             §                                                  
			 0x00, 0x00, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // A0      
//       °     ±     ²     ³                 ¶                                   ¼     ½              
			 0x44, 0x3C, 0x43, 0x42, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x47, 0x00, 0x00, // B0
//     
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // C0 
//     
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // E0 
//     
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // D0 
//     
			 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} // F0
};

static const char usPrintwheel2AsciiTable[97] = {
// SP    a     n     r     m     c     s     d     h     l     f     k     ,     V     _     G
	0x20, 0x61, 0x6E, 0x72, 0x6D, 0x63, 0x73, 0x64, 0x68, 0x6C, 0x66, 0x6B, 0x2C, 0x56, 0x2D, 0x47,
// U     F     B     Z     H     P     )     R     L     S     N     C     T     D     E     I   
	0x55, 0x46, 0x42, 0x5A, 0x48, 0x50, 0x29, 0x52, 0x4C, 0x53, 0x4E, 0x43, 0x54, 0x44, 0x45, 0x49,
// A     J     O     (     M     .     Y     ,     /     W     9     K     3     X     1     2
	0x41, 0x4A, 0x4F, 0x28, 0x4D, 0x2E, 0x59, 0x2C, 0x2F, 0x57, 0x39, 0x4B, 0x33, 0x58, 0x31, 0x32,
// 0     5     4     6     8     7     *     $     #     %     ¢     +     ±     @     Q     &
	0x30, 0x35, 0x34, 0x36, 0x38, 0x37, 0x2A, 0x24, 0x23, 0x25, 0xA2, 0x2B, 0xB1, 0x40, 0x51, 0x26,
// ]     [     ³     ²     °     §     ¶     ½     ¼     !     ?     "     '     =     :     -
	0x5D, 0x5B, 0xB3, 0xB2, 0xB0, 0xA7, 0xB6, 0xBD, 0xBC, 0x21, 0x3F, 0x22, 0x27, 0x3D, 0x3A, 0x5F,
// ;     x     q     v     z     w     j     .     y     b     g     u     p     i     t     o     e   
	0x3B, 0x78, 0x71, 0x76, 0x7A, 0x77, 0x6A, 0x2E, 0x79, 0x62, 0x67, 0x75, 0x70, 0x69, 0x74, 0x6F, 0x65};

class Wheelwriter {
public:
	Wheelwriter() : init_(0), typeStream(*this) {}
	void init(Uart9Bit* uart, uint16_t charSpace=10, uint8_t lineSpace=16) {
		uart_ = uart;
		model_ = UNKNOWN_MODEL;
		wheel_ = NO_WHEEL;
		setKeyboard(1);
		charSpace_ = charSpace;
		lineSpace_ = lineSpace;
		lineSpacing_ = LINESPACING_ONE;
		defaultAddress_ = WW_MOTOR_CTRL_ADDR;
		horizontalMicrospaces_ = 0;
		init_ = 1;
	}
	uint8_t sendCommand(ww_command command);
	uint8_t sendCommand(ww_command command, uint8_t data);
	uint8_t sendCommand(ww_command command, uint8_t data1, uint8_t data2);
	uint16_t sendCommand(uint8_t address, uint8_t command, uint8_t data1, uint8_t data2, uint8_t& error, uint8_t& failIndex, int ignoreErrors=0);
	uint16_t _sendByte(uint16_t byte);
	uint16_t _sendCommand(uint8_t address, uint8_t command, uint8_t data1, uint8_t data2, uint8_t& error, uint8_t& failIndex, int ignoreErrors=0);
	void _printCommandError(uint8_t error, uint8_t failIndex, uint16_t response);
	uint8_t readCommand(uint8_t blocking=1, uint8_t verbose=0);
	ww_keypress_type readKeypress(char& ascii, uint8_t blocking=1, uint8_t verbose=0);
	void waitReady(ww_command command);
	void readFlush(bool verbose=0);
	bool available();

	ww_model queryModel();
	ww_printwheel reset();
	void typeAsciiInPlace(char ascii, ww_typestyle style=TYPESTYLE_NORMAL);
	void typeAscii(char ascii, uint8_t advanceUsteps, ww_typestyle style=TYPESTYLE_NORMAL);
	void typeAscii(char ascii, ww_typestyle style=TYPESTYLE_NORMAL);
	void typeAsciiString(char* string, uint8_t advanceUsteps, ww_typestyle style=TYPESTYLE_NORMAL, bool newLine=false);
	void typeAsciiString(char* string, ww_typestyle style=TYPESTYLE_NORMAL, bool newLine=false);
	void typeAsciiLine(char* string, ww_typestyle style=TYPESTYLE_NORMAL);
	void typeCharacterInPlace(uint8_t wheelPosition, ww_typestyle style=TYPESTYLE_NORMAL);
	void typeCharacter(uint8_t wheelPosition, uint8_t advanceUsteps, ww_typestyle style=TYPESTYLE_NORMAL);
	void typeCharacter(uint8_t wheelPosition, ww_typestyle style=TYPESTYLE_NORMAL);
	void eraseCharacter(uint8_t wheelPosition, uint8_t advanceUsteps, ww_typestyle style=TYPESTYLE_NORMAL);
	void movePlaten(int8_t usteps);
	void movePlaten(uint8_t usteps, ww_platen_direction direction);
	void moveCarriage(int16_t usteps);
	void moveCarriage(uint16_t usteps, ww_carriage_direction direction);
	void moveCarriageSpaces(int16_t spaces);
	void carriageReturn();
	void lineFeed(ww_platen_direction direction=PLATEN_DIRECTION_UP);
	void spinWheel();
	ww_printwheel queryPrintwheel();
	void setRepeatMode(ww_repeat_mode repeatMode);
	ww_status queryStatus();
	// ww_operation queryOperation();
	void sendCode(ww_keycode code);

	char ascii2Printwheel(char ascii);
	void setKeyboard(uint16_t keyboard);
	int16_t horizontalMicrospaces();
	void setLeftMargin();
	void setCharSpace(uint16_t usteps);
	// Directly sets the line spacing
	void setLineSpace(uint8_t usteps);
	// Sets the single space distance
	void setLineSpaceSingle(uint8_t usteps);
	// Sets the number of lines per line feed (1, 1.5, 2, 3)
	void setLineSpacing(ww_linespacing spacing);
	// Sets lineSpace based on lineSpaceSingle and lineSpacing
	void updateLineSpace();
	// Set charSpace and lineSpace based on wheel pitch
	void setSpaceForWheel(ww_printwheel wheel);
	// Query wheel and set charSpace and lineSpace
	void setSpaceForWheel();

	// Get the default address
	uint8_t getDefaultAddress();
	// Set the default address
	void setDefaultAddress(uint8_t address);

	// Types a 10-length repeating pattern of characters
	void bufferTest(uint16_t numChars, uint8_t charsPerLine);
	// Types Lorem ipsum in a circle
	void circleTest();
	// Types all the characters in the printwheel (US keyboard)
	void characterTest(wheelwriter::ww_typestyle style=TYPESTYLE_NORMAL);
	// Generates a formatted printwheel sample
	void printwheelSample(uint8_t plusPosition, uint8_t underscorePosition);
	// Queries the typewriter and generates a JSON
	void queryToJson(std::string& json);
	// Read a line from the typewriter 
	// - timeout sends the line after the the last character entered. 0 disables the timeout
	// - corrected sends a corrected line if backspace is used. If false, the line is sent as entered, including backspaces
	// - Returns 1 if a line was read, 0 if timeout reached (characters in buffer available)
	int readLine(std::string& line, uint32_t timeout=0, bool newLine=true, bool corrected=true);

	class TypeStream {
	public:
		TypeStream(Wheelwriter& typewriter) : typewriter_(typewriter), useCaratAsControl_(true) {
			reset();
		}
		int operator<<(char inByte) {
			return type(inByte);
		}
		int type(char inByte);
		void reset();
		void setUseCaratAsControl(bool useCaratAsControl) {
			useCaratAsControl_ = useCaratAsControl;
		}
	private:
		void flushBuffer();
		int parseEscape(const std::string& buffer, wheelwriter::ww_typestyle& typestyle, wheelwriter::ww_linespacing& lineSpacing);
		size_t digitOffset(const std::string& buffer);

		Wheelwriter& typewriter_;
		ww_typestyle typestyle_;
		ww_linespacing lineSpacing_;
		std::string buffer_;
		enum State {
			NORMAL=1,
			CARAT,
			ESCAPE,
			CSI
		} state_;
		bool useCaratAsControl_;
	} typeStream;

private:
	uint init_;
	Uart9Bit* uart_;
	uint16_t bufferIn_[5];
	uint8_t defaultAddress_;
	ww_model model_;
	ww_printwheel wheel_;
	uint8_t keyboard_;
	uint8_t printwheelTableIndex_;
	uint16_t charSpace_;
	uint8_t lineSpace_;
	uint8_t lineSpaceSingle_;
	ww_linespacing lineSpacing_;
	int16_t horizontalMicrospaces_;

	char stringBuffer[256];
};

} // namespace wheelwriter
