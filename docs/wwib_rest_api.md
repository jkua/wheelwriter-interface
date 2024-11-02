# Wheelwriter Interface Board REST API

This describes the REST API that allows access to Wheelwriter functions.

## Endpoints:
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
wait for a carriage return. Sending an integer in the request will configure 
a timeout in milliseconds from the last typed character. Backspaces and 
newlines are included in the response.

## Setup
Use the serial console to setup the WiFi with the `wifi` command. The IP address 
will be listed and you should get a simple web page if you access that in a 
browser. Currently only DHCP is supported for IP address configuration.

## Examples
* `curl -X POST http://<ip_address>/type -d "test1234"` causes your Wheelwriter to type `test1234`
* `curl -X POST http://<ip_address>/type -d "test^[[1m1234"` will output the same, but `1234` will be bold
* `curl -X POST http://<ip_address>/type --data-binary "@<filename>"` to send a text file
* `curl -X POST http://<ip_address>/query` returns something like `{"model":6,"wheel":32,"status":0}`