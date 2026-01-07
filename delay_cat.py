#!/usr/bin/env python3

import argparse

parser = argparse.ArgumentParser(add_help=False)
parser.add_argument('-d', '--delay', type=float, default=0.5)
args, _ = parser.parse_known_args()

DELAY = args.delay

#----------------
import sys, time, heapq, select

# DELAY = 0.5
pq = []  # (emit_time, line)

stdin = sys.stdin
stdout = sys.stdout

while True:
    now = time.monotonic()

    # 计算下一个需要输出的时间
    timeout = None
    if pq:
        timeout = max(0, pq[0][0] - now)

    # 等待 stdin 可读 或 到达下一个 emit 时间
    r, _, _ = select.select([stdin], [], [], timeout)

    if r:
        line = stdin.readline()
        if not line:   # EOF
            break
        heapq.heappush(pq, (time.monotonic() + DELAY, line))

    # 输出所有到期的行
    now = time.monotonic()
    while pq and pq[0][0] <= now:
        _, line = heapq.heappop(pq)
        stdout.write(line)
        stdout.flush()

# stdin 结束后，flush 剩余
while pq:
    t, line = heapq.heappop(pq)
    time.sleep(max(0, t - time.monotonic()))
    stdout.write(line)
    stdout.flush()


