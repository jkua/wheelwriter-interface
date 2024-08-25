#!/usr/bin/env python3

import serial
import time
import sys

# from wheelwriterClient import WheelwriterClient, WWTypeMode

def hexBytesToString(array):
	return ' '.join([f"0x{v:02x}" for v in array])

def printCommand(array):
	print(f'Command:  [{hexBytesToString(array)}]')

def printResponse(array):
	print(f'Response: [{hexBytesToString(array)}]')

def sendCommand(ser, command, successStatus=0x10):
	ifCommand = command[0]
	printCommand(command)
	ser.write(command)
	
	while True:
		response = ser.readline()
		printResponse(response)
		if response[0] == ifCommand:
			if response[1] == successStatus:
				print(f'Success - query returned 0x{response[2]:x}')
			else:
				print(f'ERROR! interface returned error status 0x{response[1]:x} with data 0x{response[2]:x} ')
			break


if __name__=='__main__':
	import argparse
	parser = argparse.ArgumentParser()
	parser.add_argument('--device', '-d', required=True, help='Serial port to connect to')
	args = parser.parse_args()

	# with WheelwriterClient(args.device) as client:
	# 	with WWTypeMode(client) as typeMode:
	# 		typeMode.sendTextFile(args.file, endLines=args.endlines)

	# sys.exit()

	retryCounter = 0
	with serial.Serial(args.device, 115200, xonxoff=True) as ser:
		while True:
			print('\n*** Sending "\\n" ***')
			ser.write('\n'.encode());
			
			line = ser.readline().decode().strip()
			print(line)

			if line.startswith('###'):
				line = ser.readline().decode().strip()
				print(line)

			if line == '[READY]':
				break

			retryCounter += 1
			if retryCounter >= 5:
				print('\nERROR - Failed to connect!')
				sys.exit(1)
			time.sleep(1)

		print('\n*** Switching to relay mode ***')
		commandString = f'relay'
		print(f'{commandString}\n')
		ser.write(f'{commandString}\n'.encode())

		while True:
			line = ser.readline().decode().strip()
			print(line)
			if line == '[BEGIN]':
				break

		# Single command - full
		print('\nSending single command - full')
		ifCommand = 0x10
		address = 0x21
		wwCommand = 0x08 # Query printwheel
		data = [0, 0]
		terminator = 0x0a
		command = bytearray([ifCommand, address, wwCommand, data[0], data[1], terminator])
		sendCommand(ser, command)

		# Single command - abbreviated
		print('\nSending single command - abbreviated')
		ifCommand = 0x11
		command = bytearray([ifCommand, wwCommand, data[0], data[1], terminator])
		sendCommand(ser, command)

		# Batch command - full
		print('\nSending batch command - full')
		ifCommand = 0x12
		batchSize = 4
		wwCommand = 0x03 # Type and advance
		spacing = 0x0a
		command = [ifCommand, batchSize]
		command += [address, wwCommand, 0x01, spacing]
		command += [address, wwCommand, 0x59, spacing]
		command += [address, wwCommand, 0x05, spacing]
		command += [address, wwCommand, 0x07, spacing]
		command += [terminator]
		command = bytearray(command)
		sendCommand(ser, command)
		
		# Batch command - abbreviated
		print('\nSending batch command - abbreviated')
		ifCommand = 0x13
		batchSize = 4
		wwCommand = 0x03 # Type and advance
		spacing = 0x0a
		command = [ifCommand, batchSize]
		command += [wwCommand, 0x60, spacing]
		command += [wwCommand, 0x0a, spacing]
		command += [wwCommand, 0x5a, spacing]
		command += [wwCommand, 0x08, spacing]
		command += [terminator]
		command = bytearray(command)
		sendCommand(ser, command)

		print('\n*** Exiting type mode ***')
		ser.write(b'\x04')
		line = ser.readline().decode().strip()
		print(line)

