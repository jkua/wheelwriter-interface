// Wheelwriter interface for an Arduino Nano RP2040 Connect
// Copyright (c) 2023 John Kua <john@kua.fm>
//
#include "Uart9bit.h"
#include "Wheelwriter.h"

Uart9Bit uart;
wheelwriter::Wheelwriter typewriter;
int inByte = 0;

void setup() {
  uint sm = 0;
  uint pin = 25; // D2/GPIO25
  uart.init(pio0, sm, pin, 185700);
  typewriter.init(&uart);

  Serial.begin(115200);
  Serial.write("Press Enter to continue\n");
}

void loop() {
  if (Serial.available() > 0) {
    inByte = Serial.read();
    if (inByte = '\n') {
      Serial.write("Sending...\n");
      // uint16_t buffer[3];
      // buffer[0] = 0x121;
      // buffer[1] = 0x020;
      // buffer[1] = 0x000;
      // uart.write(buffer, 3);
      
      // typewriter.typeCharacter(0x01);
      // delay(500);
      // typewriter.moveCarriage(10, wheelwriter::WW_CARRIAGE_DIRECTION_RIGHT);
      // delay(500);
      // typewriter.typeCharacter(0x59);
      // delay(500);
      // typewriter.moveCarriage(10, wheelwriter::WW_CARRIAGE_DIRECTION_RIGHT);
      // delay(500);
      // typewriter.typeCharacter(0x05);
      // delay(500);
      // typewriter.moveCarriage(10, wheelwriter::WW_CARRIAGE_DIRECTION_RIGHT);

      typewriter.typeCharacter(0x01, 10);
      delay(500);
      typewriter.typeCharacter(0x59, 10);
      delay(500);
      typewriter.typeCharacter(0x05, 10);
      delay(500);
    }
  }
}
