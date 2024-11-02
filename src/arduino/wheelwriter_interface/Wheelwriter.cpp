// Wheelwriter class for an IBM Wheelwriter
// via the option interface
//
// Copyright (c) 2023 John Kua <john@kua.fm>
//
#include "Wheelwriter.h"
#include <Arduino.h>

using namespace wheelwriter;

uint8_t Wheelwriter::sendCommand(ww_command command) {
	bufferOut_[0] = (uint16_t)defaultAddress_ + 0x100;
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
	bufferOut_[0] = (uint16_t)defaultAddress_ + 0x100;
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
	bufferOut_[0] = (uint16_t)defaultAddress_ + 0x100;
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
uint8_t Wheelwriter::sendCommand(uint8_t address, uint8_t command, uint8_t data1, uint8_t data2, uint8_t* error, int ignoreErrors) {
		*error = 0;

		// Check if command is valid
		if (command > WW_MAX_VALID_COMMAND) {
			*error = 0x13;
			return command;
		}

		uint8_t commandLength = ww_command_length[command];

		bufferOut_[0] = address + 0x100;
	bufferOut_[1] = command;
	bufferOut_[2] = data1;
	bufferOut_[3] = data2;
	uint16_t response;
	
	// Send address
	uart_->write(bufferOut_[0]);
	uart_->read(); // Ignore self-transmission
	response = uart_->read();
	if (!ignoreErrors && (response != 0)) { // Bad ACK
		*error = 0x11;
		return 0;
	}

	// Send command
	uart_->write(bufferOut_[1]);
	uart_->read(); // Ignore self-transmission
	response = uart_->read();
	if (commandLength == 1) {
		return response;
	}
	if (!ignoreErrors && (response != 0)) { // Bad ACK
		*error = 0x11;
		return 1;
	}

	// Send data 1
	uart_->write(bufferOut_[2]);
	uart_->read(); // Ignore self-transmission
	response = uart_->read();
	if (commandLength == 2) {
		return response;
	}
	if (!ignoreErrors && (response != 0)) { // Bad ACK
		*error = 0x11;
		return 2;
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
	unsigned long startTime = millis();
	bufferIn_[0] = uart_->read();
	
	if (bufferIn_[0] != 0x121) {
		if (verbose == 2) {
			sprintf(stringBuffer, "\nERROR: readCommand(): expected address byte = 0x121! Got 0x%03x\n", bufferIn_[0]);
			Serial.write(stringBuffer);
		}
		return 0;
	}
	if (verbose) {
		sprintf(stringBuffer, "[%d] ADR 0x%03x", startTime, bufferIn_[0]);
		Serial.write(stringBuffer);
	}

	// Address ACK
	response = uart_->read();
	if (response != 0) {
		if (verbose == 2) {
			sprintf(stringBuffer, "\nERROR: readCommand(): expected address ACK = 0! Got 0x%02x\n", response);
			Serial.write(stringBuffer);
		}
		return 0;
	}

	// Command
	bufferIn_[1] = uart_->read();
	if (bufferIn_[1] > 0xe) {
		if (verbose == 2) {
			sprintf(stringBuffer, "\nERROR: readCommand(): expected command < 0x0f! Got 0x%02x\n", bufferIn_[1]);
			Serial.write(stringBuffer);
		}
		return 0;
	}
	if (verbose) {
		sprintf(stringBuffer, ", CMD: 0x%02x", bufferIn_[1]);
		Serial.write(stringBuffer);
	}

	uint8_t commandLength = ww_command_length[bufferIn_[1]];

	// Command ACK
	response = uart_->read();
	if (commandLength == 1) {
		if (verbose) {
			sprintf(stringBuffer, ", RSP: 0x%02x (%d ms)\n", response, millis()-startTime);
			Serial.write(stringBuffer);
		}
		return commandLength;
	}
	if ((commandLength > 1) && (response != 0)) {
		if (verbose == 2) {
			sprintf(stringBuffer, "\nERROR: readCommand(): expected command ACK = 0! Got 0x%02x\n", response);
			Serial.write(stringBuffer);
		}
		return 0;	
	}

	// Data1
	bufferIn_[2] = uart_->read();
	if (verbose) {
		sprintf(stringBuffer, ", DT1: 0x%02x", bufferIn_[2]);
		Serial.write(stringBuffer);
	}

	// Data1 ACK
	response = uart_->read();
	if (commandLength == 2) {
		if (verbose) {
			sprintf(stringBuffer, ", RSP: 0x%02x (%d ms)\n", response, millis()-startTime);
			Serial.write(stringBuffer);
		}
		return commandLength;
	}
	if ((commandLength > 2) && (response != 0)) {
		if (verbose == 2) {
			sprintf(stringBuffer, "\nERROR: readCommand(): expected data1 ACK = 0! Got 0x%02x\n", response);
			Serial.write(stringBuffer);
		}
		return 0;	
	}

	// Data2
	bufferIn_[3] = uart_->read();
	if (verbose) {
		sprintf(stringBuffer, ", DT2: 0x%02x", bufferIn_[3]);
		Serial.write(stringBuffer);
	}

	// Data2 ACK
	response = uart_->read();
	if (commandLength == 3) {
		if (verbose) {
			sprintf(stringBuffer, ", RSP: 0x%02x (%d ms)\n", response, millis()-startTime);
			Serial.write(stringBuffer);
		}
		return commandLength;
	}
	if ((commandLength > 3) && (response != 0)) {
		if (verbose == 2) {
			sprintf(stringBuffer, "\nERROR: readCommand(): expected data2 ACK = 0! Got 0x%02x\n", response);
			Serial.write(stringBuffer);
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
	return ascii2UsPrintwheelTable[printwheelTableIndex_][ascii];
}
void Wheelwriter::setKeyboard(uint16_t keyboard) {
	keyboard_ = keyboard;
	if (keyboard == KEYBOARD_ASCII) {
		printwheelTableIndex_ = 1;
	}
	else {
		printwheelTableIndex_ = 0;
	}
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
	movePlaten(lineSpace_, direction);
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

// Get the default address
uint8_t Wheelwriter::getDefaultAddress() {
	return defaultAddress_;
}
// Set the default address
void Wheelwriter::setDefaultAddress(uint8_t address) {
	defaultAddress_ = address;
}
// 
void Wheelwriter::bufferTest(uint16_t numChars, uint8_t charsPerLine) {
	char buffer[] = "123456789.";
	uint8_t index = 0;
	uint8_t bufferSize = strlen(buffer);
	uint16_t charsTyped = 0;

	this->readFlush();
	this->setSpaceForWheel();
	this->setLeftMargin();

	while (charsTyped < numChars) {
		index = index % bufferSize;
		this->typeAscii(buffer[index]);
		charsTyped++;
		index++;
		if ((charsTyped % charsPerLine) == 0) {
			this->carriageReturn();
			this->lineFeed();
		}
	}
	if (charsTyped % charsPerLine) {
		this->carriageReturn();
		this->lineFeed();
	}
}

void Wheelwriter::circleTest() {
  char buffer[] = "Hello, world! Lorem ipsum dolor sit amet. ";
  int dx[41] = {  15,  15,  14,  14,  11,  11,   8,   6,   4,   2, 
                  -1,  -3,  -5,  -7,  -9, -11, -13, -14, -14, -15, 
                 -16, -15, -14, -14, -13, -11,  -9,  -7,  -5,  -3, 
                  -1,   2,   4,   6,   8,  11,  11,  14,  14,  15,  15};
  int dy[41] = {  -1,  -3,  -4,  -7,  -7, -10, -10, -11, -12, -12, 
                 -12, -12, -12, -10, -10,  -9,  -7,  -5,  -4,  -2, 
                   0,   2,   4,   5,   7,   9,  10,  10,  12,  12, 
                  12,  12,  12,  11,  10,  10,   7,   7,   4,   3,   1};
  setLeftMargin();

  // Move to center
  moveCarriage(100);
  for (int i = 0; i < strlen(buffer); i++) {
    typeAsciiInPlace(buffer[i]);
    if (i == strlen(buffer) - 1) break;
    moveCarriage(dx[i]);
    movePlaten(-dy[i]);
  }
  carriageReturn();
  movePlaten(127);
  movePlaten(127);
}

void Wheelwriter::characterTest(wheelwriter::ww_typestyle style) {
  char buffer1[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  char buffer2[] = "abcdefghijklmnopqrstuvwxyz";
  char buffer3[] = "1234567890-=!@#$%\xa2&*()_+";
  char buffer4[] = "\xbc\xbd[]:;\"',.?/\xb0\xb1\xb2\xb3\xa7\xb6";
  char* buffers[4] = {buffer1, buffer2, buffer3, buffer4};

  readFlush();
  setSpaceForWheel();
  setLeftMargin();

  for (int i = 0; i < 4; i++) {
    typeAsciiString(buffers[i], style);
    carriageReturn();
    lineFeed();
  }
}

void Wheelwriter::printwheelSample(uint8_t plusPosition, uint8_t underscorePosition) {
  // A printwheel has 96 (0x60) characters
  // This prints in a pair 16 x 6 arrays (regular and bold) with a border of alignment marks

  wheelwriter::ww_typestyle typestyle = wheelwriter::TYPESTYLE_NORMAL;
  readFlush();
  setSpaceForWheel();
  setLeftMargin();

  // Row 0
  typeCharacter(plusPosition, typestyle);
  typeCharacter(plusPosition, typestyle);
  typeCharacter(underscorePosition, typestyle);
  moveCarriageSpaces(7);
  typeCharacter(plusPosition, typestyle);
  moveCarriageSpaces(9);
  typeCharacter(plusPosition, typestyle);
  carriageReturn();
  lineFeed();

  // Row 1
  typeCharacter(plusPosition, typestyle);
  carriageReturn();
  lineFeed();

  // Row 2-14
  for (int sample = 0; sample < 2; sample++) {
    switch (sample) {
      case 0:
        typestyle = wheelwriter::TYPESTYLE_NORMAL;
        break;
      case 1:
        typestyle = wheelwriter::TYPESTYLE_BOLD;
        break;
    }
    // Type all 96 printwheel positions in a 16 x 8 array
    uint8_t wheelPosition = 1;
    for (int row = 0; row < 6; row++) {
      moveCarriageSpaces(2);
      for (int i = 0; i < 16; i++, wheelPosition++) {
        typeCharacter(wheelPosition, typestyle);
        // Space after the 8th character
        if (i == 7) {
          moveCarriageSpaces(1);
        }
      }
      carriageReturn();
      lineFeed();
    }
    // Space between samples
    if (sample == 0) {
      typeCharacter(plusPosition, typestyle);
      moveCarriageSpaces(19);
      typeCharacter(plusPosition, typestyle);
      carriageReturn();
      lineFeed();
    }
  }
  typestyle = wheelwriter::TYPESTYLE_NORMAL;
  
  // Row 15
  carriageReturn();
  lineFeed();

  // Row 16
  typeCharacter(plusPosition, typestyle);
  moveCarriageSpaces(9);
  typeCharacter(plusPosition, typestyle);
  moveCarriageSpaces(9);
  typeCharacter(plusPosition, typestyle);
  carriageReturn();
  lineFeed();
}

void Wheelwriter::queryToJson(std::string& json) {
	json.clear();

	json = "{";
	json += "\"model\":";
	json += std::to_string(queryModel());
	json += ",";
	json += "\"wheel\":";
	json += std::to_string(queryPrintwheel());
	json += ",";
	json += "\"status\":";
	json += std::to_string(queryStatus());
	json += "}";
}
 int Wheelwriter::readLine(std::string& line, uint32_t timeout) {
	uint32_t startTime = millis();
	uint32_t elapsedTime = 0;
	Serial.write("Wheelwriter::readLine()\n");
  readFlush();
  line.clear();

	while (elapsedTime < timeout || timeout == 0) {
		char ascii;
		ww_keypress_type keypressType = readKeypress(ascii, 0, 0);

		if (keypressType == CHARACTER_KEYPRESS ||
			  keypressType == SPACE_KEYPRESS ||
			  keypressType == RETURN_KEYPRESS) {
			line += ascii;
			startTime = millis();
			if (ascii == '\b') {
				Serial.print("\\b");
			}
			else if (ascii == '\n') {
				Serial.println("\\n");
			}
			else {
				Serial.print(ascii);
			}
		}
		if (keypressType == RETURN_KEYPRESS) {
			Serial.write("*** Got return keypress\n");
			return 1;
		}
		elapsedTime = millis() - startTime;
	}
	Serial.write("\n*** Timeout\n");
	return 0;
}

// =================
// Wheelwriter::TypeStream class
// =================

int Wheelwriter::TypeStream::type(char inByte) {
	switch (state_) {
		case NORMAL: {
			// Control sequence start - ^
			if (useCaratAsControl_ && inByte == '^') {
				buffer_ += inByte;
				state_ = CARAT;
			}
			// EOT (CTRL-D)
			else if (inByte == 0x04) {
				reset();
				return 0;
			}
			// New line
			else if (inByte == 0x0a) {
				typewriter_.carriageReturn();
				typewriter_.lineFeed();
			}
			// Escape character
			else if (inByte == 0x1b) {
				buffer_ += inByte;
				state_ = ESCAPE;
			}
			else {
				typewriter_.typeAscii(inByte, typestyle_);
			}
			break;
		}
		case CARAT: {
			// EOT - ^D
			if ((inByte == 'd') || (inByte == 'D')) {
				reset();
				return 0;
			}
			// ANSI escape sequence - ^[
			else if (inByte == '[') {
				buffer_ += inByte;
				state_ = ESCAPE;
			}
			// Invalid control sequnece
			else {
				buffer_ += inByte;
				flushBuffer();
				state_ = NORMAL;
			}
			break;
		}
		case ESCAPE: {
			// ANSI escape sequence - ^[[<value> m
			if (inByte == '[') {
				buffer_ += inByte;
				state_ = CSI;
			}
			else {
				buffer_ += inByte;
				flushBuffer();
				state_ = NORMAL;
			}
			break;
		}
		case CSI: {
			if (inByte == 'm') {
				if (parseEscape(buffer_, typestyle_, lineSpacing_)) {
					typewriter_.setLineSpacing(lineSpacing_);
					buffer_.clear();
				}
				else {
					flushBuffer();
				}
				state_ = NORMAL;
			}
			else if (isdigit(inByte)) {
				buffer_ += inByte;
				// Too many digits
				if ((buffer_.size()-digitOffset(buffer_)) > 2) {
					flushBuffer();
					state_ = NORMAL;
				}
			}
			// Invalid ANSI escape sequence
			else {
				buffer_ += inByte;
				flushBuffer();
				state_ = NORMAL;
			}
			break;
		}
	}
	return state_;
}
int Wheelwriter::TypeStream::parseEscape(const std::string& buffer, wheelwriter::ww_typestyle& typestyle, wheelwriter::ww_linespacing& lineSpacing) {
	size_t offset = digitOffset(buffer);

	const char* start = buffer.c_str()+offset;
  char* end;
  long value = strtol(start, &end, 10);
  if (end != start) {
    switch (value) {
      case 0:  // Normal
        typestyle = wheelwriter::TYPESTYLE_NORMAL;
        lineSpacing = wheelwriter::LINESPACING_ONE;
        break;
      case 1:  // Bold
        typestyle = (wheelwriter::ww_typestyle)((uint8_t)typestyle | (uint8_t)wheelwriter::TYPESTYLE_BOLD);
        break;
      case 4:  // Underline
        typestyle = (wheelwriter::ww_typestyle)((uint8_t)typestyle | (uint8_t)wheelwriter::TYPESTYLE_UNDERLINE);
        break;
      case 10: // Single space
        lineSpacing = wheelwriter::LINESPACING_ONE;
        break;
      case 11: // 1.5 space
        lineSpacing = wheelwriter::LINESPACING_ONE_POINT_FIVE;
        break;
      case 12: // Double space
        lineSpacing = wheelwriter::LINESPACING_TWO;
        break;
      case 13: // Triple space
        lineSpacing = wheelwriter::LINESPACING_THREE;
        break;
      case 22: // Not bold
        typestyle = (wheelwriter::ww_typestyle)((uint8_t)typestyle & 0xf0);
        break;
      case 24: // Not underlined
        typestyle = (wheelwriter::ww_typestyle)((uint8_t)typestyle & 0x0f);
        break;
      default: // Invalid value
       	return 0;
    }
    return 1; // Valid escape
  }
  return 0; // Invalid escape sequence - in theory this should never be reached if the front end filtering is working
}
size_t Wheelwriter::TypeStream::digitOffset(const std::string& buffer) {
	for (size_t i = 0; i < buffer.size(); i++) {
		if (isdigit(buffer[i])) {
			return i;
		}
	}
	return buffer.size();
}
void Wheelwriter::TypeStream::flushBuffer() {
	for (char c : buffer_) {
		typewriter_.typeAscii(c, typestyle_);
	}
	buffer_.clear();
}
void Wheelwriter::TypeStream::reset() {
	buffer_.clear();
	typestyle_ = TYPESTYLE_NORMAL; 
	lineSpacing_ = LINESPACING_ONE;
	state_ = NORMAL;
}
