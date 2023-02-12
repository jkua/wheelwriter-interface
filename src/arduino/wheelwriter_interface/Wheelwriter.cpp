// Wheelwriter class for an IBM Wheelwriter
// via the option interface
//
// Copyright (c) 2023 John Kua <john@kua.fm>
//
#include "Wheelwriter.h"
#include <Arduino.h>

using namespace wheelwriter;

uint8_t Wheelwriter::sendCommand(ww_command command) {
	bufferOut_[1] = command;
	uint16_t response;

	// Send address
	uart_->write(bufferOut_[0]);
	uart_->read(); // Ignore self-transmission
	response = uart_->read();
	if (response != 0) {
		Serial.write("ERROR: sendCommand(): receive bad ACK to address byte! 0x");
		Serial.println(response, HEX);
	}

	// Send command
	uart_->write(bufferOut_[1]);
	uart_->read(); // Ignore self-transmission
	return uart_->read();
}
uint8_t Wheelwriter::sendCommand(ww_command command, uint8_t data) {
	bufferOut_[1] = command;
	bufferOut_[2] = data;
	uint16_t response;

	// Send address
	uart_->write(bufferOut_[0]);
	uart_->read(); // Ignore self-transmission
	response = uart_->read();
	if (response != 0) {
		Serial.write("ERROR: sendCommand(): receive bad ACK to address byte! 0x");
		Serial.println(response, HEX);
	}

	// Send command
	uart_->write(bufferOut_[1]);
	uart_->read(); // Ignore self-transmission
	response = uart_->read();
	if (response != 0) {
		Serial.write("ERROR: sendCommand(): receive bad ACK to command byte! 0x");
		Serial.println(response, HEX);
	}

	// Send data
	uart_->write(bufferOut_[2]);
	uart_->read(); // Ignore self-transmission
	return uart_->read();
}
uint8_t Wheelwriter::sendCommand(ww_command command, uint8_t data1, uint8_t data2) {
	bufferOut_[1] = command;
	bufferOut_[2] = data1;
	bufferOut_[3] = data2;
	uint16_t response;
	
	// Send address
	uart_->write(bufferOut_[0]);
	uart_->read(); // Ignore self-transmission
	response = uart_->read();
	if (response != 0) {
		Serial.write("ERROR: sendCommand(): receive bad ACK to address byte! 0x");
		Serial.println(response, HEX);
	}

	// Send command
	uart_->write(bufferOut_[1]);
	uart_->read(); // Ignore self-transmission
	response = uart_->read();
	if (response != 0) {
		Serial.write("ERROR: sendCommand(): receive bad ACK to command byte! 0x");
		Serial.println(response, HEX);
	}

	// Send data 1
	uart_->write(bufferOut_[2]);
	uart_->read(); // Ignore self-transmission
	response = uart_->read();
	if (response != 0) {
		Serial.write("ERROR: sendCommand(): receive bad ACK to data 1 byte! 0x");
		Serial.println(response, HEX);
	}

	// Send data 2
	uart_->write(bufferOut_[3]);
	uart_->read(); // Ignore self-transmission
	return uart_->read();
}
uint8_t Wheelwriter::readCommand(uint8_t blocking, uint8_t verbose) {
	uint16_t response;

	// Address
	if ((blocking == 0) && (!uart_->available())) {
		return 0;
	}
	bufferIn_[0] = uart_->read();
	
	if (bufferIn_[0] != 0x121) {
		if (verbose == 2) {
			Serial.write("\nERROR: readCommand(): expected address byte = 0x121! Got 0x");
			Serial.println(bufferIn_[0], HEX);
		}
		return 0;
	}
	if (verbose) {
		Serial.write("Address: 0x");
		Serial.print(bufferIn_[0], HEX);
	}

	// Address ACK
	response = uart_->read();
	if (response != 0) {
		if (verbose == 2) {
			Serial.write("\nERROR: readCommand(): expected address ACK = 0! Got 0x");
			Serial.println(response, HEX);
		}
		return 0;
	}

	// Command
	bufferIn_[1] = uart_->read();
	if (bufferIn_[1] > 0xe) {
		if (verbose == 2) {
			Serial.write("\nERROR: readCommand(): expected command < 0x0f! Got 0x");
			Serial.println(response, HEX);
		}
		return 0;
	}
	if (verbose) {
		Serial.write(", Command: 0x");
		Serial.print(bufferIn_[1], HEX);
	}

	uint8_t commandLength = ww_command_length[bufferIn_[1]];

	// Command ACK
	response = uart_->read();
	if (commandLength == 1) {
		if (verbose) {
			Serial.write(", Response: 0x");
			Serial.println(response, HEX);
		}
		return commandLength;
	}
	if ((commandLength > 1) && (response != 0)) {
		if (verbose == 2) {
			Serial.write("\nERROR: readCommand(): expected command ACK = 0! Got 0x");
			Serial.println(response, HEX);
		}
		return 0;	
	}

	// Data1
	bufferIn_[2] = uart_->read();
	if (verbose) {
		Serial.write(", Data1: 0x");
		Serial.print(bufferIn_[2], HEX);
	}

	// Data1 ACK
	response = uart_->read();
	if (commandLength == 2) {
		if (verbose) {
			Serial.write(", Response: 0x");
			Serial.println(response, HEX);
		}
		return commandLength;
	}
	if ((commandLength > 2) && (response != 0)) {
		if (verbose == 2) {
			Serial.write("\nERROR: readCommand(): expected data1 ACK = 0! Got 0x");
			Serial.println(response, HEX);
		}
		return 0;	
	}

	// Data2
	bufferIn_[3] = uart_->read();
	if (verbose) {
		Serial.write(", Data2: 0x");
		Serial.print(bufferIn_[3], HEX);
	}

	// Data2 ACK
	response = uart_->read();
	if (commandLength == 3) {
		if (verbose) {
			Serial.write(", Response: 0x");
			Serial.println(response, HEX);
		}
		return commandLength;
	}
	if ((commandLength > 3) && (response != 0)) {
		if (verbose == 2) {
			Serial.write("\nERROR: readCommand(): expected data2 ACK = 0! Got 0x");
			Serial.println(response, HEX);
		}
		return 0;	
	}
}
ww_keypress_type Wheelwriter::readKeypress(char& ascii, uint8_t blocking, uint8_t verbose) {
	uint8_t commandLength = readCommand(blocking, verbose);
	ww_keypress_type keypressType;
	ww_platen_direction platenDir;
	ww_carriage_direction carriageDir;

	if (commandLength > 0) {
		switch (bufferIn_[1]) {
			case TYPE_CHARACTER_AND_ADVANCE:
				ascii = usPrintwheel2AsciiTable[bufferIn_[2]];
				keypressType = CHARACTER_KEYPRESS;
				break;
			case MOVE_PLATEN:
				platenDir = (ww_platen_direction)(bufferIn_[2] & 0x80);
				if (platenDir == PLATEN_DIRECTION_UP) {
					ascii = '\n';
					keypressType = RETURN_KEYPRESS;
				}
				else {
					keypressType = NO_KEYPRESS;
				}
				break;
			case MOVE_CARRIAGE:
				carriageDir = (ww_carriage_direction)(bufferIn_[2] & 0x80);
				keypressType = SPACE_KEYPRESS;
				if (carriageDir == CARRIAGE_DIRECTION_RIGHT) {
					ascii = ' ';
				}
				else {
					ascii = 0x08;
				}
				break;
			default:
				keypressType = NO_KEYPRESS;
		}
	}
	else {
		keypressType = NO_COMMAND;
	}
	return keypressType;
}
void Wheelwriter::waitReady() {
	int count = 0;
	uint8_t status;
	while (status = queryStatus() != 0) {
		Serial.println(status, HEX);
		delayMicroseconds(1000);
	}
}
void Wheelwriter::readFlush() {
	uint8_t data;
	Serial.write("Wheelwriter::readFlush()\n");
	while (uart_->available()) {
		uart_->read();
		Serial.write("    0x");
		Serial.println(data, HEX);
	}
}

char Wheelwriter::ascii2Printwheel(char ascii) {
	// TODO: Add other languages
	return ascii2UsPrintwheelTable[ascii-0x20];
}
int16_t Wheelwriter::horizontalMicrospaces() {
	return horizontalMicrospaces_;
}
void Wheelwriter::setLeftMargin() {
	horizontalMicrospaces_ = 0;
}
void Wheelwriter::setCharSpace(uint16_t usteps) {
	charSpace_ = usteps;
}
void Wheelwriter::setLineSpace(uint8_t usteps) {
	lineSpace_ = usteps;
}
void Wheelwriter::setLineSpaceSingle(uint8_t usteps) {
	lineSpaceSingle_ = usteps;
}
void Wheelwriter::setLineSpacing(ww_linespacing spacing) {
	lineSpacing_ = spacing;
	updateLineSpace();
}
void Wheelwriter::updateLineSpace() {
	switch (lineSpacing_) {
		case LINESPACING_ONE:
			lineSpace_ = lineSpaceSingle_;
			break;
		case LINESPACING_ONE_POINT_FIVE:
			lineSpace_ = (uint16_t)lineSpaceSingle_ * 15 / 10;
			break;
		case LINESPACING_TWO:
			lineSpace_ = lineSpaceSingle_ * 2;
			break;
		case LINESPACING_THREE:
			lineSpace_ = lineSpaceSingle_ * 3;
			break;
	}
}
void Wheelwriter::setSpaceForWheel(ww_printwheel wheel) {
	switch (wheel) {
		case PROPORTIONAL:
			break;
		case CPI_15:
			break;
		case CPI_12:
		case NO_WHEEL:
			setLineSpaceSingle(16);
			updateLineSpace();
			setCharSpace(10);
			break;
		case CPI_10:
			setLineSpaceSingle(16);
			updateLineSpace();
			setCharSpace(12);
			break;
	}
}
void Wheelwriter::setSpaceForWheel() {
	wheel_ = queryPrintwheel();
	setSpaceForWheel(wheel_);
}
ww_model Wheelwriter::queryModel() {
	model_ = (ww_model)sendCommand(QUERY_MODEL);
	return model_;
}
void Wheelwriter::typeAsciiInPlace(char ascii, ww_typestyle style) {
	typeCharacterInPlace(ascii2Printwheel(ascii), style);
}
void Wheelwriter::typeAscii(char ascii, uint8_t advanceUsteps, ww_typestyle style) {
	typeCharacter(ascii2Printwheel(ascii), advanceUsteps, style);
}
void Wheelwriter::typeAscii(char ascii, ww_typestyle style) {
	typeAscii(ascii, charSpace_, style);
}
void Wheelwriter::typeAsciiString(char* string, uint8_t advanceUsteps, ww_typestyle style) {
	for (int i = 0; i < strlen(string); i++) {
		typeAscii(string[i], advanceUsteps, style);
	}
}
void Wheelwriter::typeAsciiString(char* string, ww_typestyle style) {
	typeAsciiString(string, charSpace_, style);
}
void Wheelwriter::typeCharacterInPlace(uint8_t wheelPosition, ww_typestyle style) {
	sendCommand(TYPE_CHARACTER_NO_ADVANCE, wheelPosition, 0);
	if ((style & 0xf0) == TYPESTYLE_UNDERLINE) {

		sendCommand(TYPE_CHARACTER_NO_ADVANCE, ascii2Printwheel('_'), 0);	
	}
	if ((style & 0x0f) == TYPESTYLE_BOLD) {
		moveCarriage(1);

		sendCommand(TYPE_CHARACTER_NO_ADVANCE, wheelPosition, 0);
	}
}
void Wheelwriter::typeCharacter(uint8_t wheelPosition, uint8_t advanceUsteps, ww_typestyle style) {
	if (style == TYPESTYLE_NORMAL) {

		sendCommand(TYPE_CHARACTER_AND_ADVANCE, wheelPosition, advanceUsteps);
		horizontalMicrospaces_ += advanceUsteps;
	}
	else {
		typeCharacterInPlace(wheelPosition, style);
		if ((style & 0x0f) == TYPESTYLE_BOLD) { 
			advanceUsteps -= 1;
		}
		moveCarriage(advanceUsteps);
	}
}
void Wheelwriter::typeCharacter(uint8_t wheelPosition, ww_typestyle style) {
	typeCharacter(wheelPosition, charSpace_, style);
}
void Wheelwriter::eraseCharacter(uint8_t wheelPosition, uint8_t advanceUsteps, ww_typestyle style) {
	sendCommand(ERASE_CHARACTER_AND_ADVANCE, wheelPosition, advanceUsteps);
}
void Wheelwriter::movePlaten(int8_t usteps) {
	ww_platen_direction direction = PLATEN_DIRECTION_UP;
	if (usteps < 0) {
		direction = PLATEN_DIRECTION_DOWN;
	}
	movePlaten(abs(usteps), direction);
}
void Wheelwriter::movePlaten(uint8_t usteps, ww_platen_direction direction) {
	usteps = usteps & 0x7f; // 7-bit value
	sendCommand(MOVE_PLATEN, usteps | direction);
}
void Wheelwriter::moveCarriage(int16_t usteps) {
	ww_carriage_direction direction = CARRIAGE_DIRECTION_RIGHT;
	if (usteps < 0) {
		direction = CARRIAGE_DIRECTION_LEFT;
	}
	moveCarriage(abs(usteps), direction);
}
void Wheelwriter::moveCarriage(uint16_t usteps, ww_carriage_direction direction) {
	uint16_t stepsAbs = usteps & 0x07ff; // 11-bit value
	uint8_t byte1 = (stepsAbs >> 8) | direction;
	uint8_t byte2 = stepsAbs & 0x00ff;

	sendCommand(MOVE_CARRIAGE, byte1, byte2);
	if (direction == CARRIAGE_DIRECTION_RIGHT) {
		horizontalMicrospaces_ += usteps;
	}
	else {
		horizontalMicrospaces_ -= usteps;
	}
}
void Wheelwriter::moveCarriageSpaces(int16_t spaces) {
	moveCarriage(spaces*charSpace_);
}
void Wheelwriter::carriageReturn() {
	moveCarriage(-horizontalMicrospaces_);
	horizontalMicrospaces_ = 0;
}
void Wheelwriter::lineFeed(ww_platen_direction direction) {
	uint8_t usteps;
	switch (lineSpacing_) {
		case LINESPACING_ONE:
			usteps = lineSpace_;
			break;
		case LINESPACING_ONE_POINT_FIVE:
			usteps = lineSpace_ + lineSpace_ / 2;
			break;
		case LINESPACING_TWO:
			usteps = lineSpace_ * 2;
			break;
		case LINESPACING_THREE:
			usteps = lineSpace_ * 3;
			break;
		default:
			usteps = lineSpace_;
	}
	movePlaten(usteps, direction);
}
void Wheelwriter::spinWheel() {
	sendCommand(SPIN_WHEEL);
}
ww_printwheel Wheelwriter::queryPrintwheel() {
	return (ww_printwheel)sendCommand(QUERY_WHEEL);
}
void Wheelwriter::setRepeatMode(ww_repeat_mode repeatMode) {
	sendCommand(SET_REPEAT_MODE, repeatMode);
}
ww_status Wheelwriter::queryStatus() {
	return (ww_status)sendCommand(QUERY_STATUS);
}
// This isn't a query - takes a single byte argument
// ww_operation Wheelwriter::queryOperation() {
// 	sendCommand(QUERY_OPERATION);
// 	return NO_OPERATION;
// }
void Wheelwriter::sendCode(ww_code code) {
	sendCommand(SEND_CODE, code);
}

