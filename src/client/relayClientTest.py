#!/usr/bin/env python3

import time
import sys

from wheelwriterClient import WheelwriterClient, WWRelayMode

if __name__=='__main__':
	import argparse
	parser = argparse.ArgumentParser()
	parser.add_argument('--device', '-d', required=True, help='Serial port to connect to')
	args = parser.parse_args()

	with WheelwriterClient(args.device) as client:
		with WWRelayMode(client) as relay:
			
			# Single command - full
			print('\nSending single command - full')
			address = 0x21
			wwCommand = 0x08 # Query printwheel
			data = [0, 0]
		
			relay.sendCommand(address, wwCommand, data)

			# Single command - abbreviated
			print('\nSending single command - abbreviated')
			relay.sendCommand(None, wwCommand, data)

			# Batch command - full
			print('\nSending batch command - full')
			spacing = 0x0a
			data = []
			data.append([0x01, spacing])
			data.append([0x59, spacing])
			data.append([0x05, spacing])
			data.append([0x07, spacing])
			wwCommand = 0x03	# Type and advance
			relay.sendCommandBatch(address, wwCommand, data)
		
			# Batch command - abbreviated
			print('\nSending batch command - abbreviated')
			data = []
			data.append([0x60, spacing])
			data.append([0x0a, spacing])
			data.append([0x5a, spacing])
			data.append([0x08, spacing])
			relay.sendCommandBatch(None, wwCommand, data)
			