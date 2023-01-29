# Low-level IBM Wheelwriter interface
This project is to build a low-level interface for an IBM Wheelwriter 
typewriter. This builds on work by 
[tofergregg](https://github.com/tofergregg/IBM-Wheelwriter-Hack), 
[RussellSenior](https://github.com/tofergregg/IBM-Wheelwriter-Hack/pull/11), 
and [jim11662418](https://github.com/jim11662418/wheelwriter-teletype) 
who figured out the internal serial communication bus and the protocol it uses.

This project was built and tested on an IBM Wheelwriter 3. Likely this will 
work on at least Series I Wheelwriters (3/5/6), and possibly later models as 
well.

## Architecture
This project consists of:

1. Hardware interface module - this attaches to the back of the Wheelwriter as
the OEM printer option did, and connect to the 10-pin option connector located 
behind the 70 mm-wide hinged panel on the top rear of the typewriter. The core 
electronics are an [Arduino Nano RP2040](https://docs.arduino.cc/hardware/nano-rp2040-connect)
and a custom shield to interface with the Wheelwriter's option connector. 

2. Arduino Wheelwriter driver - this communicates with the Wheelwriter over the 
serial bus and provide low-level control of the typewriter, including platen 
and carriage positioning and typehead imprinting.

3. REST service - the built-in WiFi connectivity on the Nano supports a REST 
server which allows a modern computer, smartphone, or tablet to control the
typewriter via the interface module.

4. Client software - this software will demonstrate the use of the REST API

## Related work
[CadetWriter](https://github.com/IBM-1620/Cadetwriter) is an effort by Dave 
Babcock and Stephen Casner to interface to a Wheelwriter 1000 by instead 
sitting between the keyboard and the logic controller. This work was developed 
to build a general-purpose ASCII terminal.
