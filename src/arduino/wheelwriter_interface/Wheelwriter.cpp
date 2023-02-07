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
	uart_->write(buffer[0]);
	delayMicroseconds(300);
	uart_->write(buffer[1]);
	delayMicroseconds(300);
	//uart_->read();
	delay(450);
	return 0;
}
uint8_t Wheelwriter::sendCommand(ww_command command, uint8_t data) {
	buffer[1] = command;
	buffer[2] = data;
	// uart_->write(buffer, 3);
	//uart_->read();
	uart_->write(buffer[0]);
	delayMicroseconds(300);
	uart_->write(buffer[1]);
	delayMicroseconds(300);
	uart_->write(buffer[2]);
	delayMicroseconds(300);

	delay(450);
	return 0;
}
uint8_t Wheelwriter::sendCommand(ww_command command, uint8_t data1, uint8_t data2) {
	buffer[1] = command;
	buffer[2] = data1;
	buffer[3] = data2;
	// uart_->write(buffer, 4);
	uart_->write(buffer[0]);
	delayMicroseconds(300);
	uart_->write(buffer[1]);
	delayMicroseconds(300);
	uart_->write(buffer[2]);
	delayMicroseconds(300);
	uart_->write(buffer[3]);
	delayMicroseconds(300);
	//uart_->read();

	delay(450);
	return 0;
}

char Wheelwriter::ascii2Printwheel(char ascii) {
	// TODO: Add other languages
	return ascii2UsPrintwheelTable[ascii-0x20];
}

ww_model Wheelwriter::queryModel() {
	sendCommand(QUERY_MODEL);
	return UNKNOWN_MODEL;
}
void Wheelwriter::typeAscii(char ascii, ww_typestyle style) {
	typeCharacter(ascii2Printwheel(ascii), style);
}
void Wheelwriter::typeAscii(char ascii, uint8_t advanceUsteps, ww_typestyle style) {
	typeCharacter(ascii2Printwheel(ascii), advanceUsteps, style);
}
void Wheelwriter::typeAsciiString(char* string, uint8_t advanceUsteps, ww_typestyle style) {
	for (int i = 0; i < strlen(string); i++) {
		typeAscii(string[i], advanceUsteps, style);
	}
}
void Wheelwriter::typeCharacter(uint8_t wheelPosition, ww_typestyle style) {
	// sendCommand(QUERY_STATUS);
	// delayMicroseconds(450);
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
	// sendCommand(QUERY_STATUS);
	// delay(5);
	if (style == TYPESTYLE_NORMAL) {
		sendCommand(TYPE_CHARACTER_AND_ADVANCE, wheelPosition, advanceUsteps);
	}
	else {
		typeCharacter(wheelPosition, style);
		if ((style & 0x0f) == TYPESTYLE_BOLD) { 
			moveCarriage(advanceUsteps-1);
		}
		else {
			moveCarriage(advanceUsteps);
		}
	}
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
}
void Wheelwriter::spinWheel() {
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
ww_operation Wheelwriter::queryOperation() {
	sendCommand(QUERY_OPERATION);
	return NO_OPERATION;
}
void Wheelwriter::sendCode(ww_code code) {
	sendCommand(SEND_CODE, code);
}

