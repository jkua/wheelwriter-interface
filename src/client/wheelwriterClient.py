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
		self.printCommand(command)
		self.wwClient.ser.write(command)

		while True:
			response = self.wwClient.ser.readline()
			self.printResponse(response)
			if response[0] == ifCommand:
				if response[1] == successStatus:
					print(f'Success - query returned 0x{response[2]:x}')
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


class WheelwriterClient(object):
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


