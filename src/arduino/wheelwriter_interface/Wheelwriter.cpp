// Wheelwriter class for an IBM Wheelwriter
// via the option interface
//
// Copyright (c) 2023 John Kua <john@kua.fm>
//
#include "Wheelwriter.h"
#include <Arduino.h>

using namespace wheelwriter;

uint8_t Wheelwriter::sendCommand(ww_command command) {
	buffer[1] = command;
	uint16_t response;

	// Send address
	uart_->write(buffer[0]);
	uart_->read(); // Ignore self-transmission
	response = uart_->read();
	if (response != 0) {
		Serial.write("ERROR: sendCommand(): receive bad ACK to address byte! 0x");
		Serial.println(response, HEX);
	}

	// Send command
	uart_->write(buffer[1]);
	uart_->read(); // Ignore self-transmission
	return uart_->read();
}
uint8_t Wheelwriter::sendCommand(ww_command command, uint8_t data) {
	buffer[1] = command;
	buffer[2] = data;
	uint16_t response;

	// Send address
	uart_->write(buffer[0]);
	uart_->read(); // Ignore self-transmission
	response = uart_->read();
	if (response != 0) {
		Serial.write("ERROR: sendCommand(): receive bad ACK to address byte! 0x");
		Serial.println(response, HEX);
	}

	// Send command
	uart_->write(buffer[1]);
	uart_->read(); // Ignore self-transmission
	response = uart_->read();
	if (response != 0) {
		Serial.write("ERROR: sendCommand(): receive bad ACK to command byte! 0x");
		Serial.println(response, HEX);
	}

	// Send data
	uart_->write(buffer[2]);
	uart_->read(); // Ignore self-transmission
	return uart_->read();
}
uint8_t Wheelwriter::sendCommand(ww_command command, uint8_t data1, uint8_t data2) {
	buffer[1] = command;
	buffer[2] = data1;
	buffer[3] = data2;
	uint16_t response;
	
	// Send address
	uart_->write(buffer[0]);
	uart_->read(); // Ignore self-transmission
	response = uart_->read();
	if (response != 0) {
		Serial.write("ERROR: sendCommand(): receive bad ACK to address byte! 0x");
		Serial.println(response, HEX);
	}

	// Send command
	uart_->write(buffer[1]);
	uart_->read(); // Ignore self-transmission
	response = uart_->read();
	if (response != 0) {
		Serial.write("ERROR: sendCommand(): receive bad ACK to command byte! 0x");
		Serial.println(response, HEX);
	}

	// Send data 1
	uart_->write(buffer[2]);
	uart_->read(); // Ignore self-transmission
	response = uart_->read();
	if (response != 0) {
		Serial.write("ERROR: sendCommand(): receive bad ACK to data 1 byte! 0x");
		Serial.println(response, HEX);
	}

	// Send data 2
	uart_->write(buffer[3]);
	uart_->read(); // Ignore self-transmission
	return uart_->read();
}
void Wheelwriter::waitReady() {
	int count = 0;
	while (sendCommand(QUERY_STATUS) != 0) {
		delayMicroseconds(1000);
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
void Wheelwriter::setLineSpacing(ww_linespacing spacing) {
	lineSpacing_ = spacing;
}

ww_model Wheelwriter::queryModel() {
	return (ww_model)sendCommand(QUERY_MODEL);
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
	waitReady();
	sendCommand(TYPE_CHARACTER_NO_ADVANCE, wheelPosition, 0);
	if ((style & 0xf0) == TYPESTYLE_UNDERLINE) {
		waitReady();
		sendCommand(TYPE_CHARACTER_NO_ADVANCE, ascii2Printwheel('_'), 0);	
	}
	if ((style & 0x0f) == TYPESTYLE_BOLD) {
		moveCarriage(1);
		waitReady();
		sendCommand(TYPE_CHARACTER_NO_ADVANCE, wheelPosition, 0);
	}
}
void Wheelwriter::typeCharacter(uint8_t wheelPosition, uint8_t advanceUsteps, ww_typestyle style) {
	if (style == TYPESTYLE_NORMAL) {
		waitReady();
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
	waitReady();
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
	waitReady();
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

	waitReady();
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
	waitReady();
	sendCommand(SPIN_WHEEL);
}
ww_printwheel Wheelwriter::queryPrintwheel() {
	sendCommand(QUERY_WHEEL);
	return NO_WHEEL;
}
void Wheelwriter::setRepeatMode(ww_repeat_mode repeatMode) {
	sendCommand(SET_REPEAT_MODE, repeatMode);
}
ww_status Wheelwriter::queryStatus() {
	sendCommand(QUERY_STATUS);
	return NO_STATUS;
}
// This isn't a query - takes a single byte argument
// ww_operation Wheelwriter::queryOperation() {
// 	sendCommand(QUERY_OPERATION);
// 	return NO_OPERATION;
// }
void Wheelwriter::sendCode(ww_code code) {
	sendCommand(SEND_CODE, code);
}

