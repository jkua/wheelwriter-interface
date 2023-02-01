// Wheelwriter interface for an Arduino Nano RP2040 Connect
// Copyright (c) 2023 John Kua <john@kua.fm>
//
#include "Uart9bit.h"
#include "Wheelwriter.h"

Uart9Bit uart;
wheelwriter::Wheelwriter typewriter;

void setup() {
  uint sm = 0;
  uint pin = 25; // D2/GPIO25
  uart.init(pio0, sm, pin, 185700);
  typewriter.init(&uart);
}

void loop() {
  // uint16_t buffer[3];
  // buffer[0] = 0x121;
  // buffer[1] = 0x020;
  // buffer[1] = 0x000;
  // uart.write(buffer, 3);
  typewriter.typeCharacter(0x01);
  delay(500);
}
