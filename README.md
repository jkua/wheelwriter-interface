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
electronics are an [Arduino Nano RP2040 Connect](https://docs.arduino.cc/hardware/nano-rp2040-connect)
and a custom shield to interface with the Wheelwriter's option connector. 

2. [Arduino Wheelwriter driver](src/arduino/wheelwriter_interface) - this 
communicates with the Wheelwriter over the serial bus and provide low-level 
control of the typewriter, including platen and carriage positioning and 
typehead imprinting.

3. REST service - the built-in WiFi connectivity on the Nano supports a REST 
server which allows a modern computer, smartphone, or tablet to control the
typewriter via the interface module.

4. Client software - this software will demonstrate the use of the REST API

## Related work
[IBM-Wheelwriter-Hack](https://github.com/tofergregg/IBM-Wheelwriter-Hack) by 
Chris Gregg/[tofergregg](https://github.com/tofergregg) is what initially 
inspired me to look into Wheelwriters. His [videos](https://www.youtube.com/watch?v=Awxbu8y5cv8) 
[on](https://www.youtube.com/watch?v=0vrvDQmZcPI) 
[YouTube](https://www.youtube.com/watch?v=5FTS4fj5Im4) showed his work on 
reverse engineering the Wheelwriter's bus with a logic analyzer in about 2017.
His implementation bit-bangs a TX line to drive a N-channel MOSFET.

Be sure to check out his 
[Smith Corona typewriter to printer conversion](https://www.youtube.com/watch?v=le4C2HeNrdQ)!

Ryan Jarvis/[Cabalist](https://github.com/Cabalist) provided a 
[complete printwheel mapping](https://github.com/tofergregg/IBM-Wheelwriter-Hack/issues/5) 
for US language printwheels.

[Russell Senior](https://github.com/RussellSenior) made the connection that 
this is an Intel 8051 9-bit serial bus in mode 2 and provided a 
[great wiki page](https://github.com/RussellSenior/IBM-Wheelwriter-Hack/wiki/Bus-Protocol) 
describing a great deal of the command set.

[jim11662418](https://github.com/jim11662418) created implementations for 
the 8051 compatible [Dallas Semi DS89C440](https://github.com/jim11662418/wheelwriter-printer)
and [STC Micro STC15W4K32S4](https://github.com/jim11662418/wheelwriter-teletype)
that are very nice.

[MicroCoreLabs](https://github.com/MicroCoreLabs) created an 
[Lattice FPGA implementation](https://github.com/MicroCoreLabs/Projects/tree/master/Wheelwriter) 
and an [Arduino Leonardo implementation](https://github.com/MicroCoreLabs/Projects/tree/master/Wheelwriter2). 
Both implement a serial port that translates an ASCII character stream to motor 
controller commands. The Arduino implementation directly uses the ATmega32u4's 
UART an interestingly toggles the transmitter off when not sending. 
[Here's a YouTube video](https://www.youtube.com/watch?v=q8gCYw75E1A) of the 
FPGA version running.

[CadetWriter](https://github.com/IBM-1620/Cadetwriter) is an effort by Dave 
Babcock and Stephen Casner to interface to a Wheelwriter 1000 by instead 
sitting between the keyboard and the logic controller. This work was developed 
to build a general-purpose ASCII terminal.
