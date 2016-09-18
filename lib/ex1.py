#!/usr/bin/env python

from __future__ import print_function

import butterfly as bf

instincts = [
    [bf.MORPH_AT_RANDOM_SPOT],
    [bf.LOOK_PLUS_AREA, [1]],
    [bf.DIE_AFTER_N, [1]]
]

f = bf.Farm(80, 24)
bf = bf.Butterfly(*instincts, error_on_looking_at_safe_neighbor_4=True)
f.spawn(bf, 20, commit=True)

for y in xrange(24):
    for x in xrange(80):
    	if f.spot_at(x, y) == 0:
        	print("#", end="")
        else:
        	print(".", end="")
    print()
