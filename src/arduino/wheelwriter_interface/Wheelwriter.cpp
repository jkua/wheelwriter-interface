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
	return 0;
}

ww_model Wheelwriter::queryModel() {
	sendCommand(QUERY_MODEL);
	return UNKNOWN;
}
void Wheelwriter::typeAscii(uint8_t ascii) {
	typeCharacter(ascii2Printwheel[ascii-0x20]);
}
void Wheelwriter::typeAscii(uint8_t ascii, uint8_t advanceUsteps) {
	typeCharacter(ascii2Printwheel[ascii-0x20], advanceUsteps);
}
void Wheelwriter::typeCharacter(uint8_t wheelPosition) {
	// sendCommand(QUERY_STATUS);
	// delayMicroseconds(450);
	sendCommand(TYPE_CHARACTER_NO_ADVANCE, wheelPosition, 0);
  delay(450);
}
void Wheelwriter::typeCharacter(uint8_t wheelPosition, uint8_t advanceUsteps) {
	// sendCommand(QUERY_STATUS);
	// delay(5);
	sendCommand(TYPE_CHARACTER_AND_ADVANCE, wheelPosition, advanceUsteps);
  delay(450);
}
void Wheelwriter::eraseCharacter(uint8_t wheelPosition, uint8_t advanceUsteps) {
	sendCommand(ERASE_CHARACTER_AND_ADVANCE, wheelPosition, advanceUsteps);
  delay(450);
}
void Wheelwriter::movePlaten(int8_t usteps) {
	uint8_t direction = WW_PLATEN_DIRECTION_UP;
	if (usteps < 0) {
		direction = WW_PLATEN_DIRECTION_DOWN;
	}
	movePlaten(abs(usteps), direction);
}
void Wheelwriter::movePlaten(uint8_t usteps, uint8_t direction) {
	usteps = usteps & 0x7f; // 7-bit value
	sendCommand(MOVE_PLATEN, usteps | direction);
  delay(450);
}
void Wheelwriter::moveCarriage(int16_t usteps) {
	uint8_t direction = WW_CARRIAGE_DIRECTION_RIGHT;
	if (usteps < 0) {
		direction = WW_CARRIAGE_DIRECTION_LEFT;
	}
	moveCarriage(abs(usteps), direction);
}
void Wheelwriter::moveCarriage(uint16_t usteps, uint8_t direction) {
	uint16_t stepsAbs = usteps & 0x07ff; // 11-bit value
	uint8_t byte1 = (stepsAbs >> 8) | direction;
	uint8_t byte2 = stepsAbs & 0x00ff;
	sendCommand(MOVE_CARRIAGE, byte1, byte2);
  delay(450);
}
ww_status Wheelwriter::queryStatus() {
	sendCommand(QUERY_STATUS);
	return NO_STATUS;
}