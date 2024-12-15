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

The basic protocol for communicating with the motor controller is to send 

`<address> <command> <optional data 1> <optional data 2>`

where each element is one byte. Each byte is acknowledged by the motor 
controller, such that this looks like (`<client> (controller)`):

`<address> (ACK) <command> (ACK/response) <optional data 1> (ACK) <optional data 2> (ACK)`

where `ACK` is simply `0x00`. For query commands, the controller will send a 
`response` byte rather than an `ACK`. 

As this is a half-duplex bus, the client must allow for the `ACK`s by either 
monitoring the bus for these responses or inserting a long delay (~450 ms to be 
safe). Failure to do so will result in garbled communication as the controller 
responses will step on the client transmissions. Some implementations simply 
wait for the bus to go low and then high again to detect the ACKs. Coupled with 
an additional delay in case the response is non-zero, this should be a reliable 
method.

However, this interface reads the motor controller replies in order to properly 
understand the query responses. Looking at the logic board output, it prepends 
any type or movement command with a status query. Likely this is to ensure that 
the carriage and platen have stopped moving before issuing the next type or 
movement command.

### Commands
| Command | Data 1           | Data 2        | Response              | Description                     |
|---------|------------------|---------------|-----------------------|---------------------------------|
| `0x00`  | -                | -             | `0x06`: Wheelwriter 3 | Query model, returns model byte |
|         |                  |               | `0x25`: Wheelwriter 5 |                                 |
|         |                  |               | `0x26`: Wheelwriter 6 |                                 |
| `0x01`  | -                | -             | Wheel pitch (see command `0x08`) | Power-on reset, moves to left stop, returns wheel pitch |
| `0x02`  | `wheel_position` | `IGNORED`     | `0x00`                | Type character without advancing    |
| `0x03`  | `wheel_position` | `uspaces`     | `0x00`                | Type character and advance carriage to the right |
| `0x04`  | `wheel_position` | `uspaces`     | `0x00`                | Type on erase ribbon and advance carriage to the right |
| `0x05`  | bit 7: `dir` (0: paper down, 1 up) | -             | `0x00`                | Rotate platen   |
|         | bits 0-6: `usteps`                 | -             |                       |                 |
| `0x06`  | bit 7: `dir` (0: left, 1 right)    | `usteps_low`  | `0x00`                | Move carriage   |
|         | bits 0-6: `usteps_high`            |               |                       |                 |
| `0x07`  | -                | -             | `0x00`                | Spin wheel                      |
| `0x08`  | -                | -             | `0x08`: Proportional  | Query printwheel pitch          |
|         |                  |               | `0x10`: 15 cpi        |                                 |
|         |                  |               | `0x20`: 12 cpi        |                                 |
|         |                  |               | `0x21`: No wheel      |                                 |
|         |                  |               | `0x40`: 10 cpi        |                                 |
| `0x09`  | `impression_ctrl`| -             | `0x00`                | Sets the hammer strike power. On a Wheelwriter 3 this is `0x00` or `0x01`. Later models (WW10) appear to have three levels. Also set with Code+Q |
| `0x0a`  | `0x00`           | -             | `0x00`                | Unknown - emitted during power up after power-on reset command |
| `0x0b`  | -    	         | -             | `0x00`: Ready         | Query status - sent prior to all typing and motion commands |
|         |                  |               | `0x04`: Carriage moving? |                                                            |
|         |                  |               | `0x07`: Carriage move complete? |                                                     |
|         |                  |               | `0x10`: Platen moving?|                                                               |
|         |                  |               | `0x14`:	             |                                                               |
|         |                  |               | `0x40`: Printwheel changed or left limit switch pressed |                             |
| `0x0c`  | `keypressed`     | `0x46`: Start keypress event  | `0x04`, `0x10` | Sent prior to typing/motion commands |
|         |                  | `0x08`: Backspace and some space events | `0x010`, `0xf0` at left margin | Sent after typing/motion command and before `0x44` |
|         |                  | `0x44`: Wait for motion?      | `0x40`, `0x80` | Sent after typing/motion command - can take a long time to get response |
|         |                  | `0x06`: End keypress event    | `0x04`, `0x10`, `0x39` | Last query of typing/motion command set - fast response |
|         |                  | `0x01`: Wait for platen/carriage move | UNKNOWN | Sent after platen/tab commands |
| `0x0d`  | 

### Typing command sequence
 1. Query status - `0x0b` / `0x00`
 2. Query - `0x0c` `keypressed` `0x46` / variable
 3. Type and advance - `0x03` `wheel_position` `uspaces` / `0x00`
 4. (rare) Query - `0x0c` `keypressed` `0x08` / variable
 5. Query - `0x0c` `keypressed` `0x44` / variable
 6. Query - `0x0c` `keypressed` `0x06` / variable
 ### Return command sequence
 1. Query status - `0x0b` / `0x00`
 2. Move platen - `0x05`
 3. Move carriage - `0x06`
 4. Query - `0x0c` `0x56` (return key) `0x01` / `0x20`
 ### Tab command sequence
 Note that pressing the space bar uses the typing command sequence above
 1. Query status - `0x0b` / `0x00`
 2. Move carriage - `0x06`
 3. Query - `0x0c` `0x4a` (tab) `0x01` / `0x20`
 ### Platen command sequence (paper up/down)
 1. Query status - `0x0b` / `0x00`
 2. Move platen - `0x05`
 3. Query - `0x0c` `keypressed` (return key) `0x01` / `0x20`
 ### Power on sequence
 1. Power-on reset - `0x01` / `0x20`
 2. Unknown - `0x0a` `0x00` / `0x00`
 3. Query - `0x0c` `0x00` `0x01` / `0x20`
 4. Move carriage right 120 uspaces (1") - `0x06` `0x80` `0x78` / `0x00` 
