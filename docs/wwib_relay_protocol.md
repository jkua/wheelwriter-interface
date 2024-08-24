# Wheelwriter Interface Board Command Relay Protocol
This binary protocol is used to have the interface board relay raw commands to 
the typewriter and the responses back. This mode also supports sending batches 
of commands.


## Commands (client to interface board)
All commands, except for NOOP (0x0a) and end relay mode (0x04) will return a response.


### Special commands (0x0N)

#### NOOP (0x0a, 1 byte)
* Format: `\n`

Nothing happens. Typically, this is a command terminator, but if no command has 
been started, the interface board will ignore additional/duplicate values.

#### End relay mode (0x04, 1 byte)
* Format: `\d`

Ends relay mode and returns.


### Relay command (0x1N)
The structure of a relay command is the following sequence of bytes:
* `<relay_flags> <ww_dest_addr> <ww_command> <ww_data_1> <ww_data_2>\n`
	* `<relay_flags>` identifies that this is a relay command and sets certain 
		operating modes. See below for more information.
	* `<ww_dest_addr`> is the Wheelwriter address to relay the command to. 
		Typically this is 0x21.
	* `<ww_command>` is the Wheelwriter command
	* `<ww_data_1>` is the first data byte for the Wheelwriter command. If the 
		command does not accept an argument, this is ignored.
	* `<ww_data_2>` is the second data byte for the Wheelwriter command. If the 
		command does not accept an argument, this is ignored.

Structure of the `relay_flags` byte (8 bits): 
* `<relay_command (0 0 0 1) | <reserved (0)> <ignore_errors_flag> <batch_flag> <abbreviated_command_flag>`
	* `abbreviated_command_flag`, when set, indicates that destination address 
	    should be the configured destination. See the **set destination address** 
	    configuration command for more information.
	* `batch_flag`, when set, indicates that a batch of commands will be sent in 
		a single transaction.
		* The batch format is: `<command_byte> <# commands (1 byte)> <command_0> <command_1> ... <command_2>\n`
		* If `abbreviated_command_flag` is set, the commands will not include the destination address.
	* `ignore_errors_flag`, when set, tells the interface board to ignore any errors and send blindly.
	* `reserved` must be 0

The following are usage examples:

#### Single command - full, halt on error (0x10, 6 bytes)
* Format: `0x10 <address (1 byte)> <command (1 byte)> <data1 (1 byte)> <data2 (1 byte)> \n`
* Example: `0x10 0x21 0x03 0x01 0x0a 0x0a`

This sends the command (0x03) to type a character (0x01, first position on the 
typewheel), and then advance the carriage (0x0a) spaces.

If any byte of the command is not acknowledged by the typewriter before the 
timeout, the error code will be set in the response.

#### Batched commands - full, halt on error (0x12, 4n+4 bytes)
* Format: `0x12 <# commands (2 bytes)> <full command 1 (4 bytes)> <full command 2 (4 bytes)> ... <full command n (4 bytes)>\n`

If any byte of any command is not acknowleged by the typewriter before the 
timeout, the interface board will stop sending commands and return 
`batch_failed` with the index of the failed command.

#### Single command - full, ignore errors (0x14, 6 bytes)
* Format: `0x14 <address (1 byte)> <command (1 byte)> <data1 (1 byte)> <data2 (1 byte)> \n`
* Example: `0x14 0x21 0x03 0x01 0x0a 0x0a`

With the `ignore_errors_flag` set, the interface board will continue sending 
command bytes, even if not acknowledged. Configure the timeout value to control 
how long the interface board should wait for a response before continuing.

#### Batched commands - full, ignore errors (0x16, 4n+4 bytes)
* Format: `0x16 <# commands (2 bytes)> <full command 1 (4 bytes)> <full command 2 (4 bytes)> ... <full command n (4 bytes)>\n`

With the `ignore_errors_flag` set, the interface board will continue sending 
command bytes, even if not acknowledged. Configure the timeout value to control 
how long the interface board should wait for a response before continuing.

#### Single command - abbreviated, halt on error (0x11, 5 bytes)
* Format: `0x11 <command (1 byte)> <data1 (1 byte)> <data2 (1 byte)> \n`
* Example: `0x01 0x03 0x01 0x0a 0x0a`

In this mode, the Wheelwriter destination address is not sent and the default 
value (motor control board, 0x21) is used. This can be changed at runtime with 
the **set destination address** configuration command. 

#### Batched commands - abbreviated, halt on error (0x13, 3n+4 bytes)
* Format: `0x13 <# commands (2 bytes)> <abbreviated command 1 (3 bytes)> <abbreviated command 2 (3 bytes)> ... <abbreviated command n (3 bytes)>\n`

In this mode, the Wheelwriter destination address is not sent and the default 
value (motor control board, 0x21) is used. This can be changed at runtime with 
the **set destination address** configuration command. 


### Configuration commands (0xeN, 0xfN)
Used to configure relay parameters.

The structure of a query/configuration command is the following sequence of bytes:
* Parameter query (2 bytes): `<query_command (0xeN)> \n`
* Parameter config (3 bytes): `<config_command (0xfN)> <value> \n`

Structure of the `query_command` / `config_command` byte (8 bits): 
* `<config_command_flag (1)> <reserved (1)> <reserved (1)> <get/set> | <parameter (4 bits)>`
	* `<parameter>` specifies the parameter to be configured or queried
		* 0x00: Wheelwriter destination address (default: 0x21)
		* 0x01: Command timeout in milliseconds (default: 100 (0x64))

#### Get destination address (0xe0, 2 bytes)
* Format: `0xe0 \n`

This returns the current address. The `response_status` will be **parameter 
query success** (0xe0) and `parameter_value` will be the address.

#### Set destination address (0xf0, 3 bytes)
* Format: `0xf0 <address (1 byte)> \n`
* Example (set address to `0xab`): `0xf0 0xab 0x0a`

This changes the destination address until the interface board is power-cycled, 
at which time it returns to the default. The `response_status` will be 
**parameter config success** (0xf0) and `parameter_value` will be the new 
address.

#### Get command timeout (0xe1, 2 bytes)
* Format: `0xe1 \n`

This returns the current timeout in milliseconds. The `response_status` will 
be **parameter query success** (0xe0) and `parameter_value` will be the 
timeout value.

#### Set command timeout (0xf1, 3 bytes)
* Format: `0xf1 <address (1 byte)> \n`
* Example (set timeout to 200 ms): `0xf1 0xc8 0x0a`

This changes the destination address until the interface board is power-cycled, 
at which time it returns to the default. The `response_status` will be 
**parameter config success** (0xf0) and `parameter_value` will be the new 
timeout.


## Response (interface board to client) (3 bytes)
* Format: `<response_status> <typewriter_reply/error_data/parameter_value> \n`

### Response statuses
* Reserved: 0x0N
* Relay reponses: 0x1N
	* 0x10 - Success
		* `typewriter_reply` contains the typewriter's response
	* 0x11 - NACK/timeout - typewriter command was not acknowleged before timeout 
		* `error_data` is the index of the command byte which was not acknowledged (zero-indexed)
	* 0x12 - Batch failed
		* `error_data` is the index of the command which failed (zero-indexed)
	* 0x13 - Invalid typewriter command - the command to be relayed is invalid
		* `error_data` is the index of the command which failed (zero-indexed)
	* 0x14 - Invalid relay command byte - the relay command byte is invalid
		* `error_data` contains the command byte
	* 0x15 - Invalid relay command length - the length of the relay command is invalid
		* `error_data` contains the expected length
	* 0x16 - Relay command transmission timeout - the full relay command, including the 
	         terminating `\n`, was not received before the timeout
	    * `error_data` contains the relay command byte
* Parameter query responses: 0xeN
	* 0xe0 - Parameter query success
	* 0xe1 - Parameter query failed
	* 0xe3 - Invalid parameter
		* `error_data` contains the command byte
	* 0xe5 - Invalid query command length
		* `error_data` contains the expected length
	* 0xe6 - Query command transmission timeout
		* `error_data` contains the command byte
* Parameter config responses: 0xfN
	* 0xf0 - Parameter config success
	* 0xf1 - Parameter config failed
	* 0xf3 - Invalid parameter
		* `error_data` contains the command byte
	* 0xf5 - Invalid config command length
		* `error_data` contains the expected length
	* 0xf5 - Config command transmission timeout
		* `error_data` contains the command byte
* Panic: 0xff - things have gone off the rails
