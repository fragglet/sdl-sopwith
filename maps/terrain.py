#!/usr/bin/env python3
#
# Sopwith terrain generator script.
#

from math import exp
from random import random

MIRROR = 6

context_vars = {}
ground = []
objects = []
curr_y = 0

def push_context(**overrides):
	global context_vars
	result = context_vars
	context_vars = context_vars.copy()
	context_vars.update(overrides)
	return result

def pop_context(old):
	global context_vars
	context_vars = old

class Context(object):
	def __init__(self, **kwargs):
		self.overrides = kwargs

	def __enter__(self):
		self.saved = push_context(**self.overrides)

	def __exit__(self, *_):
		pop_context(self.saved)

def defaults(**default_vals):
	def decorator(func):
		def inner(*args, **kwargs):
			kwargs = kwargs.copy()
			for k, v in default_vals.items():
				if k not in kwargs:
					kwargs[k] = context_vars.get(k, v)
			func(*args, **kwargs)
		return inner
	return decorator

class Territory(object):
	def __init__(self, **kwargs):
		self.overrides = kwargs

	def __enter__(self):
		self.start_x = len(ground)
		self.first_object = len(objects)
		self.saved = push_context(**self.overrides)

	def __exit__(self, *_):
		pop_context(self.saved)
		end_x = len(ground)
		# Go back and set territory boundaries for all planes.
		for o in objects[self.first_object:]:
			if o["type"] == "PLANE":
				o.setdefault("territory_l", self.start_x)
				o.setdefault("territory_r", end_x)

def EnemyTerritory(**kwargs):
	kwargs.setdefault("owner", "PLAYER2")
	return Territory(**kwargs)

def shape_fn(x, center=150):
	# No variation at small level or large level.
	x = (x - center) * 2 / center
	return max(exp(-(x ** 2)) - 0.03, 0)

def interpolate_fn(coord0, coord1, coord2):
	"""Generate quadratic interpolation function from coords."""
	x0, y0 = coord0
	x1, y1 = coord1
	x2, y2 = coord2
	L0 = lambda x: (x - x1) * (x - x2) / ((x0 - x1) * (x0 - x2))
	L1 = lambda x: (x - x0) * (x - x2) / ((x1 - x0) * (x1 - x2))
	L2 = lambda x: (x - x0) * (x - x1) / ((x2 - x0) * (x2 - x1))
	return lambda x: (y0 * L0(x) + y1 * L1(x) + y2 * L2(x)).real

def biased_random(yrange):
	bias = 3.0
	brange = (yrange / 2) ** (1 / bias)
	r = random() ** (yrange / 100)
	r = ((brange * r) ** bias).real
	if random() < 0.5:
		r = -r
	return r

def fillground(start_x, end_x, rockiness, guidepoint):
	"""Fill ground[start_x:end_x] with fractal landscape."""
	start_y = ground[start_x]
	end_y = ground[end_x]

	if end_x - start_x < 2:
		 return (min((start_x, end_x)), min((start_y, end_y)))

	# Generate new interpolated midpoint.
	mid_x = (start_x + end_x) // 2
	fn = interpolate_fn(guidepoint, (start_x, start_y), (end_x, end_y))
	base_y = fn(mid_x)

	width = end_x - start_x
	yrange = 300 * rockiness * shape_fn(width)

	min_y = max(30, base_y - yrange / 2)
	max_y = min(180, base_y + yrange / 2)
	base_y = (min_y + max_y) / 2
	yrange = max_y - min_y

	offset = biased_random(yrange)
	ground[mid_x] = base_y + offset

	next_guide = fillground(start_x, mid_x, rockiness, guidepoint)
	fillground(mid_x, end_x, rockiness, next_guide)

	return (mid_x, base_y + offset)

@defaults(rockiness=0.3)
def terrain(width, *, rockiness, end_y=None):
	global curr_y
	if end_y is None:
		end_y = curr_y
	start_x = len(ground)
	end_x = start_x + width
	ground.extend(0 for _ in range(width))
	ground[start_x] = curr_y
	ground[end_x - 1] = end_y
	midpoint = (start_x - 32, curr_y)
	fillground(start_x, end_x - 1, rockiness, midpoint)
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

def target_type(orient):
	return lambda x: add_object(type="TARGET", x=x, orient=orient)

hangar = target_type(0)
building = target_type(1)
oil_tank = target_type(2)
tank = target_type(3)
truck = target_type(4)
tanker_truck = target_type(5)
flag_pole = target_type(6)
tent = target_type(7)

def plane(x):
	add_object(type="PLANE", x=x)

def ox(x):
	add_object(type="OX", x=x, mirror=random() < 0.5)

@defaults(mirror=False)
def airfield(*, mirror, width=200):
	x = len(ground)
	mult = 1
	flat_ground(width)
	if mirror:
		mult = -1
		x = len(ground) - 16
	# Oil tank
	oil_tank(x=x)
	# Hangar
	hangar(x=x+mult*24)
	plane(x=x+mult*48)

@defaults(mirror=False, owner="PLAYER1", territory=None)
def add_object(mirror, territory, **kwargs):
	if mirror:
		if kwargs["type"] == "PLANE":
			kwargs["orient"] = 1
		else:
			kwargs["transform"] = 6
	if territory is not None:
		kwargs["territory"] = territory
	objects.append(kwargs)

@defaults(max_tanks=3)
def convoy(max_tanks, width=200, callback=tank):
	start_x = len(ground)
	terrain(width, rockiness=0.03)

	x = start_x
	tanks = 0
	while tanks < max_tanks and x < start_x + width - 64:
		ground_slice = ground[x:x+48]
		ground_min = min(ground_slice)
		ground_max = max(ground_slice)
		if ground_max - ground_min < 2:
			callback(x=x+16)
			# Flatten
			for x2 in range(x + 8, x + 40):
				ground[x2] = ground_max
			tanks += 1
			x += 32
			continue

		x += 1

@defaults(max_oxen=3)
def oxen_field(max_oxen, width=200):
	convoy(width=width, max_tanks=max_oxen, callback=ox)

def print_object(file, o):
	o = dict(o)
	print("\tobject {", file=file)
	for k, v in sorted(o.items()):
		print("\t\t%s: %s" % (k, v), file=file)
	print("\t}", file=file)

def print_ground(file):
	print("\tground {", file=file)

	for idx, g in enumerate(ground):
		if (idx % 8) == 0:
			print("\t\t", end="", file=file)
		else:
			print(" ", end="", file=file)
		print("_: %3d" % int(g + 0.5), end="", file=file)
		if (idx % 8) == 7:
			print("", file=file)

	print("\n\t}", file=file)

def write_to_file(filename):
	with open(filename, "w") as file:
		print("level {", file=file)

		for o in objects:
			print_object(file, o)

		print_ground(file)
		print("}", file=file)

