# Wheelwriter Interface Board Command Relay Protocol
This binary protocol is used to have the interface board relay raw commands to 
the typewriter and the responses back. This mode also supports sending batches 
of commands.

## Relay commands

### Single command - full (0x01, 7 bytes)
Format: `0x01 <address (2 bytes)> <command (1 byte)> <data1 (1 byte)> <data2 (1 byte)> \n`
Example: 0x01 0x0121 0x03 0x01 0x0a 0x0a

### Batched commands (halt on error) - full (0x02, 5n+4 bytes)
Format: `0x02 <# commands (2 bytes)> <full command 1 (5 bytes)> <full command 2 (5 bytes)> ... <full command n (5 bytes)>\n`

In `halt on error` mode, the interface board will stop sending commands if any 
command fails and return `batch_failed` with the index of the failed command.

### Batched commands (ignore errors) - full (0x03, 5n+4 bytes)
Format: `0x02 <# commands (2 bytes)> <full command 1 (5 bytes)> <full command 2 (5 bytes)> ... <full command n (5 bytes)>\n`

In `ignore errors` mode, the interface board will continue sending commands 
even if any of them fail.

### Single command - motor (0x11, 5 bytes)
Format: `0x11 <command (1 byte)> <data1 (1 byte)> <data2 (1 byte)> \n`

### Batched commands (halt on error) - motor (0x12, 3n+4 bytes)
Format: `0x12 <# commands (2 bytes)> <motor command 1 (3 bytes)> <motor command 2 (3 bytes)> ... <motor command n (3 bytes)>\n`

In `halt on error` mode, the interface board will stop sending commands if any 
command fails and return `batch_failed` with the index of the failed command.

### Batched commands (ignore errors) - motor (0x13, 5n+4 bytes)
Format: `0x13 <# commands (2 bytes)> <motor command 1 (3 bytes)> <motor command 2 (3 bytes)> ... <motor command n (3 bytes)>\n`

In `ignore errors` mode, the interface board will continue sending commands 
even if any of them fail.

### NOOP (0x0a, 1 byte)
Format: `\n`

Nothing happens.

### End relay mode (0x04, 1 byte)
Format: `\d`

Ends relay mode and returns

## Response (3 bytes)
Format: `<response_status> <typewriter_reply/error_data> \n`

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
