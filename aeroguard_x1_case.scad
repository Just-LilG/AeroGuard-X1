// ============================================================
// AEROGUARD-X1 — PRODUCT CASE (v1)
// LPG leak + fire early-warning device
//
// Parts: base tray, lid, gas sensor mount (x1; print x2 later
// for expansion). No on-device servo vent — smart vents/windows
// are controlled by the AeroGuard app.
//
// HOW TO USE:
// 1. Install OpenSCAD: https://openscad.org/downloads.html
// 2. Set part = "all_export" (default), press F6, export STL
// 3. Slice and print
// ============================================================

part = "all_export"; // "base", "lid", "sensor_mount", "all_preview", "all_export"

wall = 2;
$fn = 48;

case_L = 150;
case_W = 100;
case_H = 42;

uno_L = 68.6; uno_W = 53.4;
lcd_L = 80;   lcd_W = 36;
sd_L = 28;    sd_W = 24;
sim_L = 25;   sim_W = 24;
ble_L = 52;   ble_W = 28;   // ESP32 DevKit footprint (approx)
led_d = 5;
button_d = 12;
buzzer_d = 12;

if (part == "base") base_tray();
if (part == "lid") lid();
if (part == "sensor_mount") sensor_mount();
if (part == "all_preview") {
    base_tray();
    translate([0, 0, case_H + 8]) lid();
    translate([case_L + 20, 0, 0]) sensor_mount();
}
if (part == "all_export") {
    translate([0, 0, 0]) base_tray();
    translate([case_L + 20, 0, 0]) lid();
    translate([0, case_W + 20, 0]) sensor_mount();
    // optional second mount for future multi-zone expansion
    translate([70, case_W + 20, 0]) sensor_mount();
}

module base_tray() {
    difference() {
        cube([case_L, case_W, case_H]);
        translate([wall, wall, wall])
            cube([case_L - 2*wall, case_W - 2*wall, case_H]);

        // USB
        translate([-1, 18, wall + 4])
            cube([wall + 2, 12, 8]);
        // Barrel jack
        translate([-1, 40, wall + 5])
            rotate([0, 90, 0]) cylinder(h = wall + 2, d = 8);

        // Gas sensor airflow slots (left)
        for (i = [0:3]) {
            translate([-1, 14 + i*8, case_H - 14])
                cube([wall + 2, 3, 18]);
        }
        // Flame window (right)
        translate([case_L - wall - 1, 40, case_H - 14])
            cube([wall + 2, 12, 10]);
        // SIM antenna notch (front)
        translate([60, -1, case_H - 10])
            cube([16, wall + 2, 5]);
        // WiFi antenna notch (front) — ESP32
        translate([100, -1, case_H - 10])
            cube([18, wall + 2, 5]);
    }

    // Uno standoffs
    uno_holes = [[15.24, 2.54], [15.24, 50.8], [66.04, 17.78], [66.04, 45.72]];
    uno_origin = [8, 8];
    for (h = uno_holes) {
        translate([uno_origin[0] + h[0], uno_origin[1] + h[1], wall])
            difference() {
                cylinder(h = 6, d = 6);
                cylinder(h = 6.2, d = 2.5);
            }
    }

    // SD platform
    translate([105, 8, wall]) cube([sd_L + 4, sd_W + 4, 3]);
    // SIM800L platform
    translate([105, 40, wall]) cube([sim_L + 4, sim_W + 4, 3]);
    // ESP32 WiFi bridge platform
    translate([8, 68, wall]) cube([ble_L + 4, ble_W + 4, 3]);
}

module lid() {
    difference() {
        cube([case_L, case_W, wall]);

        // LCD window
        translate([12, (case_W - lcd_W)/2, -1])
            cube([lcd_L, lcd_W, wall + 2]);

        // LEDs: Green, Yellow, Red
        for (i = [0:2]) {
            translate([110, 22 + i*12, -1])
                cylinder(h = wall + 2, d = led_d);
        }

        // Reset button
        translate([110, 62, -1])
            cylinder(h = wall + 2, d = button_d);
        // Demo button
        translate([130, 62, -1])
            cylinder(h = wall + 2, d = button_d);

        // Buzzer
        translate([120, 85, -1])
            cylinder(h = wall + 2, d = buzzer_d);
    }

    // LCD lip underside
    translate([10, (case_W - lcd_W)/2 - 2, -2])
        difference() {
            cube([lcd_L + 4, lcd_W + 4, 2]);
            translate([2, 2, -1]) cube([lcd_L, lcd_W, 4]);
        }
}

module sensor_mount() {
    mount_L = 35;
    mount_W = 25;
    mount_H = 15;
    mq_L = 32;

    difference() {
        union() {
            cube([mount_L, mount_W, wall]);
            cube([wall, mount_W, mount_H]);
            translate([mount_L - wall, 0, 0])
                cube([wall, mount_W, mount_H]);
        }
        translate([wall - 0.5, -1, wall])
            cube([mq_L + 1, mount_W + 2, mount_H]);
    }
    translate([mount_L/2 - 2, mount_W - 1, 0])
        cylinder(h = wall + 8, d = 4);
}
