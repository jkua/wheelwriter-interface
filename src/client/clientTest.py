#!/usr/bin/env python3

import time
import sys

from wheelwriterClient import WheelwriterClient, WheelwriterInterface, WWRelayMode

if __name__=='__main__':
	import argparse
	parser = argparse.ArgumentParser()
	parser.add_argument('--device', '-d', required=True, help='Serial port to connect to')
	args = parser.parse_args()

	with WheelwriterInterface(args.device) as interface:
		with WWRelayMode(interface) as relay:

			client = WheelwriterClient(relay)

			print('\nQuery model')
			model = client.queryModel()
			print(f'Model: 0x{model:02x}')

			print('\nQuery printwheel')
			wheel = client.queryPrintwheel()
			print(f'Wheel: 0x{wheel:02x}')

			client.setCharSpaceForWheel(wheel)

			client.setKeyboard('us')

			client.setLeftMargin();

			print('\nType and advance - normal')
			spacing = 0x0a
			characters = [0x01, 0x59, 0x05]
			for character in characters:
				client.type(character, spacing)

			print('\nMove carriage one space')
			client.moveCarriage(client.charSpace)
			
			print('\nType and advance - bold')
			spacing = 0x0a
			characters = [0x01, 0x59, 0x05]
			for character in characters:
				client.type(character, spacing, 'bold')

			print('\nMove carriage one space')
			client.moveCarriageNumSpaces(1)
			
			print('\nType and advance - underline')
			spacing = 0x0a
			characters = [0x01, 0x59, 0x05]
			for character in characters:
				client.type(character, spacing, 'underline')

			print('\nMove carriage one space')
			client.moveCarriageNumSpaces(1)

			print('\nType and advance - bold+underline')
			spacing = 0x0a
			characters = [0x01, 0x59, 0x05]
			for character in characters:
				client.type(character, spacing, 'bold+underline')

			client.carriageReturn()
			client.lineFeed()


			print('\nCircle test')
			string = "Hello, world! Lorem ipsum dolor sit amet. "
			dx = [  15,  15,  14,  14,  11,  11,   8,   6,   4,   2, 
					-1,  -3,  -5,  -7,  -9, -11, -13, -14, -14, -15, 
				   -16, -15, -14, -14, -13, -11,  -9,  -7,  -5,  -3, 
					-1,   2,   4,   6,   8,  11,  11,  14,  14,  15,  15]
			dy = [  -1,  -3,  -4,  -7,  -7, -10, -10, -11, -12, -12, 
				   -12, -12, -12, -10, -10,  -9,  -7,  -5,  -4,  -2, 
					 0,   2,   4,   5,   7,   9,  10,  10,  12,  12, 
					12,  12,  12,  11,  10,  10,   7,   7,   4,   3,   1]

			# Move to center
			client.moveCarriage(100)
			for character, x, y in zip(string, dx, dy):
				client.typeCharacterInPlace(character)
				client.moveCarriage(x)
				client.movePlaten(-y)

			client.carriageReturn();
			client.movePlaten(127)
			client.movePlaten(127)
