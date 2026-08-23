// ============================================================
// AEROGUARD-X1 - CASE
// Smart Gas Leak Detection System
// Group 1
//
// HOW TO USE:
// 1. Install OpenSCAD (free): https://openscad.org/downloads.html
// 2. Open this file in OpenSCAD
// 3. Change the "part" variable below to choose what to render:
//      - "all_export"  -> ALL 6 parts (base, lid, 2x sensor mount,
//                          vent frame, vent flap) laid out flat with
//                          gaps between them, ready to export as ONE
//                          STL and print as a single multi-part job.
//                          This is the easiest option -- use this one
//                          unless you need to isolate a single part.
//      - "base", "lid", "sensor_mount", "vent_flap", "vent_frame"
//                       -> exports just that one part on its own
//      - "all_preview"  -> shows all parts stacked/spread out for a
//                          quick visual check only -- do not export
//                          this one for printing
// 4. Press F6 (render) -- this can take a few seconds, wait for it
//    to finish before exporting
// 5. File > Export > Export as STL
// 6. Open the STL in your slicer (Cura, PrusaSlicer, etc.) -- each
//    shape will still be selectable/movable as a separate object
//    since none of them touch or overlap
// ============================================================

part = "all_export"; // options: "base", "lid", "sensor_mount", "vent_flap", "vent_frame", "all_preview", "all_export"

// ============================================================
// GLOBAL SETTINGS
// ============================================================
wall = 2;              // wall thickness (mm)
$fn = 48;               // circle smoothness

// Overall case internal dimensions
case_L = 160;
case_W = 110;
case_H = 45;

// ============================================================
// COMPONENT DIMENSIONS (real, standard sizes)
// ============================================================
uno_L = 68.6; uno_W = 53.4; uno_H = 15;
lcd_L = 80;   lcd_W = 36;   lcd_H = 20;
servo_L = 23; servo_W = 12.5; servo_H = 29;
sd_L = 28;    sd_W = 24;    sd_H = 10;
sim_L = 25;   sim_W = 24;   sim_H = 3;
led_d = 5;
button_d = 12;
buzzer_d = 12;

// ============================================================
// PART SELECTOR
// ============================================================
if (part == "base") base_tray();
if (part == "lid") lid();
if (part == "sensor_mount") sensor_mount();
if (part == "vent_flap") vent_flap();
if (part == "vent_frame") vent_frame();
if (part == "all_preview") {
    base_tray();
    translate([0, 0, case_H + 10]) lid();
    translate([case_L + 20, 0, 0]) sensor_mount();
    translate([case_L + 20, 60, 0]) vent_frame();
    translate([case_L + 20, 120, 0]) vent_flap();
}

// "all_export" - same as all_preview but laid flat on the print bed (no
// stacking), with generous gaps so the parts never touch or fuse, and
// each part printed twice where needed (2x sensor_mount for the 2 gas
// sensors). Safe to export as ONE STL and print as a single job -- most
// slicers (Cura, PrusaSlicer) will keep each shape as a separate,
// independently movable object even inside one STL, since they never
// touch or share geometry.
if (part == "all_export") {
    // Base tray - bottom-left
    translate([0, 0, 0])
        base_tray();

    // Lid - placed flat, to the right of the base, gap of 20mm
    translate([case_L + 20, 0, 0])
        lid();

    // Sensor mount #1 - below base tray, gap of 20mm
    translate([0, case_W + 20, 0])
        sensor_mount();

    // Sensor mount #2 - next to sensor mount #1, gap of 20mm
    translate([70, case_W + 20, 0])
        sensor_mount();

    // Vent frame - below sensor mounts, gap of 20mm
    translate([0, case_W + 20 + 40, 0])
        vent_frame();

    // Vent flap - next to vent frame, gap of 20mm
    translate([70, case_W + 20 + 40, 0])
        vent_flap();
}

// ============================================================
// BASE TRAY
// ============================================================
module base_tray() {
    difference() {
        // Outer shell
        cube([case_L, case_W, case_H]);

        // Hollow interior (leaving floor + walls)
        translate([wall, wall, wall])
            cube([case_L - 2*wall, case_W - 2*wall, case_H]);

        // USB port slot - back wall, left side
        translate([-1, 20, wall + 5])
            cube([wall + 2, 12, 8]);

        // Barrel jack hole - back wall
        translate([-1, 40, wall + 5])
            rotate([0, 90, 0])
                cylinder(h = wall + 2, d = 8);

        // Ventilation slots for gas sensor 1 - left side wall
        for (i = [0:3]) {
            translate([-1, 15 + i*8, case_H - 15])
                cube([wall + 2, 3, 20]);
        }

        // Ventilation slots for gas sensor 2 - left side wall (further along)
        for (i = [0:3]) {
            translate([-1, 60 + i*8, case_H - 15])
                cube([wall + 2, 3, 20]);
        }

        // Flame sensor opening - right side wall
        translate([case_L - wall - 1, 45, case_H - 15])
            cube([wall + 2, 10, 10]);

        // SIM800L antenna/wire notch - front wall
        translate([70, -1, case_H - 12])
            cube([15, wall + 2, 5]);
    }

    // Mounting standoffs for Arduino Uno
    // Standard Uno hole positions relative to board corner (approx, mm)
    uno_holes = [[15.24, 2.54], [15.24, 50.8], [66.04, 17.78], [66.04, 45.72]];
    uno_origin = [10, 10]; // where the Uno's corner sits inside the tray

    for (h = uno_holes) {
        translate([uno_origin[0] + h[0], uno_origin[1] + h[1], wall])
            difference() {
                cylinder(h = 6, d = 6);
                cylinder(h = 6.2, d = 2.5); // M2.5 self-tap screw hole
            }
    }

    // Mounting platform for SD card module
    translate([115, 10, wall])
        cube([sd_L + 4, sd_W + 4, 3]);

    // Mounting platform for SIM800L
    translate([115, 45, wall])
        cube([sim_L + 4, sim_W + 4, 3]);

    // Mounting posts for servo (simple raised block with strap slot concept)
    translate([10, 70, wall])
        cube([servo_L + 4, servo_W + 4, 4]);
}

// ============================================================
// LID
// ============================================================
module lid() {
    lid_L = case_L;
    lid_W = case_W;
    lid_thickness = wall;

    difference() {
        cube([lid_L, lid_W, lid_thickness]);

        // LCD window cutout, centered-left, with a 1.5mm ledge
        // (ledge created by only cutting most of the way through)
        translate([15, (lid_W - lcd_W)/2, -1])
            cube([lcd_L, lcd_W, lid_thickness - 1.5 + 2]);

        // LED holes - Yellow, Orange, Red (right side of lid)
        for (i = [0:2]) {
            translate([120, 25 + i*10, -1])
                cylinder(h = lid_thickness + 2, d = led_d);
        }

        // Push button hole
        translate([120, 70, -1])
            cylinder(h = lid_thickness + 2, d = button_d);

        // Buzzer hole
        translate([120, 90, -1])
            cylinder(h = lid_thickness + 2, d = buzzer_d);
    }

    // Small lip around LCD cutout underside for the screen to rest on
    // (printed as a thin raised frame on the underside of the lid)
    translate([15 - 2, (lid_W - lcd_W)/2 - 2, -2])
        difference() {
            cube([lcd_L + 4, lcd_W + 4, 2]);
            translate([2, 2, -1])
                cube([lcd_L, lcd_W, 4]);
        }
}

// ============================================================
// EXTERNAL GAS SENSOR MOUNT (print x2)
// ============================================================
module sensor_mount() {
    mount_L = 35;
    mount_W = 25;
    mount_H = 15;
    mq_L = 32; mq_W = 20;

    difference() {
        // Open-backed bracket shape (U-channel cross-section), unioned as one solid
        union() {
            cube([mount_L, mount_W, wall]); // base plate
            translate([0, 0, 0])
                cube([wall, mount_W, mount_H]); // left wall
            translate([mount_L - wall, 0, 0])
                cube([wall, mount_W, mount_H]); // right wall
        }
        // Clearance so sensor slides in from the top, snug fit
        translate([wall - 0.5, -1, wall])
            cube([mq_L + 1, mount_W + 2, mount_H]);
    }

    // Mounting post on the back for attaching to case wall (press-fit peg)
    // Overlaps into the base plate by 1mm so it's a solid union, not just touching
    translate([mount_L/2 - 2, mount_W - 1, 0])
        cylinder(h = wall + 8, d = 4);
}

// ============================================================
// VENT FRAME (fixed part, mounts to case, flap hinges to this)
// ============================================================
module vent_frame() {
    frame_L = 46;
    frame_W = 46;
    frame_thick = 3;

    difference() {
        cube([frame_L, frame_W, frame_thick]);
        translate([3, 3, -1])
            cube([frame_L - 6, frame_W - 6, frame_thick + 2]); // open center for airflow
    }

    // Two hinge loops along one edge
    for (x = [8, 34]) {
        translate([x, frame_W, 0])
            rotate([90, 0, 0])
                difference() {
                    cylinder(h = 6, d = 8);
                    cylinder(h = 6.2, d = 3.2); // hole for hinge pin (e.g. 3mm filament or wire rod)
                }
    }
}

// ============================================================
// VENT FLAP (moving part, hinges into vent_frame, driven by servo arm)
// ============================================================
module vent_flap() {
    flap_L = 40;
    flap_W = 40;
    flap_thick = 2;

    cube([flap_L, flap_W, flap_thick]);

    // Hinge loops on one edge, offset to interleave with frame's loops
    for (x = [2, 20, 38]) {
        translate([x, flap_W, 0])
            rotate([90, 0, 0])
                difference() {
                    cylinder(h = 5, d = 7.5);
                    cylinder(h = 5.2, d = 3.2);
                }
    }

    // Small tab on the opposite edge for the servo linkage arm to push against
    translate([flap_L/2 - 4, -8, 0])
        cube([8, 8, flap_thick + 3]);
}
