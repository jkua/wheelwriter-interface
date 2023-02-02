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
	uart_->write(buffer, 2);
	//uart_->read();
	return 0;
}
uint8_t Wheelwriter::sendCommand(ww_command command, uint8_t data) {
	buffer[1] = command;
	buffer[2] = data;
	uart_->write(buffer, 3);
	//uart_->read();
	return 0;
}
uint8_t Wheelwriter::sendCommand(ww_command command, uint8_t data1, uint8_t data2) {
	buffer[1] = command;
	buffer[2] = data1;
	buffer[3] = data2;
	uart_->write(buffer, 4);
	//uart_->read();
	return 0;
}

ww_model Wheelwriter::queryModel() {
	sendCommand(QUERY_MODEL);
	return UNKNOWN;
}
void Wheelwriter::typeCharacter(uint8_t wheelPosition) {
	// sendCommand(QUERY_STATUS);
  // delay(5);
  sendCommand(TYPE_CHARACTER_NO_ADVANCE, wheelPosition);
}
void Wheelwriter::typeCharacter(uint8_t wheelPosition, uint8_t advanceUsteps) {
  // sendCommand(QUERY_STATUS);
  // delay(5);
	sendCommand(TYPE_CHARACTER_AND_ADVANCE, wheelPosition, advanceUsteps);
}
void Wheelwriter::eraseCharater(uint8_t wheelPosition, uint8_t advanceUsteps) {
  sendCommand(ERASE_CHARACTER_AND_ADVANCE, wheelPosition, advanceUsteps);
}
void Wheelwriter::movePlaten(uint8_t usteps, uint8_t direction) {
  sendCommand(MOVE_PLATEN, usteps | direction);
}
void Wheelwriter::moveCarriage(uint16_t usteps, uint8_t direction) {
  sendCommand(MOVE_CARRIAGE, usteps | direction);
}