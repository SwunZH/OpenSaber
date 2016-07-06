use <../threads.scad>
use <../shapes.scad>
include <dim.scad>

INCHES_TO_MM = 25.4;
MM_TO_INCHES = 1 / 25.4;
FACES = 180;

H_HEAT_SINK_THREAD = 0.37 * INCHES_TO_MM;
H_VOL			   = 18;

RENDER_THREADS = false;

// LED / button positive (above axis)
// thread negative (below axis)
//
module switch(drawThread)
{
	D_OUTER_TOP = 13.8;
	D_INNER_TOP = 11.0;
	H_TOP       =  1.5;
	H_BODY		= 13;	// approx. connections below.

	color("green") {
		cylinder(h = H_TOP, r1 = D_OUTER_TOP / 2, r2 = D_INNER_TOP / 2);
		translate([0, 0, -H_BODY]) {
			if (drawThread) {
				metric_thread(diameter=12.0, length=H_BODY, pitch=0.75);
			}
			else {
				cylinder(h = H_BODY, r = 6.0);
			}
		}
	}
}

module emitterVolume()
{
	translate([0, 0, H_HEAT_SINK_THREAD]) {
		cylinder(h=H_VOL, r = INCHES_TO_MM / 2, $fn = FACES);
		translate([0, 0, H_VOL]) {
			cylinder(h=3, r = (1 - 1/8) * INCHES_TO_MM / 2, $fn = FACES);
		}
	}
}


*switch(false);

// LED holder
difference() {
	cylinder(h=H_HEAT_SINK_THREAD, r = INCHES_TO_MM / 2, $fn=FACES);
	if (RENDER_THREADS) {
		english_thread(diameter=0.5, length=H_HEAT_SINK_THREAD * MM_TO_INCHES, threads_per_inch=14, internal=true);
	}
	else {
		cylinder(h=H_HEAT_SINK_THREAD, r = 0.5 * INCHES_TO_MM / 2);
	}
	
}

X_SWITCH = 9.6;
Z_SWITCH = 8;

// Switch holder
/*
translate([-X_SWITCH,0,18])
{
	rotate([0, -90, 0]) {
   		
        translate([0, 0, 0]) {
			switch(false);
		}
        difference() 
		{
			cylinder(r=15/2, h=Z_SWITCH);
			cylinder(r=12/2, h=Z_SWITCH);
		}
	}
}
*/

// Switch holder v2
T_HOLDER = 3;
difference() 
{
    intersection() 
    {
        emitterVolume();
        translate([-X_SWITCH, -20, H_HEAT_SINK_THREAD]) {
            cube([T_HOLDER, 40, 40]);
        }
    }    
    translate([-X_SWITCH, 0, H_HEAT_SINK_THREAD + Z_SWITCH]) {
        rotate([0, 90, 0]) {
            if (RENDER_THREADS) {
                metric_thread(diameter=12.0, length=T_HOLDER + 0.1, pitch=0.75, inner=true);
            }
            else {
                cylinder(r=6, h=T_HOLDER + 0.1);
            }
        }
    }
}

*translate([-X_SWITCH, 0, H_HEAT_SINK_THREAD + Z_SWITCH]) {
    rotate([0, -90, 0]) {
        switch(false);
    }
}