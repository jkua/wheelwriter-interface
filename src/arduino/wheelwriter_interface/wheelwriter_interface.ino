// Wheelwriter interface for an Arduino Nano RP2040 Connect
// Copyright (c) 2023 John Kua <john@kua.fm>
//
#include "uart_9bit/Uart9bit.h"
#include "Wheelwriter.h"

Uart9Bit uart;
wheelwriter::Wheelwriter typewriter;
int inByte = 0;
char inputBuffer[64];

void setup() {
  gpio_set_drive_strength(25, GPIO_DRIVE_STRENGTH_12MA);

  // Level converter output enable - set to disable
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);

  // UART RX pin - remove when implemented in Uart9Bit
  pinMode(3, INPUT);

  // UART TX
  uint sm_tx = 0;
  uint pin_tx = 25; // D2/GPIO25
  uint sm_rx = 1;
  uint pin_rx = 15; // D3/GPIO15
  uart.init(pio0, sm_tx, pin_tx, sm_rx, pin_rx, 185700);
  typewriter.init(&uart);

  // USB Serial
  Serial.begin(115200);
}

void loop() {
  // char hexString[7] = "0x0000";
  // uint16_t inWord = uart.read();
  // sprintf(hexString, "0x%02X", inWord);
  // Serial.println(hexString);

  Serial.write("\n### Wheelwriter Interface ###\n");
  Serial.write("[READY]\n");

  while (Serial.available() == 0) {
    delay(100);
  }

  if (Serial.available() > 0) {
    String inString = Serial.readStringUntil('\n');
    inString.toLowerCase();
    inString.toCharArray(inputBuffer, 64);

    const char delim[2] = " ";
    char* token = strtok(inputBuffer, delim);

    if (strlen(token) == 0) {
      // Do nothing, loop back to the ready prompt
    }
    else if ((token[0] == 'h') || (strcmp(token, "help") == 0)) {
      Serial.write("Available functions:\n");
      Serial.write("help - print this help text\n");
      Serial.write("buffer - execute the buffer test\n");
      Serial.write("char - execute the character test\n");
      Serial.write("circle - execute the circle test\n");
      Serial.write("keyboard - read input from the keyboard\n");
      Serial.write("loopback - execute the loopback test\n");
      Serial.write("query - query typewriter for information\n");
      Serial.write("raw - send raw commands\n");
      Serial.write("read - read bus commands\n");
      Serial.write("relay - relay bus commands\n");
      Serial.write("sample - print a type sample\n");
      Serial.write("type - type characters on the typewriter\n");
    }
    else if (strcmp(token, "buffer") == 0) {
      uint16_t numChars = 10;
      uint8_t charsPerLine = 80;
      token = strtok(NULL, delim);
      char* end;
      int i = 0;
      while (token != NULL) {
        long value = strtol(token, &end, 0);
        if (token != end) {
          if (i == 0) {
            numChars = value;
            i++;
          }
          else {
            charsPerLine = value;
            break;
          }
        }
        token = strtok(NULL, delim);
      }
      Serial.write("[FUNCTION] Buffer Test ");
      Serial.write("| # Characters: ");
      Serial.print(numChars);
      Serial.write(", Characters Per Line: ");
      Serial.println(charsPerLine);
      bufferTest(numChars, charsPerLine);
    }
    else if (strcmp(token, "char") == 0) {
      uint8_t typestyle = wheelwriter::TYPESTYLE_NORMAL;
      token = strtok(NULL, delim);
      while (token != NULL) {
        if (strcmp(token, "bold") == 0) {
          typestyle = typestyle | wheelwriter::TYPESTYLE_BOLD;
        }
        else if (strcmp(token, "underline") == 0) {
          typestyle = typestyle | wheelwriter::TYPESTYLE_UNDERLINE; 
        }
        token = strtok(NULL, delim);
      }
      Serial.write("[FUNCTION] Character Test - ");
      if (typestyle == wheelwriter::TYPESTYLE_NORMAL) {
        Serial.write("Normal\n");
      }
      else {
        if (typestyle & 0x0F) {
          Serial.write("Bold ");
        }
        if (typestyle & 0xF0) {
          Serial.write("Underline");
        }
        Serial.write("\n");
      }
      characterTest((wheelwriter::ww_typestyle)typestyle);
    }
    else if (strcmp(token, "circle") == 0) {
      Serial.write("[FUNCTION] Circle Test\n");
      circleTest();
    }
    else if (strcmp(token, "keyboard") == 0) {
      uint8_t verbose = 0;
      token = strtok(NULL, delim);
      char* end;
      while (token != NULL) {
        if (strlen(token) >= 0) {
          if ((token[0] == 'v') && (verbose == 0)) {
            verbose = 1;
          }
          if (strcmp(token, "vv") == 0) {
            verbose = 2;
          }
        }
        token = strtok(NULL, delim);
      }
      Serial.write("[FUNCTION] Keyboard Input");
      if (verbose == 0) {
        Serial.write('\n');
      }
      else {
        Serial.write(" - verbose: ");
        Serial.println(verbose);
      }
      keyboardFunction(verbose);
    }
    else if (strcmp(token, "loopback") == 0) {
      Serial.write("[FUNCTION] Loopback Test\n");
      loopbackTest();
    }
    else if (strcmp(token, "query") == 0) {
      Serial.write("[FUNCTION] Query\n");
      queryFunction();
    }
    else if (strcmp(token, "raw") == 0) {
      Serial.write("[FUNCTION] Command\n");
      rawCommandFunction();
    }
    else if (strcmp(token, "read") == 0) {
      Serial.write("[FUNCTION] Read Bus\n");
      readFunction();
    }
    else if (strcmp(token, "relay") == 0) {
      Serial.write("[FUNCTION] Relay commands\n");
      relayFunction();
    }
    
    else if (strcmp(token, "sample") == 0) {
      uint8_t plusPosition = 0x3b;
      uint8_t underscorePosition = 0x4f;
      token = strtok(NULL, delim);
      char* end;
      int i = 0;
      while (token != NULL) {
        long value = strtol(token, &end, 0);
        if (token != end) {
          if (i == 0) {
            plusPosition = value;
            i++;
          }
          else {
            underscorePosition = value;
            break;
          }
        }
        token = strtok(NULL, delim);
      }
      Serial.write("[FUNCTION] Printwheel Sample ");
      Serial.write("| plus: 0x");
      Serial.print(plusPosition, HEX);
      Serial.write(", - underscore: 0x");
      Serial.println(underscorePosition, HEX);
      printwheelSample(plusPosition, underscorePosition);
    }
    else if (strcmp(token, "type") == 0) {
      uint8_t keyboard = 1;
      uint8_t useCaratAsControl = 1;
      token = strtok(NULL, delim);
      char* end;
      int i = 0;
      while (token != NULL) {
        uint8_t value = atoi(token);
        if (i == 0) {
          keyboard = value;
        }
        else {
          useCaratAsControl = value;
          break;
        }
        i++;
        token = strtok(NULL, delim);
      }
      Serial.write("[FUNCTION] Type ");
      Serial.write("| Keyboard: ");
      Serial.print(keyboard);
      Serial.write(", UseCaratAsControl: ");
      Serial.println(useCaratAsControl);
      typeFunction(keyboard, useCaratAsControl);
    }
    else {
      Serial.write("[UNKNOWN FUNCTION] Enter 'help' to see a list of available commands\n");
    }
    // uint16_t buffer[3];
    // buffer[0] = 0x121;
    // buffer[1] = 0x020;
    // buffer[1] = 0x000;
    // uart.write(buffer, 3);
    // uart.write(0x121);
    
    // typewriter.queryStatus();
    // typewriter.queryModel();

    // typewriter.typeCharacter(0x01);
    // delay(500);
    // typewriter.moveCarriage(10, wheelwriter::CARRIAGE_DIRECTION_RIGHT);
    // delay(500);
    // typewriter.typeCharacter(0x59);
    // delay(500);
    // typewriter.moveCarriage(10, wheelwriter::CARRIAGE_DIRECTION_RIGHT);
    // delay(500);
    // typewriter.typeCharacter(0x05);
    // delay(500);
    // typewriter.moveCarriage(10, wheelwriter::CARRIAGE_DIRECTION_RIGHT);

    // typewriter.typeCharacter(0x01, 10);
    // delay(500);
    // typewriter.typeCharacter(0x59, 10);
    // delay(500);
    // typewriter.typeCharacter(0x05, 10);
    // delay(500);
  }
}

void bufferTest(uint16_t numChars, uint8_t charsPerLine) {
  char buffer[] = "123456789.";
  uint8_t index = 0;
  uint8_t bufferSize = strlen(buffer);
  uint16_t charsTyped = 0;

  typewriter.readFlush();
  typewriter.setSpaceForWheel();
  typewriter.setLeftMargin();

  while (charsTyped < numChars) {
    index = index % bufferSize;
    typewriter.typeAscii(buffer[index]);
    charsTyped++;
    index++;
    if ((charsTyped % charsPerLine) == 0) {
      typewriter.carriageReturn();
      typewriter.lineFeed();
    }
  }
  if (charsTyped % charsPerLine) {
    typewriter.carriageReturn();
    typewriter.lineFeed();
  }
}

void circleTest() {
  char buffer[] = "Hello, world! Lorem ipsum dolor sit amet. ";
  int dx[41] = {  15,  15,  14,  14,  11,  11,   8,   6,   4,   2, 
                  -1,  -3,  -5,  -7,  -9, -11, -13, -14, -14, -15, 
                 -16, -15, -14, -14, -13, -11,  -9,  -7,  -5,  -3, 
                  -1,   2,   4,   6,   8,  11,  11,  14,  14,  15,  15};
  int dy[41] = {  -1,  -3,  -4,  -7,  -7, -10, -10, -11, -12, -12, 
                 -12, -12, -12, -10, -10,  -9,  -7,  -5,  -4,  -2, 
                   0,   2,   4,   5,   7,   9,  10,  10,  12,  12, 
                  12,  12,  12,  11,  10,  10,   7,   7,   4,   3,   1};
  typewriter.setLeftMargin();

  // Move to center
  typewriter.moveCarriage(100);
  for (int i = 0; i < strlen(buffer); i++) {
    typewriter.typeAsciiInPlace(buffer[i]);
    if (i == strlen(buffer) - 1) break;
    typewriter.moveCarriage(dx[i]);
    typewriter.movePlaten(-dy[i]);
  }
  typewriter.carriageReturn();
  typewriter.movePlaten(127);
  typewriter.movePlaten(127);
}

void characterTest(wheelwriter::ww_typestyle style) {
  char buffer1[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  char buffer2[] = "abcdefghijklmnopqrstuvwxyz";
  char buffer3[] = "1234567890-=!@#$%\xa2&*()_+";
  char buffer4[] = "\xbc\xbd[]:;\"',.?/\xb0\xb1\xb2\xb3\xa7\xb6";
  char* buffers[4] = {buffer1, buffer2, buffer3, buffer4};

  typewriter.readFlush();
  typewriter.setSpaceForWheel();
  typewriter.setLeftMargin();

  for (int i = 0; i < 4; i++) {
    typewriter.typeAsciiString(buffers[i], style);
    typewriter.carriageReturn();
    typewriter.lineFeed();
  }
}

void keyboardFunction(uint8_t verbose) {
  typewriter.readFlush();

  Serial.write("[BEGIN]\n");
  while (true) {
    if (Serial.available()) {
      String inString = Serial.readStringUntil('\n');
      inString.toLowerCase();
      inString.toCharArray(inputBuffer, 64);

      // End with 'q' or EOT (CTRL-D)
      if ((strlen(inputBuffer) == 1) && ((inputBuffer[0] == 'q') || (inputBuffer[0] == 0x04))) {
        break;
      }
    }
    char ascii;
    uint8_t blocking = 0;
    wheelwriter::ww_keypress_type keypressType = typewriter.readKeypress(ascii, blocking, verbose);
    if (keypressType == wheelwriter::NO_COMMAND) {
      continue;
    }
    if (keypressType != wheelwriter::NO_KEYPRESS) {
      if (verbose) {
        Serial.write("Keypress: 0x");
        Serial.println(ascii, HEX);
      }
      else {
        Serial.write(ascii);
      }
    }
    else {
      if (verbose == 2) {
        Serial.write("NO_KEYPRESS\n");
      }
    }
  }
  Serial.write("\n[END]\n");
}

void loopbackTest() {
  typewriter.readFlush();

  while (true) {
    Serial.write("\nPress Enter to send query, q to quit...\n");

    while (Serial.available() == 0) {
      delay(100);
    }
    char inByte = Serial.read();
    if (inByte == 'q') {
      while (Serial.available()) {
        Serial.read();
      }
      break;
    }
    else if (inByte == '\n') {
      uint32_t value;
      uint16_t address = 0x121;
      uint16_t command = wheelwriter::QUERY_MODEL;
      Serial.write("Send address: 0x");
      Serial.println(address, HEX);
      uart.write(address);
      delayMicroseconds(150);
      value = uart.read();
      Serial.write("Received: 0x");
      Serial.println(value, HEX);
      delayMicroseconds(150);
      value = uart.read();
      Serial.write("Received: 0x");
      Serial.println(value, HEX);
      
      Serial.write("Send command: 0x");
      Serial.println(command, HEX);
      uart.write(command);
      delayMicroseconds(150);
      value = uart.read();
      Serial.write("Received: 0x");
      Serial.println(value, HEX);
      delayMicroseconds(150);
      value = uart.read();
      Serial.write("Received: 0x");
      Serial.println(value, HEX);
    }
  }
}

void printwheelSample(const uint8_t plusPosition, const uint8_t underscorePosition) {
  // A printwheel has 96 (0x60) characters
  // This prints in a pair 16 x 6 arrays (regular and bold) with a border of alignment marks

  typewriter.readFlush();

  wheelwriter::ww_typestyle typestyle = wheelwriter::TYPESTYLE_NORMAL;
  typewriter.setSpaceForWheel();
  typewriter.setLeftMargin();

  // Row 0
  typewriter.typeCharacter(plusPosition, typestyle);
  typewriter.typeCharacter(plusPosition, typestyle);
  typewriter.typeCharacter(underscorePosition, typestyle);
  typewriter.moveCarriageSpaces(7);
  typewriter.typeCharacter(plusPosition, typestyle);
  typewriter.moveCarriageSpaces(9);
  typewriter.typeCharacter(plusPosition, typestyle);
  typewriter.carriageReturn();
  typewriter.lineFeed();

  // Row 1
  typewriter.typeCharacter(plusPosition, typestyle);
  typewriter.carriageReturn();
  typewriter.lineFeed();

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
      typewriter.moveCarriageSpaces(2);
      for (int i = 0; i < 16; i++, wheelPosition++) {
        typewriter.typeCharacter(wheelPosition, typestyle);
        // Space after the 8th character
        if (i == 7) {
          typewriter.moveCarriageSpaces(1);
        }
      }
      typewriter.carriageReturn();
      typewriter.lineFeed();
    }
    // Space between samples
    if (sample == 0) {
      typewriter.typeCharacter(plusPosition, typestyle);
      typewriter.moveCarriageSpaces(19);
      typewriter.typeCharacter(plusPosition, typestyle);
      typewriter.carriageReturn();
      typewriter.lineFeed();
    }
  }
  typestyle = wheelwriter::TYPESTYLE_NORMAL;
  
  // Row 15
  typewriter.carriageReturn();
  typewriter.lineFeed();

  // Row 16
  typewriter.typeCharacter(plusPosition, typestyle);
  typewriter.moveCarriageSpaces(9);
  typewriter.typeCharacter(plusPosition, typestyle);
  typewriter.moveCarriageSpaces(9);
  typewriter.typeCharacter(plusPosition, typestyle);
  typewriter.carriageReturn();
  typewriter.lineFeed();
}

void queryFunction() {
  typewriter.readFlush();
  uint16_t model = typewriter.queryModel();
  Serial.write("Model: 0x");
  Serial.println(model, HEX);
  uint16_t wheel = typewriter.queryPrintwheel();
  Serial.write("Wheel: 0x");
  Serial.println(wheel, HEX);
  uint16_t status = typewriter.queryStatus();
  Serial.write("Status: 0x");
  Serial.println(status, HEX);
  // uint16_t operation = typewriter.queryOperation();
  // Serial.write("Operation: 0x");
  // Serial.println(operation, HEX);
}

void rawCommandFunction() {
  typewriter.readFlush();
  Serial.write("[BEGIN]\n");

  while (true) {
    if (Serial.available()) {
      String inString = Serial.readStringUntil('\n');
      inString.toLowerCase();
      inString.toCharArray(inputBuffer, 64);

      // End with 'q' or EOT (CTRL-D)
      if ((strlen(inputBuffer) == 1) && ((inputBuffer[0] == 'q') || (inputBuffer[0] == 0x04))) {
        break;
      }

      const char delim[2] = " ";
      char* token = strtok(inputBuffer, delim);
      char* end;
      uint8_t command[3];
      uint8_t numValidTokens = 0;
      uint8_t error = 0;

      while (token != NULL) {
        long value = strtol(token, &end, 0);
        if (value > 0xff) {
          error = 1;
          Serial.write("ERROR - values must be bytes!\n");
          break;
        }
        if (numValidTokens >= 3) {
          break;
        }
        if (end != token) {
          command[numValidTokens] = value;
          numValidTokens++;
        }
        token = strtok(NULL, delim);
      }
      if (!error) {
        if (numValidTokens == 1) {
          typewriter.sendCommand((wheelwriter::ww_command)command[0]);
        }
        if (numValidTokens == 2) {
          typewriter.sendCommand((wheelwriter::ww_command)command[0], command[1]);
        }
        if (numValidTokens == 3) {
          typewriter.sendCommand((wheelwriter::ww_command)command[0], command[1], command[2]);
        }
      }
    }
  }

  Serial.write("[END]\n");
}

void readFunction() {
  typewriter.readFlush();
  Serial.write("[BEGIN]\n");
  unsigned long lastCommandTime = 0;
  while (true) {
    if (Serial.available()) {
      String inString = Serial.readStringUntil('\n');
      inString.toLowerCase();
      inString.toCharArray(inputBuffer, 64);

      // End with 'q' or EOT (CTRL-D)
      if ((strlen(inputBuffer) == 1) && ((inputBuffer[0] == 'q') || (inputBuffer[0] == 0x04))) {
        break;
      }
    }
    // typewriter.readFlush();
    uint8_t blocking = 0;
    uint8_t verbose = 1;
    uint8_t commandLength = typewriter.readCommand(blocking, verbose);

    if (commandLength) {
      lastCommandTime = millis();
    }
    if (lastCommandTime && ((millis() - lastCommandTime) > 1000)) {
      Serial.write('\n');
      lastCommandTime = 0;
    }
  }
  
  Serial.write("\n[END]\n");
}

void relayFunction() {
  typewriter.readFlush();
  Serial.write("[BEGIN]\n");
  unsigned long commandStartTime, commandDuration;
  unsigned long timeout = 1000;
  unsigned char inByte;
  unsigned char response[4];
  while (true) {
    if (Serial.available()) {
      commandStartTime = millis();

      // Read the command byte and hand off to the respective handlers
      inByte = Serial.read();
      if (inByte == 0x04) { // ^D, end mode
        break;
      }
      if (inByte == 0x0a) { // \n, NOOP
        continue;
      }
      if (inByte & 0x10) {
        relayCommand(inByte, commandStartTime, timeout);
        continue;
      }
      if (inByte & 0x80) {
        //configCommand(inByte, commandStartTime, timeout);
        continue;
      }

      // Unknown command byte - read until \n and return error
      response[0] = inByte; // Command byte we are responding to
      response[1] = 0xf0;   // Invalid command response
      response[2] = inByte;
      response[3] = '\n';
      while (true) {
        if (Serial.available()) {
          inByte = Serial.read();
          if (inByte == '\n') {
            break;
          }
        }
        commandDuration = millis() - commandStartTime;
        if (commandDuration > timeout) {
          break;
        }
      }
      Serial.write(response, 4);
    }
  }
  Serial.write("\n[END]\n");
}

void relayCommand(char commandByte, unsigned long commandStartTime, unsigned long timeout) {  
  bool abbreviatedFlag = (commandByte & 0x01);
  bool batchFlag = (commandByte & 0x02);
  bool ignoreErrorsFlag = (commandByte & 0x04);

  unsigned char batchSize = 1;
  
  // In batch mode, read the batch size
  if (batchFlag) {
    while (true) {
      if (Serial.available()) {
        batchSize = Serial.read();
        break;
      }
      if (timeoutCheckAndRespond(commandStartTime, timeout, commandByte)) {
        return;
      }
    }
  }

  // Read and relay the commands
  unsigned char commandBuffer[4];
  unsigned char response[4];
  response[0] = commandByte;
  response[1] = 0x10;
  response[3] = '\n';
  int bytesRead, expectedBytes;
  if (abbreviatedFlag) {
    commandBuffer[0] = typewriter.getDefaultAddress();
  }
  for (unsigned char i = 0; i < batchSize; i++) {
    if (abbreviatedFlag) {
      expectedBytes = 3;
      bytesRead = Serial.readBytes(commandBuffer+1, expectedBytes);
    }
    else {
      expectedBytes = 4;
      bytesRead = Serial.readBytes(commandBuffer, expectedBytes);
    }
    if (bytesRead < expectedBytes) {
      sendTimeoutResponse(commandByte);
      return;
    }
    uint8_t error;
    // Serial.write("Sending command\n");
    // Serial.write(commandBuffer, 4);
    // Serial.write('\n');
    uint8_t wwCmdResponse = typewriter.sendCommand(commandBuffer[0], commandBuffer[1], commandBuffer[2], commandBuffer[3], &error, (int)ignoreErrorsFlag);
    if (!ignoreErrorsFlag && error) {
        if (batchFlag) {
          response[1] = 0x12; // Batch error
          response[2] = i;    // Failed command index
        }
        else {
          response[1] = error;
          response[2] = wwCmdResponse;
        }
        Serial.write(response, 4);
        return;
    }
    else {
      response[2] = wwCmdResponse;
    }
  }

  // Read terminator
  // Serial.write("Reading terminator\n");
  unsigned char inByte;
  bytesRead = Serial.readBytes(&inByte, 1);
  // Serial.write("bytesRead: ");
  // Serial.write(bytesRead);
  // Serial.write('\n');
  if (bytesRead) {
    // Serial.write("inByte: ");
    // Serial.write(inByte);
    // Serial.write('\n');
    if (inByte != '\n') {
      response[1] = 0x15; // Command length error
      response[2] = expectedBytes * batchSize;  // Expected length
    }
    Serial.write(response, 4);
  }
  else {
    sendTimeoutResponse(commandByte);
  }
  return;
}

int timeoutCheckAndRespond(unsigned long commandStartTime, unsigned long timeout, unsigned char commandByte) {
  unsigned long commandDuration = millis() - commandStartTime;
  if (commandDuration > timeout) {
    sendTimeoutResponse(commandByte);
    return 1;
  }
  else {
    return 0;
  }
}

void sendTimeoutResponse(unsigned char commandByte) {
  unsigned char response[4];
  response[0] = commandByte;  // Command byte we are responding to
  response[1] = 0xf2;         // Command transmission timeout
  response[2] = commandByte;
  response[3] = '\n';
  Serial.write(response, 4);
}

void typeFunction(uint8_t keyboard, uint8_t useCaratAsControl) {
  uint8_t USE_CARAT_AS_CONTROL = useCaratAsControl;
  wheelwriter::ww_linespacing lineSpacing;
  uint8_t caratFlag = 0;
  uint8_t bytesAvailable = 0;
  uint8_t paused = false;
  wheelwriter::ww_typestyle typestyle = wheelwriter::TYPESTYLE_NORMAL;

  typewriter.readFlush();
  typewriter.setSpaceForWheel();
  typewriter.setKeyboard(keyboard);
  typewriter.setLeftMargin();

  Serial.write("[BEGIN]\n");

  while (true) {
    bytesAvailable = Serial.available();

    // Flow control
    if (bytesAvailable > 48) {
      Serial.write(0x13); // XOFF
      paused = true;
    }
    else if (paused && (bytesAvailable < 16)) {
      Serial.write(0x11); // XON
      paused = false;
    }

    if (bytesAvailable) {
      char inByte = Serial.read();
      // End when receiving EOT (CTRL-D) or the string "^D"
      if ((inByte == 0x04) || (caratFlag && ((inByte == 'd') || (inByte == 'D')))) {
        while (Serial.available()) {
          Serial.read();
        }
        break;
      }
      // ANSI escape code - CSI (Control Sequence Introducer) with SGR (Select Graphic Rendition) parameter
      // ^[ [ <value> m
      else if ((inByte == 0x1b) || (caratFlag && (inByte == '['))) {
        String inString = Serial.readStringUntil('m');
        parseEscape(inString.c_str(), typestyle, lineSpacing);
        typewriter.setLineSpacing(lineSpacing);
      }
      // New line
      else if (inByte == 0x0a) {
        typewriter.carriageReturn();
        typewriter.lineFeed();
      }
      // Carat may be used as a special symbol for control characters (^D, ^[)
      else if (USE_CARAT_AS_CONTROL && (inByte == '^')) {
        caratFlag = 1;
      }
      // Print the character
      else {
        // Serial.print(inByte);
        typewriter.typeAscii(inByte, typestyle);
      }
      
      if (caratFlag && (inByte != '^')) {
        caratFlag = 0;
      }
    }
  }

  Serial.write("\n[END]\n");
}

void parseEscape(const char* buffer, wheelwriter::ww_typestyle& typestyle, wheelwriter::ww_linespacing& lineSpacing) {
  if (buffer[0] == '[') {
    char* end;
    long value = strtol(buffer+1, &end, 10);
    if (end != buffer+1) {
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
      }
    }
  }
}