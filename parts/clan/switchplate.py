import sys
from utility import *
from material import init_material
from rectangleTool import rectangleTool
from mecode import G
from hole import hole

# this assumes meshcam did the initial curvature & shaping

mat = init_material(sys.argv[1])

DZ_PLATE = 31.0
W_PLATE = 15.0
DZ_PORT, DZ_BOLT, DZ_SWITCH = 7.1, 17.5, 25.7
H_PLATE = 6.6

# D_SWITCH = 8.2
D_SWITCH = 4.4
INSET_WITH_BOLT = True

depth = -H_PLATE - 0.5

if H_PLATE < 6.35:
    raise RuntimeError("Need to implement planing the stock")

g = G(outfile='path.nc', aerotech_include=False, header=None, footer=None)
nomad_header(g, mat, CNC_TRAVEL_Z)
g.absolute()

travel(g, mat, y=DZ_PORT)
g.move(z=0)
hole(g, mat, depth, d=11.0)

travel(g, mat, y=DZ_BOLT)
g.move(z=0)
hole(g, mat, depth, d=4.4)

if INSET_WITH_BOLT:
    travel(g, mat, y=DZ_BOLT - 4.0)
    g.move(z=0)
    rectangleTool(g, mat, -2.0, 8.0, DZ_SWITCH - DZ_BOLT + 8.0, 8.0/2, "bottom", "inner", fill=True, adjust_trim=True)
else:
    g.move(z=0)
    hole(g, mat, -2.0, d=8.0)

travel(g, mat, y=DZ_SWITCH)
g.move(z=0)
hole(g, mat, depth, d=D_SWITCH)

# travel(g, mat, y=DZ_SWITCH - 11.0/2)
# rectangleTool(g, mat, sink, TROUGH, LENGTH - SWITCH + 16.0, TROUGH/2, "bottom", "inner", fill=True, adjust_trim=True)

travel(g, mat, x=0, y=0)
g.move(z=0)
rectangleTool(g, mat, depth, W_PLATE, DZ_PLATE, W_PLATE/2, "bottom", "outer", fill=False, adjust_trim=True)
