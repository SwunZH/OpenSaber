M_POMMEL = -26.0;
DZ_SPKR = 12.0;
M_MC_BATTERY = M_POMMEL + DZ_SPKR;
M_0 = 0;
M_BODY_END = 7 * 25.4;
M_EXT_END = M_BODY_END + 18.80;

M_SWITCH = 104.4;
M_PORT = M_SWITCH - 16.0;
M_ALIGN = M_SWITCH;
D_CAPSULE = 12.0;

N_BATT_BAFFLES = 10;
DZ_BAFFLE = 3.7;
M_JOINT = M_MC_BATTERY + (N_BATT_BAFFLES * 2 - 1)*DZ_BAFFLE;

// M_FIRST_DOTSTAR = 137.0;    // front
//M_FIRST_DOTSTAR = 86.2;
M_FIRST_DOTSTAR = M_ALIGN - 7 * 2.5;    // back one dotstar

M_PCB_COUPLER = M_EXT_END 
    - 28.9  // to base of thread
    + 12.9  // to emitter pcb
    - 12.4; // header + coupler pcb

M_DESIGN_MIN = M_BODY_END - 58.6;   // no physical limit except the rings
M_DESIGN_MAX_THREAD = M_BODY_END - 15.66;  // threads
M_DESIGN_MAX_COUPLER = M_PCB_COUPLER;
M_DESIGN_MAX = M_DESIGN_MAX_THREAD;

D_INNER = 31.60;
D_RING = 32.0;  // fixme
D_OUTER = 36.9;

OUTER_RING_START = 17.70;
OUTER_RING_SPACE = 14.19;
OUTER_RING_WIDTH = 1.9;
OUTER_RING_DEPTH = 0.65;
N_OUTER_RING = 8;

DO_COPPER = 24.2;
DI_COPPER = 22.23;
DZ_COPPER = 42.20;
M_COPPER = M_DESIGN_MIN + 4.0;

DO_BRASS = D_INNER;
DI_BRASS = 28.96;
M_BRASS = 80.0;
DZ_BRASS = (M_COPPER + DZ_COPPER) - M_BRASS; // trying to avoid cutting the copper // DZ_COPPER + 4.0;

