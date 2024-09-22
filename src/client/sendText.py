#!/usr/bin/env python3

import serial
import time
import sys

from wheelwriterClient import WheelwriterInterface, WWTypeMode

if __name__=='__main__':
	import argparse
	parser = argparse.ArgumentParser()
	parser.add_argument('--device', '-d', required=True, help='Serial port to connect to')
	parser.add_argument('file', help='Text file to send')
	parser.add_argument('--endlines', '-e', type=int, default=0, help='Additional carriage returns at the end')
	parser.add_argument('--keyboard', '-k', type=int, choices=[1, 103], default=1, help='Keyboard layout')
	parser.add_argument('--noPrintableEscape', action='store_true', help='Do not allow printable escape sequence ^[, binary only')
	args = parser.parse_args()

	with WheelwriterInterface(args.device) as interface:
		with WWTypeMode(interface) as typeMode:
			typeMode.sendTextFile(args.file, endLines=args.endlines)
