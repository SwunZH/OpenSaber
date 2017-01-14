include <dim.scad>

EPS = 0.01;
EPS2 = EPS * 2;

module battery(h)
{
//	BATTERY_Y = 6;
	BATTERY_Y = R_AFT - R_BATTERY;

	translate([0, BATTERY_Y, 0]) {
	    cylinder(h=h, d=D_BATTERY);

	    translate([-BATTERY_CUTOUT/2, 0, 0]) {
	        cube(size=[BATTERY_CUTOUT, 40, h]);
	    }
	}
}

module circuitry(h, deltaY)
{
	W_MC 	= 18;
	H_MC    = 10;
	Y_MC    = -12;

	translate([-W_MC/2, Y_MC, 0]) cube(size=[W_MC, H_MC, h]);
	translate([-W_WING/2, Y_MC + H_MC, 0]) cube(size=[W_WING, H_WING + deltaY, h]);
}

module buttress(leftWiring=true, rightWiring=true, battery=true, mc=true, mcDeltaY=0, trough=false)
{
	difference() {
		cylinder(h=H_BUTTRESS, d=D_AFT);	

		// Battery
		if (battery) {
			translate([0, 0, -EPS]) battery(H_BUTTRESS + EPS2);
		}

		// Board
		if (mc) {
			translate([0, 0, -EPS]) circuitry(H_BUTTRESS + EPS2, mcDeltaY);
		}

		if (trough) {
            translate([-BATTERY_CUTOUT/2, -20, -EPS]) {
                cube(size=[BATTERY_CUTOUT, 40, H_BUTTRESS + EPS2]);
            }
		}

	    // Wiring holes
	    X_WIRING = 11.5;
	    Y_WIRING = -5.5;

	    if (leftWiring) {
		    translate([ X_WIRING, Y_WIRING, -EPS]) cylinder(h=H_BUTTRESS + EPS2, d=3.5);
	    }
	    if (rightWiring) {
		    translate([-X_WIRING, Y_WIRING, -EPS]) cylinder(h=H_BUTTRESS + EPS2, d=3.5);
	    }
	}
}

