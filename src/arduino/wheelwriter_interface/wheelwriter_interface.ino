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
  digitalWrite(6, HIGH);

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
      Serial.write("char - execute the character test\n");
      Serial.write("circle - execute the circle test\n");
      Serial.write("type - type characters on the typewriter\n");
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
    else if (strcmp(token, "type") == 0) {
      Serial.write("[FUNCTION] Type\n");
      typeFunction();
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
  // Move to center
  typewriter.moveCarriage(100);
  for (int i = 0; i < strlen(buffer); i++) {
    typewriter.typeAscii(buffer[i]);
    if (i == strlen(buffer) - 1) break;
    typewriter.moveCarriage(dx[i]);
    typewriter.movePlaten(-dy[i]);
  }
  typewriter.movePlaten(127);
  typewriter.movePlaten(127);
  typewriter.moveCarriage(100, wheelwriter::CARRIAGE_DIRECTION_LEFT);
}

void characterTest(wheelwriter::ww_typestyle style) {
  char buffer1[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  char buffer2[] = "abcdefghijklmnopqrstuvwxyz";
  char buffer3[] = "1234567890-=!@#$%\xa2&*()_+";
  char buffer4[] = "\xbc\xbd[]:;\"',.?/\xb0\xb1\xb2\xb3\xa7\xb6";
  char* buffers[4] = {buffer1, buffer2, buffer3, buffer4};
  uint8_t charSpace = 10;
  uint8_t lineSpace = 16;

  for (int i = 0; i < 4; i++) {
    typewriter.typeAsciiString(buffers[i], charSpace, style);
    typewriter.moveCarriage(strlen(buffers[i])*charSpace, wheelwriter::CARRIAGE_DIRECTION_LEFT);
    typewriter.movePlaten(lineSpace);
  }
}

void typeFunction() {
  Serial.write("[BEGIN]\n");
  uint8_t charSpace = 10;
  uint8_t lineSpace = 16;
  uint8_t numCharacters = 0;
  uint8_t caratFlag = 0;
  while (true) {
    if (Serial.available()) {
      char inByte = Serial.read();
      // End when receiving EOT (CTRL-D)
      if ((inByte == 0x04) || (caratFlag && ((inByte == 'd') || (inByte == 'D')))) {
        while (Serial.available()) {
          Serial.read();
        }
        break;
      }
      else if ((inByte == 0x0a) && numCharacters) {
        typewriter.moveCarriage(numCharacters * charSpace, wheelwriter::CARRIAGE_DIRECTION_LEFT);
        typewriter.movePlaten(lineSpace);
        numCharacters = 0;
      }
      else if (inByte == '^') {
        caratFlag = 1;
      }
      else {
        Serial.println(inByte, HEX);
        typewriter.typeAscii(inByte, charSpace);
        numCharacters++;
      }
      
      if (caratFlag && (inByte != '^')) {
        caratFlag = 0;
      }
    }
  }
  Serial.write("[END]\n");
}