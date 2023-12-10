#!/usr/bin/env python3

import serial
import time
import sys

# from wheelwriterClient import WheelwriterClient, WWTypeMode

if __name__=='__main__':
	import argparse
	parser = argparse.ArgumentParser()
	parser.add_argument('--device', '-d', required=True, help='Serial port to connect to')
	parser.add_argument('file', help='Text file to send')
	parser.add_argument('--endlines', '-e', type=int, default=0, help='Additional carriage returns at the end')
	parser.add_argument('--keyboard', '-k', type=int, choices=[1, 103], default=1, help='Keyboard layout')
	args = parser.parse_args()

	# with WheelwriterClient(args.device) as client:
	# 	with WWTypeMode(client) as typeMode:
	# 		typeMode.sendTextFile(args.file, endLines=args.endlines)

	# sys.exit()

	infile = open(args.file, 'rt')
	textLines = infile.readlines()

	retryCounter = 0
	characterCounter = 0
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

		print('\n*** Switching to type mode ***')
		print(f'type {args.keyboard}\n')
		ser.write(f'type {args.keyboard}\n'.encode())

		while True:
			line = ser.readline().decode().strip()
			print(line)
			if line == '[BEGIN]':
				break

		for textLine in textLines:
			for char in textLine:
				ser.write(char.encode())
				time.sleep(0.05)
				characterCounter += 1

		for i in range(args.endlines):
			ser.write('\n'.encode())
			time.sleep(0.05)

		print('\n*** Exiting type mode ***')
		ser.write(b'\x04')
		line = ser.readline().decode().strip()
		print(line)

	print(f'\n*** Sent {characterCounter} characters ***')