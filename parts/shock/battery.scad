include <dim.scad>
use <vents.scad>
use <../shapes.scad>

$fn = 90;

module battery()
{
	translate([-X_BAT_CASE / 2, -Y_BAT_CASE /2, 0]) {
		difference() {
			cube(size=[X_BAT_CASE, Y_BAT_CASE*.7, H_BAT_CASE]);	

			{
				cube(size=[BAT_CASE_NOTCH, BAT_CASE_NOTCH, H_BAT_CASE]);
			}
			translate([X_BAT_CASE - BAT_CASE_NOTCH, 0, 0]) {
				cube(size=[BAT_CASE_NOTCH, BAT_CASE_NOTCH, H_BAT_CASE]);
			}
		}
	}
	translate([0, (Y_BAT_CASE - D_BATTERY)/2, 0]) {
		cylinder(h=H_BAT_CASE, d=D_BATTERY);
	}
}


difference() {
	cylinder(h=H_BAT_CASE, d=D_AFT);
	translate([0, 1.5, 0]) {
		battery();
	}

	for(r=[0:5]) {
        H = H_BAT_CASE / 4 - 1;
		rotate([0, 0, r*60]) {
			translate([0, 0, 2]) {
				vent2(8, H, 20);
			}
			translate([0, 0, 2 + H*2]) {
				vent2(8, H, 20);
			}
		}
		rotate([0, 0, r*60 + 30]) {
			translate([0, 0, 2 + H]) {
				vent2(8, H, 20);
			}		
			translate([0, 0, 2 + H * 3]) {
				vent2(8, H, 20);
			}		
		}
	}
}
