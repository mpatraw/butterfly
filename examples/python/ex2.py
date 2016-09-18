#!/usr/bin/env python

from __future__ import print_function

import butterfly as bf

WALL = 0
FLOOR = 1

tunnel = [
    [bf.MORPH_AT_RANDOM_SPOT],
    [bf.GOAL_RANDOM_SAFE_SPOT],
    [bf.FLUTTER_TUNNEL],
    [bf.LOOK_1_AREA, [1]],
    [bf.DIE_AT_SAFE_SPOT]
]

f = bf.Farm(80, 24, seed=1)
f.max_cancels(1)
f.spawn(bf.random_1x1(FLOOR), commit=True)
fly = bf.Butterfly(*tunnel,
				   look_method=bf.METHOD_CYCLE,
				   cancel_on_looking_at_safe_neighbor_4x=True)
f.spawn(fly, 1000, commit=True)

print(f.seed())

for y in xrange(24):
    for x in xrange(80):
    	if f.spot_at(x, y) == WALL:
        	print("#", end="")
        elif f.spot_at(x, y) == -1:
            print("?", end="")
        else:
        	print(".", end="")
    print()
