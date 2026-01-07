#!/usr/bin/env python3

import sys
import time
import asyncio
import argparse

parser = argparse.ArgumentParser(add_help=False)
parser.add_argument('-d', '--delay', type=float, default=0.5)
args, _ = parser.parse_known_args()

#----------------
DELAY = args.delay
loop = asyncio.get_event_loop()

pending = 0
done = asyncio.Event()

def emit(line):
    global pending
    sys.stdout.write(line)
    sys.stdout.flush()
    pending -= 1
    if pending == 0 and done.is_set():
        loop.stop()

async def main():
    global pending
    reader = sys.stdin

    for line in reader:
        pending += 1
        loop.call_later(DELAY, emit, line)

    # stdin EOF
    done.set()
    if pending == 0:
        loop.stop()

loop.create_task(main())
loop.run_forever()


