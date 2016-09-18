#!/usr/bin/env python

from __future__ import print_function

import butterfly as bf

WALL = 0
FLOOR = 1

room = bf.Butterfly(*[
    [bf.MORPH_AT_RANDOM_SPOT],
    [bf.LOOK_RECT_AREA, [FLOOR, 3, 1]],
    [bf.LOOK_RECT_AREA, [FLOOR, 5, 3]],
    [bf.LOOK_RECT_AREA, [FLOOR, 6, 4]],
    [bf.DIE_AFTER_N, [1]]
], error_on_looking_at_safe_neighbor_8=True,
   error_on_looking_outside_farm=True)

tunnel = bf.Butterfly(*[
    [bf.MORPH_AT_LAST_DEATH_SPOT],
    [bf.GOAL_RANDOM_SAFE_SPOT],
    [bf.FLUTTER_TUNNEL],
    [bf.LOOK_1_AREA, [FLOOR]],
    [bf.DIE_AT_SAFE_SPOT]
])

f = bf.Farm(80, 24)
f.spawn(bf.random_1x1(FLOOR), commit=True)
f.spawn([room, tunnel], 50, commit=True)

print(f.seed())

for y in xrange(24):
    for x in xrange(80):
    	if f.spot_at(x, y) == 0:
        	print("#", end="")
        else:
        	print(".", end="")
    print()
