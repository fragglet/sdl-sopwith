#!/usr/bin/env python3
#
# Sopwith terrain generator script.
#

from math import exp
from random import random

territories = []
ground = []
objects = []
curr_y = 0

def shape_fn(x, center=100):
	if False and x < 4:
		return 0
	# No variation at small level or large level.
	x = (x - center) * 2 / center
	return max(exp(-(x ** 2)) - 0.03, 0)

def fillground(start_x, end_x, rockiness):
	if end_x - start_x < 2:
		 return

	start_y = ground[start_x]
	end_y = ground[end_x]
	mid_y = (start_y + end_y) / 2
	width = end_x - start_x
	yrange = 200 * rockiness * shape_fn(width)
#print("// start_x: %d end_x: %d width: %d range: %d" % (start_x, end_x, width, yrange))

	min_y = max(30, mid_y - yrange / 2)
	max_y = min(199, mid_y + yrange / 2)
	mid_x = int((start_x + end_x) / 2)
	mid_y = (min_y + max_y) / 2
	yrange = max_y - min_y

	brange = (yrange / 2) ** (1 / bias)
	r = random() ** (width / 500)
	if random() < 0.5:
		r = -r
	offset = (brange * r) ** bias
	ground[mid_x] = mid_y + offset
#print("// mid_y: %d yrange: %d, [%d]=%d" % (mid_y, yrange, mid_x, ground[mid_x]))

	fillground(start_x, mid_x, rockiness)
	fillground(mid_x, end_x, rockiness)

def terrain(width, end_y=None, rockiness=0.3):
	global curr_y
	if end_y is None:
		end_y = curr_y
	start_x = len(ground)
	end_x = start_x + width
	ground.extend(0 for _ in range(width))
	ground[start_x] = curr_y
	ground[end_x - 1] = end_y
	fillground(start_x, end_x - 1, rockiness)
	curr_y = end_y

def flat_ground(width):
	ground.extend([curr_y] * width)

def slope(end_y):
	global curr_y
	while curr_y != end_y:
		if curr_y < end_y:
			curr_y += 1
		else:
			curr_y -= 1
		ground.append(curr_y)

def left_barrier(end_y=30):
	global curr_y
	curr_y = 199
	flat_ground(300)
	slope(end_y + 50)
	terrain(70, end_y=end_y, rockiness=0.1)
	curr_y = end_y

def right_barrier():
	terrain(70, end_y=curr_y + 50, rockiness=0.1)
	slope(199)
	flat_ground(300)

def mountain(width=300, height=150, end_y=None):
	if end_y is None:
		end_y = curr_y
	quarter_width = int(width * 0.25)
	terrain(quarter_width, end_y=height * 0.9)
	terrain(quarter_width, end_y=height)
	terrain(quarter_width, end_y=height * 0.9)
	terrain(width - quarter_width * 3, end_y=end_y)

def airfield(width=200, owner="PLAYER1", right_side=False):
	x = len(ground)
	mult = 1
	flat_ground(width)
	if right_side:
		mult = -1
		x = len(ground) - 16
	# Oil tank
	add_object(type="TARGET", x=x, orient=2, owner=owner)
	# Hangar
	add_object(type="TARGET", x=x+mult*24, orient=0, owner=owner)
	add_object(type="PLANE", x=x+mult*48, orient=1 if right_side else 0, owner=owner)

def print_object(o):
	o = dict(o)
	t = o["territory"]
	del o["territory"]
	if "territory_l" not in o:
		o["territory_l"] = territories[t][0]
		o["territory_r"] = territories[t][1]
	print("\tobject {")
	for k, v in sorted(o.items()):
		print("\t\t%s: %s" % (k, v))
	print("\t}")

def add_object(**kwargs):
	kwargs["territory"] = len(territories)
	objects.append(kwargs)

def new_territory():
	start_x = (territories or [(0, 0)])[-1][1]
	territories.append((start_x, len(ground)))

def convoy(width=200, max_tanks=3, type="TARGET", orient=3):
	start_x = len(ground)
	terrain(width, rockiness=0.03)

	x = start_x
	tanks = 0
	while tanks < max_tanks and x < start_x + width - 64:
		ground_slice = ground[x:x+48]
		if max(ground_slice) - min(ground_slice) < 2:
			add_object(type=type, x=x+16, orient=orient,
			           owner="PLAYER2")
			tanks += 1
			x += 32
			continue

		x += 1

def oxen_field(width=200, max_oxen=3):
	convoy(width=width, max_tanks=max_oxen, type="OX", orient=0)

left_barrier()
terrain(500)
oxen_field()
airfield()
terrain(200, rockiness=0.05)
mountain(end_y=50)
mountain(height=90, width=150, end_y=30)
new_territory()
mountain(height=140, width=120, end_y=40)
terrain(300, rockiness=0.1)
convoy()
airfield(owner="PLAYER2", right_side=True)
terrain(500, rockiness=0.3)
terrain(500, rockiness=0.1)
right_barrier()
new_territory()

print("level {")

for o in objects:
	print_object(o)

print("\tground {")

for idx, g in enumerate(ground):
	if (idx % 8) == 0:
		print("\t\t", end="")
	else:
		print(" ", end="")
	print("_: %3d" % int(g + 0.5), end="")
	if (idx % 8) == 7:
		print("")
	#print('#' * int(g))
	#print('%d: %d' % (idx, g))

print("\n\t}\n}")

