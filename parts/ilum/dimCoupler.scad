HEADER_PINS = 6.1;
TOOTH_CAP = 1.0;
TOOTH_BASE = 8.6;   // height w/o teeth caps
TOOTH_HEIGHT = TOOTH_BASE + TOOTH_CAP;
TOOTH_BASE_H = TOOTH_BASE - HEADER_PINS - TOOTH_CAP; // need space for teeth caps to sink in
COUPLER_PLASTIC_HEIGHT = TOOTH_HEIGHT + TOOTH_BASE_H + 0.1; // not sure why the extra

H_COUPLER_PCB = 1.5;
R_COUPLER_MOUNT = 2.54 * 3.0 + 0.5;
D_COUPLER_INNER = (2.54 * 3.0 - 1.5) * 2;
D_COUPLER_OUTER = 20.24; //(2.54 * 3.0 + 1.5) * 2 + 2.0;
D_M2_HEAD = 4.0;
D_M2 = 1.8;
D_COUPLER_DISC = 22.0;

R_M2_FASTENER_BOLT = (D_COUPLER_OUTER - D_M2_HEAD) / 2;
