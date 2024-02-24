#!/usr/bin/env python3
#
# Player must fly over the mountains and destroy two tank convoys.

from terrain import *

left_barrier()

with Territory():
	terrain(500)
	oxen_field()
	airfield()
	terrain(200, rockiness=0.05)

mountain(end_y=50)
mountain(height=90, width=150, end_y=30)

with EnemyTerritory(mirror=True):
	mountain(height=140, width=120, end_y=40)
	terrain(300, rockiness=0.1)
	convoy()
	airfield()
	terrain(500, rockiness=0.3)
	convoy(max_tanks=5)
	terrain(500, rockiness=0.1)

right_barrier()

write_to_file("tank_strike.sop")

