import sys
from nanopcb import nanopcb
from utility import *
from material import init_material
from rectangleTool import rectangleTool
from mecode import G
from hole import hole

CUT_DEPTH = -1.8
PCB_DEPTH = -0.3
UNIT = 2.54
R_BOLT = UNIT * 3.0 + 0.5
D_M2 = 2.05

D_COUPLER_OUTER = 20.24
D_M2_HEAD = 4.0
R_M2_FASTENER_BOLT = (D_COUPLER_OUTER - D_M2_HEAD) / 2

D_COUPLER_DISC = 22.0


# 0 is the aft side
# 1 the fore side

back = sys.argv[1] == '0'

mat = init_material("np883-fr-1.0")
g = G(outfile='path.nc', aerotech_include=False, header=None, footer=None)
nomad_header(g, mat, CNC_TRAVEL_Z)

nanopcb("emitter.txt", g, mat, PCB_DEPTH, CUT_DEPTH, 
    False,  # don't cut
    False, 
    back == False, # drill
    back == True, # flip
    False)

g.absolute()

g.move(z=2)
g.move(x=0, y=0)

if back is False:
    INSET = 13.5 - 0.5
    D_VENT = 26.0
    dhy = math.sqrt(D_VENT*D_VENT/4 - INSET*INSET/4)

    bit = mat['tool_size']
    dhy = dhy + bit / 2
    INSET = INSET + bit

    travel(g, mat, x=INSET/2, y=dhy)
    g.spindle('CCW', mat['spindle_speed'])
    g.move(z=0)

    g.relative()
    def path(g, plunge, total_plunge):
        g.arc2(x=-INSET, y=0, i=-INSET/2, j=-dhy,  direction='CCW')
        g.move(x=0, y=-dhy*2, z=plunge/2)
        g.arc2(x=INSET, y=0, i=INSET/2, j=dhy, direction='CCW')
        g.move(x=0, y=dhy*2, z=plunge/2)

    steps = calc_steps(CUT_DEPTH, -mat['pass_depth'])
    run_3_stages(path, g, steps)

    g.absolute()

g.move(z=10)
g.spindle()

