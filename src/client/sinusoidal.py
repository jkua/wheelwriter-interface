#!/usr/bin/env python3

import numpy as np

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


			string = "Life is sinusoidal, with ups and downs; first flying high, then wearing a frown"

			x = np.arange(0, 800, client.charSpace)
			y = np.round(np.sin(x/790*4*np.pi) * client.lineSpaceSingle * 5)

			dx = np.diff(x).astype(int)
			dy = np.diff(-y).astype(int)

			client.setLeftMargin();

			for char, xStep, yStep in zip(string, dx, dy):
				client.typeCharacter(char, xStep)
				client.movePlaten(yStep)

			client.carriageReturn()
			client.movePlatenNumLines(8)

