import time
import collections
import numbers

import serial


class WWMode(object):
	def __init__(self, wwClient):
		self.wwClient = wwClient
		self.characterCounter = 0

	def __exit__(self, exception_type, exception_value, traceback):
		self.wwClient._exitMode()

	def switchMode(self, modeName):
		self.wwClient.connect()
		print(f'\n*** Switching to {modeName} mode ***')
		self.wwClient.ser.write(f'{modeName}\n'.encode())

		while True:
			line = self.wwClient.ser.readline().decode().strip()
			print(line)
			if line == '[BEGIN]':
				self.wwClient.state = modeName.upper()
				break

			# TODO - add timeout if we never see [BEGIN]


class WWReadMode(WWMode):
	def __init__(self, wwClient):
		super().__init__(wwClient)

		self.terminator = 0x0a	# \n

	def __enter__(self):
		super().switchMode('read')
		return self

	def read(self):
		print('*** Press CTRL+C to stop ***')
		try:
			while True:
				print(self.wwClient.ser.readline().decode().strip())
		except KeyboardInterrupt:
			return

class WWRelayMode(WWMode):
	def __init__(self, wwClient):
		super().__init__(wwClient)

		self.terminator = 0x0a	# \n

	def __enter__(self):
		super().switchMode('relay')
		return self

	def sendCommand(self, address, command, data=None):
		if data is None:
			data = [0, 0]
		elif not isinstance(data, collections.abc.Sequence):
			data = [data, 0]

		if address is not None:
			commandBytes = [0x10, address, command] + data
		else:
			commandBytes = [0x11, command] + data

		commandBytes += [self.terminator]
		return self._transmitCommand(commandBytes)

	def sendCommandBatch(self, addresses, commands, data=None):
		if isinstance(commands, collections.abc.Sequence):
			batchSize = len(commands)
		elif isinstance(data, collections.abc.Sequence):
			batchSize = len(data)
		else:
			raise TypeError('At least one of [commands, data] should be a list or tuple!')

		if batchSize > 255:
			raise Exception('Batch sizes > 255 are not currently supported!')

		if not isinstance(commands, collections.abc.Sequence):
			commands = [commands for i in range(batchSize)]

		if data is None:
			data = [[0, 0] for i in range(batchSize)]
		elif not isinstance(data[0], collections.abc.Sequence):
			data = [data for i in range(batchSize)]

		if addresses is not None:
			if not isinstance(addresses, collections.abc.Sequence):
				addresses = [addresses for i in range(batchSize)]
			commandBytes = [0x12, batchSize]
			for address, command, dt in zip(addresses, commands, data):
				commandBytes += [address, command] + dt
			
		else:
			commandBytes = [0x13, batchSize]
			for command, dt in zip(commands, data):
				commandBytes += [command] + dt
		
		commandBytes += [self.terminator]
		return self._transmitCommand(commandBytes)

	def _transmitCommand(self, command, successStatus=0x10):
		command = bytearray(command)
		ifCommand = command[0]
		print('')
		self.printCommand(command)
		self.wwClient.ser.write(command)

		while True:
			response = self.wwClient.ser.readline()
			self.printResponse(response)
			if response[0] == ifCommand:
				if response[1] == successStatus:
					print(f'Success - command returned 0x{response[2]:x}')
				else:
					print(f'ERROR! interface returned error status 0x{response[1]:x} with data 0x{response[2]:x} ')
				break

		return response[2]

	@classmethod
	def hexBytesToString(cls, array):
		return ' '.join([f"0x{v:02x}" for v in array])

	@classmethod
	def printCommand(cls, array):
		print(f'Command:  [{cls.hexBytesToString(array)}]')

	@classmethod
	def printResponse(cls, array):
		print(f'Response: [{cls.hexBytesToString(array)}]')


class WWTypeMode(WWMode):
	def __init__(self, wwClient):
		super().__init__(wwClient)
		self.characterCounter = 0

	def __enter__(self):
		super().switchMode('type')
		return self

	def sendTextLines(self, textLines):
		if self.wwClient.state != 'TYPE':
			raise RuntimeError('Not in type mode!')

		for textLine in textLines:
			for char in textLine:
				self.wwClient.ser.write(char.encode())
				time.sleep(0.05)
				self.characterCounter += 1

	def sendTextFile(self, filename, endLines=0):
		with open(filename, 'rt') as f:
			textLines = f.readlines()
			self.sendTextLines(textLines)

		for i in range(endLines):
			self.wwClient.ser.write('\n'.encode())
			time.sleep(0.05)

		print(f'\n*** Sent {self.characterCounter} characters ***')


class WheelwriterInterface(object):
	MAX_RETRIES = 5
	def __init__(self, device, baudrate=115200, xonxoff=True):
		self.ser = serial.Serial(device, baudrate, xonxoff=xonxoff)

		self.state = None
	
	def __enter__(self):
		return self
	
	def __exit__(self, exception_type, exception_value, traceback):
		if self.state and self.state != 'READY':
			self._exitMode()

		self.ser.close()

	def connect(self):
		if self.checkReady():
			return

		self._waitForReady()

	def checkReady(self):
		return self.state == 'READY'

	def _waitForReady(self):
		retryCounter = 0
		while True:
			print('\n*** Sending "\\n" ***')
			self.ser.write('\n'.encode());
			
			line = self.ser.readline().decode().strip()
			print(line)

			if line.startswith('###'):
				line = self.ser.readline().decode().strip()
				print(line)

			if line == '[READY]':
				self.state = 'READY'
				break

			retryCounter += 1
			if retryCounter >= self.MAX_RETRIES:
				raise TimeoutError('Failed to connect to Wheelwriter!')
			time.sleep(1)

	def _exitMode(self):
		print(f'\n*** Exiting {self.state.lower()} mode ***')
		self.ser.write(b'\x04')
		line = self.ser.readline().decode().strip()
		print(line)
		self.state = 'READY'


class WheelwriterClient(object):
	wwCommandByte = {'queryModel': 0x00,
					 'reset': 0x01,
					 'type': 0x02,
					 'typeAndAdvance': 0x03,
					 'eraseAndAdvance': 0x04,
					 'movePlaten': 0x05,
					 'moveCarriage': 0x06,
					 'spinWheel': 0x07,
					 'queryPrintwheel': 0x08,
					 'unknown0x9': 0x09,
					 'unknown0xa': 0x0a,
					 'unknown0xb': 0x0b,
					 'unknown0xc': 0x0c,
					 'unknown0xd': 0x0d,
					 'unknown0xe': 0x0e
					}
	wwCommand = {v:k for k, v in wwCommandByte.items()}
	wwCarriageDirection = {'left': 0x00, 'right': 0x80}
	wwModelByte = {'unknown': 0x00,
				   'wheelwriter 3': 0x06,
				   'wheelwriter 5': 0x25,
				   'wheelwriter 6': 0x26
				  }
	wwModel = {v:k for k, v in wwCommandByte.items()}
	wwUnderscorePosition = 0x4f
	wwPlatenDirection = {'down': 0x00, 'up': 0x80}
	wwPrintWheelByte = {'proportional': 0x08,
						'15 cpi': 0x10,
						'12 cpi': 0x20,
						'no wheel': 0x21,
						'10 cpi': 0x40,
						}
	wwPrintWheel = {v:k for k, v in wwPrintWheelByte.items()}

	keyboardByte = {'us': 1, 
					'germany': 26, 
					'uk': 67, 
					'spain': 70, 
					'ascii': 103, 
					'symbol1': 200, 
					'symbol2': 202, 
					'symbol3': 203, 
					'ussr': 231}

	ascii2Wheel = {
	    1: [ # US
		# col: 00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F    row:
		#      NUL   SOH   STX   ETX   EOT   ENQ   ACK   BEL   BS    HT    LF    VT    FF    CR    SO    SI
			   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # 00
		#      DLE   DC1   DC2   DC3   DC4   NAK   SYN   ETB   CAN   EM    SUB   ESC   FS    GS    RS    US
			   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # 10
		#      SP     !     "     #     $     %     &     '     (     )     *     +     ,     -     .     /
			   0x00, 0x49, 0x4b, 0x38, 0x37, 0x39, 0x3f, 0x4c, 0x23, 0x16, 0x36, 0x3b, 0x0c, 0x0e, 0x57, 0x28, # 20
		#       0     1     2     3     4     5     6     7     8     9     :     ;     <     =     >     ?
			   0x30, 0x2e, 0x2f, 0x2c, 0x32, 0x31, 0x33, 0x35, 0x34, 0x2a ,0x4e, 0x50, 0x00, 0x4d, 0x00, 0x4a, # 30
		#       @     A     B     C     D     E     F     G     H     I     J     K     L     M     N     O
			   0x3d, 0x20, 0x12, 0x1b, 0x1d, 0x1e, 0x11, 0x0f, 0x14, 0x1F, 0x21, 0x2b, 0x18, 0x24, 0x1a, 0x22, # 40
		#       P     Q     R     S     T     U     V     W     X     Y     Z     [     \     ]     ^     _   
			   0x15, 0x3e, 0x17, 0x19, 0x1c, 0x10, 0x0d, 0x29, 0x2d, 0x26, 0x13, 0x41, 0x00, 0x40, 0x00, 0x4f, # 50
		#       `     a     b     c     d     e     f     g     h     i     j     k     l     m     n     o
			   0x00, 0x01, 0x59, 0x05, 0x07, 0x60, 0x0a, 0x5a, 0x08, 0x5d, 0x56, 0x0b, 0x09, 0x04, 0x02, 0x5f, # 60
		#       p     q     r     s     t     u     v     w     x     y     z     {     |     }     ~    DEL  
			   0x5c, 0x52, 0x03, 0x06, 0x5e, 0x5b, 0x53, 0x55, 0x51, 0x58, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, # 70
		#     
			   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # 80      
		#     
			   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # 90      
		#                   ¢                             §                                                  
			   0x00, 0x00, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # A0      
		#       °     ±     ²     ³                 ¶                                   ¼     ½              
			   0x44, 0x3C, 0x43, 0x42, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x47, 0x00, 0x00, # B0
		#     
			   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # C0 
		#     
			   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # E0 
		#     
			   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # D0 
		#     
			   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00] # F0
		,
		103: [ # ASCII
		# col: 00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F    row:
		#      NUL   SOH   STX   ETX   EOT   ENQ   ACK   BEL   BS    HT    LF    VT    FF    CR    SO    SI
      		   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # 00
		#      DLE   DC1   DC2   DC3   DC4   NAK   SYN   ETB   CAN   EM    SUB   ESC   FS    GS    RS    US
    		   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # 10
		#      SP     !     "     #     $     %     &     '     (     )     *     +     ,     -     .     /
		       0x00, 0x49, 0x4b, 0x38, 0x37, 0x39, 0x3f, 0x4c, 0x23, 0x16, 0x36, 0x3b, 0x0c, 0x0e, 0x57, 0x28, # 20
		#       0     1     2     3     4     5     6     7     8     9     :     ;     <     =     >     ?
		       0x30, 0x2e, 0x2f, 0x2c, 0x32, 0x31, 0x33, 0x35, 0x34, 0x2a ,0x4e, 0x50, 0x45, 0x4d, 0x46, 0x4a, # 30
		#       @     A     B     C     D     E     F     G     H     I     J     K     L     M     N     O
		       0x3d, 0x20, 0x12, 0x1b, 0x1d, 0x1e, 0x11, 0x0f, 0x14, 0x1F, 0x21, 0x2b, 0x18, 0x24, 0x1a, 0x22, # 40
		#       P     Q     R     S     T     U     V     W     X     Y     Z     [     \     ]     ^     _   
		       0x15, 0x3e, 0x17, 0x19, 0x1c, 0x10, 0x0d, 0x29, 0x2d, 0x26, 0x13, 0x41, 0x42, 0x40, 0x3a, 0x4f, # 50
		#       `     a     b     c     d     e     f     g     h     i     j     k     l     m     n     o
		       0x3c, 0x01, 0x59, 0x05, 0x07, 0x60, 0x0a, 0x5a, 0x08, 0x5d, 0x56, 0x0b, 0x09, 0x04, 0x02, 0x5f, # 60
		#       p     q     r     s     t     u     v     w     x     y     z     {     |     }     ~    DEL  
		       0x5c, 0x52, 0x03, 0x06, 0x5e, 0x5b, 0x53, 0x55, 0x51, 0x58, 0x54, 0x48, 0x43, 0x47, 0x44, 0x00, # 70
		#  	   
		       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # 80      
		#     
		       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # 90      
		#                   ¢                             §                                                  
		       0x00, 0x00, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # A0      
		#       °     ±     ²     ³                 ¶                                   ¼     ½              
		       0x44, 0x3C, 0x43, 0x42, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x47, 0x00, 0x00, # B0
		#     
			   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # C0 
		#     
			   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # E0 
		#     
			   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, # D0 
		#     
			   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00] # F0
		}

	def __init__(self, sender):
		self.sender = sender
		self.charSpace = 10
		self.lineSpaceSingle = 16
		self.lineSpacing = 1
		self.carriagePosition = 0
		self.keyboard = 'us'

	def carriageReturn(self):
		self.moveCarriage(-self.carriagePosition);
		self.carriagePosition = 0;

	def erase(self, wheelPosition, advanceUsteps, style=None):
		if style is None:
			style = 'normal'

		advanceUsteps = min(advanceUsteps, 0x7f) # 7-bit value
		if style == 'normal':
			self._sendCommand(self.wwCommandByte['eraseAndAdvance'], [wheelPosition, advanceUsteps])
			self.carriagePosition += advanceUsteps

	def lineFeed(self, direction=None):
		usteps = self.lineSpacing * self.lineSpaceSingle
		if direction == 'down':
			usteps = -usteps
		self.movePlaten(usteps)

	def moveCarriage(self, usteps):
		numSteps = abs(usteps)
		numSteps = min(numSteps, 0x7fff)  # 11-bit value
		direction = 'left' if usteps < 0 else 'right'

		data1 = (numSteps >> 8) | self.wwCarriageDirection[direction]
		data2 = numSteps & 0x00ff

		self._sendCommand(self.wwCommandByte['moveCarriage'], [data1, data2])

		if direction == 'right':
			self.carriagePosition += numSteps;
		else:
			self.carriagePosition -= numSteps;

	def moveCarriageNumSpaces(self, spaces):
		self.moveCarriage(self.charSpace * spaces)

	def movePlaten(self, usteps):
		numSteps = abs(usteps)
		numSteps = min(numSteps, 0x7f) # 7-bit value
		direction = 'down' if usteps < 0 else 'up'

		data = numSteps | self.wwPlatenDirection[direction]

		self._sendCommand(self.wwCommandByte['movePlaten'], data)

	def movePlatenNumLines(self, lines):
		self.movePlaten(self.lineSpaceSingle * lines * self.lineSpacing)

	def queryModel(self):
		return self._sendCommand(self.wwCommandByte['queryModel'])

	def queryPrintwheel(self):
		return self._sendCommand(self.wwCommandByte['queryPrintwheel'])

	def reset(self):
		self._sendCommand(self.wwCommandByte['reset'])

	def spinWheel(self):
		self._sendCommand(self.wwCommandByte['spinWheel'])

	def type(self, wheelPosition, advanceUsteps=None, style=None):
		if style is None:
			style = 'normal'

		if advanceUsteps is None:
			advanceUsteps = self.charSpace
		if style == 'normal':
			self._sendCommand(self.wwCommandByte['typeAndAdvance'], [wheelPosition, advanceUsteps])
			self.carriagePosition += advanceUsteps
		else:
			self.typeInPlace(wheelPosition, style)
			if 'bold' in style:
				advanceUsteps -= 1
			self.moveCarriage(advanceUsteps)

	def typeInPlace(self, wheelPosition, style=None):
		if style is None:
			style = 'normal'

		if wheelPosition != self.wwUnderscorePosition:
			self._sendCommand(self.wwCommandByte['type'], wheelPosition)

		if 'underline' in style:
			self._sendCommand(self.wwCommandByte['type'], self.wwUnderscorePosition)
		if 'bold' in style:
			self.moveCarriage(1)
			self._sendCommand(self.wwCommandByte['type'], wheelPosition)

	def typeCharacter(self, character, advanceUsteps=None, keyboard=None, style=None):
		position = self.wheelPosition(character, keyboard)
		self.type(position, advanceUsteps, style)

	def typeCharacterInPlace(self, character, keyboard=None, style=None):
		position = self.wheelPosition(character, keyboard)
		self.typeInPlace(position, style)

	def typeMultiple(self, wheelPositions, advanceUsteps=None, style=None):
		if advanceUsteps is None:
			advanceUsteps = self.charSpace

		for wheelPosition in wheelPositions:
			self.type(wheelPosition, advanceUsteps, style)

	def wheelPosition(self, character, keyboard=None):
		if keyboard is None:
			keyboard = self.keyboard

		return self.ascii2Wheel[self.keyboardByte[keyboard]][ord(character)]

	def setLeftMargin(self):
		self.carriagePosition = 0

	def setCharSpaceForWheel(self, wheelByte=None):
		if wheelByte is None:
			wheelByte = self.queryPrintwheel()

		if wheelByte == self.wwPrintWheelByte['10 cpi']:
			self.lineSpaceSingle = 16
			self.charSpace = 12
		elif wheelByte == self.wwPrintWheelByte['12 cpi']:
			self.lineSpaceSingle = 16
			self.charSpace = 10

	def setKeyboard(self, keyboard):
		if keyboard not in self.keyboardByte.keys():
			raise ValueError(f'Unknown keyboard: {keyboard}!')
		self.keyboard = keyboard

	def setLineSpacing(self, spacing):
		if spacing not in [1, 1.5, 2, 3]:
			raise ValueError('Invalid line spacing value!')
		self.lineSpacing = spacing

	def _sendCommand(self, command, data=None):
		return self.sender.sendCommand(None, command, data)
