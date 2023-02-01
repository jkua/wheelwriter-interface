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
	void typeCharacter(uint8_t wheelPosition);
	void typeCharacter(uint8_t wheelPosition, uint8_t advanceUsteps);
	void eraseCharater(uint8_t wheelPosition, uint8_t advanceUsteps);
	void movePlaten(uint8_t usteps, uint8_t direction);
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
