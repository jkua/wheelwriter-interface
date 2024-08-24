# Wheelwriter Interface Board Command Relay Protocol
This binary protocol is used to have the interface board relay raw commands to 
the typewriter and the responses back. This mode also supports sending batches 
of commands.

## Relay commands

### Full command mode
In this mode, the complete command is sent, including the address.

#### Single command - full (0x01, 6 bytes)
* Format: `0x01 <address (1 byte)> <command (1 byte)> <data1 (1 byte)> <data2 (1 byte)> \n`
* Example: `0x01 0x21 0x03 0x01 0x0a 0x0a`

#### Batched commands (halt on error) - full (0x02, 4n+4 bytes)
* Format: `0x02 <# commands (2 bytes)> <full command 1 (4 bytes)> <full command 2 (4 bytes)> ... <full command n (4 bytes)>\n`

In `halt on error` mode, the interface board will stop sending commands if any 
command fails and return `batch_failed` with the index of the failed command.

#### Batched commands (ignore errors) - full (0x03, 4n+4 bytes)
* Format: `0x02 <# commands (2 bytes)> <full command 1 (4 bytes)> <full command 2 (4 bytes)> ... <full command n (4 bytes)>\n`

In `ignore errors` mode, the interface board will continue sending commands 
even if any of them fail.

### Abbreviated command mode
In this mode, the address is fixed. By default, this is the address for the 
motor control board, `0x21`. This can be changed at runtime with the **set 
destination address** configuration command.

#### Single command - abbreviated (0x11, 5 bytes)
* Format: `0x11 <command (1 byte)> <data1 (1 byte)> <data2 (1 byte)> \n`
* Example: `0x01 0x03 0x01 0x0a 0x0a`

#### Batched commands (halt on error) - abbreviated (0x12, 3n+4 bytes)
* Format: `0x12 <# commands (2 bytes)> <abbreviated command 1 (3 bytes)> <abbreviated command 2 (3 bytes)> ... <abbreviated command n (3 bytes)>\n`

In `halt on error` mode, the interface board will stop sending commands if any 
command fails and return `batch_failed` with the index of the failed command.

#### Batched commands (ignore errors) - abbreviated (0x13, 5n+4 bytes)
* Format: `0x13 <# commands (2 bytes)> <abbreviated command 1 (3 bytes)> <abbreviated command 2 (3 bytes)> ... <abbreviated command n (3 bytes)>\n`

In `ignore errors` mode, the interface board will continue sending commands 
even if any of them fail.

### Relay configuration
Configure relay parameters.

#### Get destination address (0xe1, 2 bytes)
* Format: `0xe1 \n`

This returns the current address. The `response_status` will be **configuration 
success** (0xf0) and `typewriter_reply` will be the address.

#### Set destination address (0xf1, 3 bytes)
* Format: `0xf1 <address (1 byte)> \n`
* Example (set address to `0xab`): `0xf1 0xab 0x0a`

This changes the destination address until the interface board is power-cycled, 
at which time it returns to the default. The `response_status` will be 
**configuration success** (0xf0) and `typewriter_reply` will be the new 
address.

### NOOP (0x0a, 1 byte)
* Format: `\n`

Nothing happens.

### End relay mode (0x04, 1 byte)
* Format: `\d`

Ends relay mode and returns

## Response (3 bytes)
* Format: `<response_status> <typewriter_reply/error_data> \n`

### Response statuses
* 0x00 - Success
	* `typewriter_reply` contains the typewriter's response
* 0x01 - NACK/timeout - typewriter command was not acknowleged before timeout 
	* `error_data` is the index of the command byte which was not acknowledged (zero-indexed)
* 0x02 - Batch failed
	* `error_data` is the index of the command which failed (zero-indexed)
* 0x03 - Invalid typewriter command - the command to be relayed is invalid
	* `error_data` is the index of the command which failed (zero-indexed)
* 0x04 - Invalid relay command byte - the relay command byte is invalid
* 0x05 - Invalid relay command length - the length of the relay command is invalid
	* `error_data` contains the expected length
* 0x06 - Relay command timeout - the full relay command, including the 
         terminating `\n`, was not received before the timeout
    * `error_data` contains the relay command byte
* 0xf0 - Configuration success
* 0xff - Panic - things have gone off the rails
