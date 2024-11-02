# Wheelwriter Interface Board REST API

This describes the REST API that allows access to Wheelwriter functions.

## Endpoints:
Note that parameters are sent as a comma-separated list.
* `/` (**GET**) - returns a simple web page
* `/type` (**POST**) - send an ASCII file to this endpoint and the typewriter will 
type it. Supports the same ANSI/CSI-style escape codes that the [serial console](wwib_serial_protocol.md) 
does.
* `/query` (**POST**) - responds with a JSON listing the Wheelwriter model, 
printwheel pitch, and status
* `/characterTest` (**POST**) - types all the characters on the printwheel
* `/circleTest` (**POST**) - types `Lorem ipsum` in a circle
* `/printwheelSample` (**POST**) - types a formatted printwheel sample
* `/readLine` (**POST**) - reads a line of text from the typewriter. This will 
wait for a carriage return. There is a configurable timeout in milliseconds 
from the last typed character. The terminating newline is included. By default, 
backspace characters are not included and the corrected line is returned.
	* timeout (int) - the number of milliseconds after the last character 
	entered to wait for return to be pressed. 0 (default) waits indefinitely
	* corrected (bool) - return the corrected line. Default is 1. If 0, the 
	uncorrected line in returned, including backspace characters.

## Setup
Use the serial console to setup the WiFi with the `wifi` command. The IP address 
will be listed and you should get a simple web page if you access that in a 
browser. Currently only DHCP is supported for IP address configuration.

## Examples
* `curl -X POST http://<ip_address>/type -d "test1234"` causes your Wheelwriter to type `test1234`
* `curl -X POST http://<ip_address>/type -d "test^[[1m1234"` will output the same, but `1234` will be bold
* `curl -X POST http://<ip_address>/type --data-binary "@<filename>"` to send a text file
* `curl -X POST http://<ip_address>/query` returns something like `{"model":6,"wheel":32,"status":0}`
