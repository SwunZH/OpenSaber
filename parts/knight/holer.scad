include <dim.scad>
use <../shapes.scad>
include <dim.scad>

//INCHES_TO_MM = 25.4;
//MM_TO_INCHES = 1/ INCHES_TO_MM;

D_OUTER = D_SABER_OUTER + 20;       //
D_TUBE = 6;							// FIXME
D_BIT  = 5;							// FIXME
H = 10;
H_SLEEVE = 2;						// FIXME
THETA = 73.5;

module puzzle() {
	translate([0, 0, H/2]) {
		rotate([90, 0, 90]) {
			linear_extrude(height=30) {
				W = 22;
				polygon([[-W,0], [W,0], [W-4,10], [-W+4,10]]);
			}
		}
	}
}

//difference()
intersection()
{
	difference() 
	{
		tube(H, D_OUTER/2, D_SABER_OUTER/2);
		translate([0,0,H/2]) {
			rotate([0,90,0]) {
				for(r=[0:3]) {
					rotate([r*THETA/3 - THETA/2, 0, 0]) {
	                    cylinder(h=50, r=D_BIT/2, $fn=28);
	                    translate([0,0,D_SABER_OUTER /2 + H_SLEEVE]) {
	                        cylinder(h=H, r=D_TUBE/2,$fn=28);
	                    }
					}
				}
			}
			*translate([-100,-10,-D_TUBE/2]) {
				cube([100,20,D_TUBE]);
			}
		}
	}
	puzzle();
}