#!/usr/bin/env python

from __future__ import print_function

import butterfly as bf

WIDTH = 80
HEIGHT = 40

WALL = 0
CAVE_WALL = 1
FLOOR = 2
CAVE_FLOOR = 3

farm = bf.Farm(WIDTH, HEIGHT)
farm.last_dangerous(CAVE_WALL)

cave = bf.Butterfly(*[
    [bf.MORPH_AT_RANDOM_SPOT],
    [bf.GOAL_RANDOM_SAFE_SPOT],
    [bf.FLUTTER_WEIGHTED_4, [70]],
    [bf.LOOK_PLUS_AREA, [CAVE_FLOOR]],
    [bf.DIE_AT_SAFE_SPOT]
], enable_neighbor_look_8=True, neighbor_look_8=CAVE_WALL)

room = bf.Butterfly(*[
    [bf.MORPH_AT_RANDOM_SPOT],
    [bf.LOOK_SHRINKING_RECT_AREA, [FLOOR, 3, 8]],
    [bf.LOOK_SHRINKING_CIRCLE_AREA, [FLOOR, 3, 8]],
    [bf.DIE_AFTER_N, [1]]
], cancel_on_looking_at_safe=True)

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
        s = farm.spot_at(x, y)
    	if s == WALL:
        	print("#", end="")
        elif s == CAVE_WALL:
            print("%", end="")
        elif s == FLOOR:
        	print(".", end="")
        elif s == CAVE_FLOOR:
            print(",", end="")
    print()
