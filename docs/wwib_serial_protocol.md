# Wheelwriter Interface Board Serial Communication Protocol

This describes the serial communication protocol between a client and the 
Wheelwriter Interface Board (WWIB).

This is implemented in [src/arduino/wheelwriter_interface/wheelwriter_interface.ino](../src/arduino/wheelwriter_interface/wheelwriter_interface.ino).

## Serial port parameters
The WWIB communicates using serial over USB at a baud rate of 115200 with 
software flow control (XON/XOFF).

## Character encoding and line endings
This protocol uses ASCII with line feeds (`\n`) to end lines.

## Main loop
On power up, the WWIB starts the main loop and outputs:

```
### Wheelwriter Interface ###
[READY]
```

The `[READY]` prompt indicates that the WWIB is ready to accept one of the 
following primary commands:

1. `help` - print the help text, listings these commands
2. `buffer` - execute the buffer test
3. `char` - execute the character test
4. `circle` - execute the circle test
5. `keyboard` - read input from the keyboard
6. `loopback` - execute the loopback test
7. `query` - query typewriter for information
8. `raw` - raw command mode
9. `read` - read bus commands
10. `sample` - print a type sample
11. `type` - type characters on the typewriter

Some commands accept parameters - these are separated by spaces. The full 
command string, including parameters, are sent terminated with a line feed (`\n`).

e.g. `buffer 10 80\n`

### Buffer test
* *Command:* `buffer <numChars> <charsPerLine>`
* *Arguments:* 
    1. `numChars` - number of characters to send (default 10)
    2. `charsPerLine` - number of characters per line (default 80)

This tests the interface boards ability to send character to the Wheelwriter. It 
repeatedly sends the sequence `123456789.` until it reaches the specified number 
of characters to send (`numChars`). `charsPerLine` specifies the number of 
characters per line.

This returns to the main loop after completion.

### Character test
* *Command:* `char <typestyle>`
* *Arguments:* 
    1. `typestyle` - typestyle to use [`bold`, `underline`]. (default `normal`)

This exercises all the characters on the typewheel in US keyboard order. The 
typestyle may be specified to test the bold and underline modes.

This returns to the main loop after completion.

### Circle test
* *Command:* `circle`
* *Arguments:* None

This tests the ability of the WWIB to use discrete carriage and platen movement 
commands to type the string `Hello, world! Lorem ipsum dolor sit amet.` in a 
circle.

This returns to the main loop after completion.

### Keyboard mode
* *Command:* `keyboard`
* *Arguments:*
    1. `verbose` - supports two levels of verbosity (`v`, `vv`) beyond the default

In this mode, the Wheelwriter keyboard is read and the keypresses are output. 
This mode assumes that the Wheelwriter is using a US keyboard layout.

This mode does not suppress the actual typing that the Wheelwriter does.

The WWIB starts by sending `[BEGIN]` and then listening for keyboard input. 
When a keypress is detected, the ASCII value is output. 

In `v` verbosity mode, each keypress is sent as a hexadecimal value on a 
separate line. e.g. `Keypress: 0x61`

In `vv` verbosity mode, in addition to the above, the line `NO_KEYPRESS` is 
output if no keypress is detected.

Sending `q` or EOT/`^D`/`0x24` will end the capture and return to the main 
loop.

### Loopback test
* *Command:* `loopback`
* *Arguments:* None

This performs a simple low level test of Wheelwriter bus communication by 
continuously sending a query command one byte at a time and listening for 
ACKs/responses after each byte.

Sending `q` or EOT/`^D`/`0x24` will end the test and return to the main 
loop.

### Query test
* *Command:* `query`
* *Arguments:* None

This uses the high level API to query the model, printwheel pitch, and status of 
the Wheelwriter.

This returns to the main loop after completion.

### Raw mode
* *Command:* `raw`
* *Arguments:* None

In this mode, raw commands may be relayed to the Wheelwriter. 

The WWIB starts by outputting `[BEGIN]`.

Each command is accepted as 1-3 bytes, represented as space separated numeric 
strings, terminated with a line feed (`\n`). The prefix `0x` may be used to 
specify hexadecimal values, otherwise they are assumed to be decimal values.

e.g. `0x121 0x00\n` sends the query command (`0x00`) to address (`0x121`)

Sending `q` or EOT/`^D`/`0x24` will end this mode and return to the main 
loop.

### Read mode (bus sniffer)
* *Command:* `read`
* *Arguments:* None

In this mode, the WWIB listens for communications on the Wheelwriter bus.

The WWIB starts by outputting `[BEGIN]`. Then it outputs each command sequence 
that it hears on the bus.

Sending `q` or EOT/`^D`/`0x24` will end this mode and return to the main 
loop.

### Sample mode
* *Command:* `sample <plusPosition> <underscorePosition>`
* *Arguments:*
    1. `plusPosition` - the position of the `+` character on the printwheel
    2. `underscorePosition` - the position of the `_` character on the printwheel

This types a sample of text which exercises all 96 characters of the printwheel, 
in a regular pattern (16 x 6) along with alignment marks intended for scanning 
and automatic image processing. 

This returns to the main loop after completion.


### Type mode
* *Command:* `type <keyboard> <useCaratAsControl>`
* *Arguments:*
    1. `keyboard` - set the keyboard (default `1` for US)
    2. `useCaratAsControl` - uses the `^` symbol as a control character for
        escaping commands.

In this mode, ASCII strings sent to the WWIB are typed on the Wheelwriter. 
[ANSI escape](https://en.wikipedia.org/wiki/ANSI_escape_code) style codes are 
supported to set various typing modes. The escape character is sent as `0x1b` or 
with the `^[` if `useCaratAsControl` is set. The latter is useful when 
composing documents in a text editor.

The escape codes follow the CSI (Control Sequence Introducer) `[` with the 
[SGR (Select Graphic Rendition) parameter](https://en.wikipedia.org/wiki/ANSI_escape_code#SGR_(Select_Graphic_Rendition)_parameters) format:

`<ESC><CSI><n>m`

*Supported escape codes*
* Normal: `^[[0m`
* Bold: `^[[1m`
* Underline: `^[[4m`
* Single space: `^[[10m`
* 1.5 space: `^[[11m`
* Double space: `^[[12m`
* Triple space: `^[[13m`
* Not bold: `^[[22m`
* Not underline: `^[[24m`

