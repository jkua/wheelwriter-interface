// Wheelwriter interface for an Arduino Nano RP2040 Connect
// Copyright (c) 2023 John Kua <john@kua.fm>
//
#include "Uart9bit.h"
#include "Wheelwriter.h"

Uart9Bit uart;
wheelwriter::Wheelwriter typewriter;
int inByte = 0;

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
  Serial.write("Press Enter to continue\n");
}

void loop() {
  // char hexString[7] = "0x0000";
  // uint16_t inWord = uart.read();
  // sprintf(hexString, "0x%02X", inWord);
  // Serial.println(hexString);

  if (Serial.available() > 0) {
    inByte = Serial.read();
    if (inByte = '\n') {
      Serial.write("Sending...\n");
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

      // circleTest();
      characterTest(wheelwriter::TYPESTYLE_NORMAL);
    }
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
  for (int i = 0; i < strlen(buffer); i++) {
    typewriter.typeAscii(buffer[i]);
    if (i == strlen(buffer) - 1) break;
    typewriter.moveCarriage(dx[i]);
    typewriter.movePlaten(-dy[i]);
  }
  typewriter.movePlaten(127);
  typewriter.movePlaten(127);
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
    typewriter.movePlaten(lineSpace);
    typewriter.moveCarriage(strlen(buffers[i])*charSpace, wheelwriter::CARRIAGE_DIRECTION_LEFT);
  }
}