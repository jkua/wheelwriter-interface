#!/usr/bin/env python3

import serial
import time
import sys

# from wheelwriterClient import WheelwriterClient, WWTypeMode

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

		ser.write(bytearray([0x10, 0x21, 0x08, 0x00, 0x00, 0x0a]))
		
		while True:
			line = ser.readline()
			print(line)
			if line[0] == 0x10:
				if line[1] == 0x10:
					print(f'Success - query returned 0x{line[2]:x}')
				else:
					print(f'ERROR! interface returned error status 0x{line[1]:x} with data 0x{line[2]:x} ')
				break

		print('\n*** Exiting type mode ***')
		ser.write(b'\x04')
		line = ser.readline().decode().strip()
		print(line)

