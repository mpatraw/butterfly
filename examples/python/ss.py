#!/usr/bin/env python

from __future__ import print_function

import butterfly as bf

WIDTH = 80
HEIGHT = 40

WALL = 0
FLOOR = 1

farm = bf.Farm(WIDTH, HEIGHT)

cave = bf.Butterfly(*[
    [bf.MORPH_AT_RANDOM_SPOT],
    [bf.GOAL_RANDOM_SAFE_SPOT],
    [bf.FLUTTER_WEIGHTED_4, [51]],
    [bf.LOOK_PLUS_AREA, [FLOOR]],
    [bf.DIE_AT_SAFE_SPOT]
])

room = bf.Butterfly(*[
    [bf.MORPH_AT_RANDOM_SPOT],
    [bf.LOOK_SHRINKING_RECT_AREA, [FLOOR, 3, 8]],
    [bf.LOOK_SHRINKING_CIRCLE_AREA, [FLOOR, 3, 8]],
    [bf.DIE_AFTER_N, [1]]
], error_on_looking_at_safe=True)

tunnel = bf.Butterfly(*[
    [bf.MORPH_AT_LAST_DEATH_SPOT],
    [bf.GOAL_RANDOM_SAFE_SPOT],
    [bf.FLUTTER_TUNNEL],
    [bf.LOOK_1_AREA, [FLOOR]],
    [bf.DIE_AT_SAFE_SPOT]
])

farm.spawn(bf.random_1x1(FLOOR), commit=True)
tries = WIDTH * HEIGHT
while farm.safe_percentage() < 0.65 and tries > 0:
    tries -= 1
    if farm.random() < 0.3:
        farm.spawn(cave, commit=True)
    else:
        farm.spawn([room, tunnel], commit=True)

print(farm.seed())

for y in xrange(HEIGHT):
    for x in xrange(WIDTH):
    	if farm.spot_at(x, y) == 0:
        	print("#", end="")
        else:
        	print(".", end="")
    print()
