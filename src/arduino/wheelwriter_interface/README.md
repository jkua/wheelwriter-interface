# Arduino Nano RP2040 Connect Wheelwriter Interface
This programs an Arduino Nano RP2040 to communicate with an IBM Wheelwriter 3/5/6.

## Serial UART
The IBM Wheelwriter serial bus is the Intel 8051 serial port in mode 2. This 
operates at 187500 baud (12 MHz / 64), 9 data bits, no parity, 1 stop bit. 
The 9th bit is used to indicate that this is an address (1) or data (0).

While the RP2040 built-in UART and the underlying mbed OS supports Mark/Space 
(Forced0/Forced1) parity, the Arduino SDK does not seem to support it. 
Modifying the board package Serial.cpp to set the Forced0/Forced1 parity mode 
causes the board to stop functioning.

To get around this, a PIO-based 9-bit UART was implemented, based on the 
Raspberry Pi Pico PIO UART example.