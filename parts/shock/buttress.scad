include <dim.scad>
use <vents.scad>

EPS = 0.1;
EPS2 = EPS * 2;

module buttress(battery=false, pcb=0, crystal="none", showBolt=false, rods=true, altRod=false, biasRight=false, lowerWiring=false, upperWiring=false, crystalHolder=0) {

    H_C = crystalHolder > 0 ? crystalHolder : H_CRYSTAL;
    
    difference() {
        cylinder(h=H_BUTTRESS, r=D_INNER/2, $fn=FACES);
        
        if (battery) {
            translate([0, 0, -EPS]) {
               cylinder(h=H_BUTTRESS + EPS2, r=D_BATTERY/2, $fn=FACES);    
            }
        }
        
        if(pcb > 0) {
            OFFSET = 40;

            translate([-W_PCB/2 + (biasRight ? 0 : -OFFSET), -11, -EPS]) {
                // Front: H_PCB
                // Back pins: H_PCB + 1
                // Center: 12
                cube(size=[W_PCB + OFFSET, pcb, H_BUTTRESS + EPS2]);
            }
        }

        if (upperWiring) {
            for(r=[0:1]) {
                bias = (r==0) ? 1 : -1;
                translate([9 * bias, 8, -EPS]) {
                    cylinder(r=2, h=H_BUTTRESS + EPS2, $fn=40);
                }
            }
        }

        if (lowerWiring) {
            for(r=[0:1]) {
                bias = (r==0) ? 1 : -1;
                translate([9 * bias, -2, -EPS]) {
                    cylinder(r=2, h=H_BUTTRESS + EPS2, $fn=40);
                }
            }
        }

        if (crystal == "body" || crystal == "tip") {
            translate([0, CRYSTAL_Y, -EPS]) {
                if (crystal == "body") {
                    scale([W_CRYSTAL, H_C, 1]) {
                        cylinder(h=H_BUTTRESS + EPS2, r=0.5, $fn=FACES);   
                    }
                }
                else {
                    cylinder(h=H_BUTTRESS + EPS2, d=D_CRYSTAL_TIP, $fn=FACES);   
                }

            }
            translate([0, CRYSTAL_Y/2 + 2, -EPS]) 
            { 
                scale([0.8, 1, 1]) {
                    rotate([0, 0, 45]) {
                        cube(size=[20, 20, H_BUTTRESS + EPS2]);
                    }
                }
            }
        }

        if (rods) {     
            translate([X_ROD, Y_ROD, 0]) {
                cylinder(d=D_ROD, h=H_BUTTRESS + EPS2, $fn=FACES);
            }
        }
        if (altRod) {
            translate([-X_ROD, Y_ROD, 0]) {
                cylinder(d=D_ROD, h=H_BUTTRESS + EPS2, $fn=FACES);
            }
        }

        // Notch.
        NOTCH_ANGLES = [NOTCH_ANGLE_0, NOTCH_ANGLE_1];
        for(angle = NOTCH_ANGLES) {
            rotate([0, 0, angle]) {
                translate([-R_INNER, -NOTCH_WIDTH/2, -EPS]) {
                    cube(size=[NOTCH_DEPTH, NOTCH_WIDTH, H_BUTTRESS + EPS2]);
                }
            }
        }
    }

    if (crystalHolder > 0) {
        translate([0, CRYSTAL_Y, -H_CRYSTAL_HOLDER]) {
            difference() {
                MULT = 1.15;
                scale([W_CRYSTAL * MULT, H_C * MULT, 1]) {
                    cylinder(h=H_CRYSTAL_HOLDER + H_BUTTRESS, r=0.5, $fn=FACES);   
                }
                scale([W_CRYSTAL, H_C, 1]) {
                    cylinder(h=H_CRYSTAL_HOLDER + H_BUTTRESS, r=0.5, $fn=FACES);   
                }
                translate([-10, -3, 0]) {
                    cube(size=[20, 6, H_CRYSTAL_HOLDER/2]);
                }
            }
        }
    }

    if (showBolt) {
        for(r=[0:1]) {
            translate([0, ROD_Y, 0]) {
                rotate([0, 0, r*180]) {
                    translate([ROD_X, 0, H_BUTTRESS]) {
                        color("red") {
                            cylinder(r=D_ROD/2, h=2);
                        }
                        color("green") {
                            cylinder(r=4.8, h=1);
                        }
                    }
                }
            }
        }
    }
}
