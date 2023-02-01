# Wheelwriter bus
The Wheelwriter bus is a shared +5V serial line operating at 187,500 bps 
(12 MHz/64), with 9 data bits and 1 stop bit. This is likely the Intel 8051
serial port operating in mode 2. See the description on page 3-14 of the 
[Intel 8051 datasheet](http://datasheets.chipdb.org/Intel/MCS51/MANUALS/27238302.PDF). 
The 9th bit is used as to indicate the transmitted byte is an address if the 
bit if 1 and data if the bit is 0.

## Implementation
Many embedded devices don't actually implement a 9-bit serial port, but instead 
use the parity bit to implement this. Many have "sticky" or "stick" parity 
modes which override a transmitted parity bit. Sometimes this is referred to as 
"mark" parity to send/expect a 1 or "space" parity to send/expect a 0. Embedded 
devices often will interrupt when the parity bit indicates when an address byte 
is sent.

However, while the RP2040 built-in UART supports this mode, the Arduino SDK 
does not appear to support it. Using these modes causes the board to stop 
functioning. Instead, a 9-bit PIO-based UART was implemented. See 
[src/arduino/wheelwriter_interface/README.md](src/arduino/wheelwriter_interface/README.md)
for more information.

## Protocol
[Bus protocol](https://github.com/RussellSenior/IBM-Wheelwriter-Hack/wiki/Bus-Protocol) 
as reverse engineered by [RussellSenior](https://github.com/RussellSenior).

The motor controller appears to be address `0x21`.
