#!/usr/bin/env python3

import time
import sys

from wheelwriterClient import WheelwriterClient, WWReadMode

if __name__=='__main__':
	import argparse
	parser = argparse.ArgumentParser()
	parser.add_argument('--device', '-d', required=True, help='Serial port to connect to')
	args = parser.parse_args()

	with WheelwriterClient(args.device) as client:
		with WWReadMode(client) as reader:
			reader.read()
			