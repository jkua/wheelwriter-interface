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

			print('\nType and advance - normal')
			spacing = 0x0a
			characters = [0x01, 0x59, 0x05]
			for character in characters:
				client.type(character, spacing)
			
			print('\nType and advance - bold')
			spacing = 0x0a
			characters = [0x01, 0x59, 0x05]
			for character in characters:
				client.type(character, spacing, 'bold')
			
			print('\nType and advance - underline')
			spacing = 0x0a
			characters = [0x01, 0x59, 0x05]
			for character in characters:
				client.type(character, spacing, 'underline')

			print('\nType and advance - bold+underline')
			spacing = 0x0a
			characters = [0x01, 0x59, 0x05]
			for character in characters:
				client.type(character, spacing, 'bold+underline')
			
			