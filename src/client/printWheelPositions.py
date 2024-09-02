#!/usr/bin/env python3

# This takes a text file of comma separated wheel positions and sends them to be printed
# -1 is a space
# 0 - 95 are regular type style
# 96 - 191 are bold type style

import numpy as np

from wheelwriterClient import WheelwriterClient, WheelwriterInterface, WWRelayMode

if __name__=='__main__':
	import argparse
	parser = argparse.ArgumentParser()
	parser.add_argument('input', help='Text file of wheel positions to print')
	parser.add_argument('--device', '-d', required=True, help='Serial port to connect to')
	args = parser.parse_args()

	with open(args.input, 'rt') as f:
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

				for line in f:
					tokens = line.strip().split(',')
					positions = [int(t) for t in tokens]

					for position in positions:
						if position == -1:
							client.moveCarriageNumSpaces(1)
						elif position < 96:
							client.type(position+1)
						else:
							client.type((position % 96)+1, style='bold')

					client.carriageReturn()
					client.movePlatenNumLines(1)

