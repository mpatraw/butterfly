#!/usr/bin/env python

from __future__ import print_function

import butterfly as bf

seed = [
    [bf.MORPH_AT_RANDOM_SPOT],
    [bf.LOOK_1_AREA, [1]],
    [bf.DIE_AFTER_N, [1]]
]

tunnel = [
    [bf.MORPH_AT_RANDOM_SPOT],
    [bf.GOAL_RANDOM_SAFE_SPOT],
    [bf.FLUTTER_TUNNEL],
    [bf.LOOK_PLUS_AREA, [1]],
    [bf.LOOK_1_AREA, [1]],
    [bf.LOOK_1_AREA, [1]],
    [bf.LOOK_1_AREA, [1]],
    [bf.DIE_AT_SAFE_SPOT]
]

f = bf.Farm(80, 24)
f.spawn(bf.Butterfly(*seed), commit=True)
fly = bf.Butterfly(*tunnel,
				   look_method=bf.METHOD_CYCLE,
				   error_on_looking_at_safe_neighbor_4x=True)
f.spawn(fly, 100, commit=True)

print(f.seed())

for y in xrange(24):
    for x in xrange(80):
    	if f.spot_at(x, y) == 0:
        	print("#", end="")
        else:
        	print(".", end="")
    print()
