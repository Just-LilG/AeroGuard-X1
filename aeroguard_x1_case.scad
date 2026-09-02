// ============================================================
// AEROGUARD-X1 — COMPACT PRODUCT CASE (v2)
// Smaller, rounded shell with mounts for every module and
// engraved product branding on the lid + inside the base.
//
// Fits: Arduino Uno R3, LCD1602 I2C, MQ gas sensor, KY-026 flame,
//       SIM800L, LM2596 buck, microSD, LEDs, Demo + Reset buttons,
//       buzzer. Extra floor shelf is left empty for the demo kit.
//
// HOW TO USE:
// 1. Install OpenSCAD: https://openscad.org/downloads.html
// 2. Set part = "all_export" (default), press F6, export STL
// 3. Slice (PLA/PETG). Print base flat, lid flat, sensor mount.
// ============================================================

part = "all_export"; // "base", "lid", "sensor_mount", "all_preview", "all_export"

$fn = 64;
wall = 2.2;
fillet = 7;          // outer corner radius look
clear = 0.35;        // lid seating clearance

// Outer envelope — tighter than v1 (150×100×42)
case_L = 126;
case_W = 90;
case_H = 36;
lid_H  = 3.2;

// Module footprints (mm)
uno_L = 68.6;  uno_W = 53.4;
lcd_L = 72;    lcd_W = 25;     // visible window (bezel covers rest)
lcd_bezel_L = 80; lcd_bezel_W = 36;
sd_L = 28;     sd_W = 24;
sim_L = 25;    sim_W = 24;
buck_L = 43;   buck_W = 21;
esp_L = 51;    esp_W = 28;
led_d = 5.2;
button_d = 11;
buzzer_d = 12;

brand_font = "Inter:style=Bold";

// ---------- layout origins (inside floor, from outer 0,0) ----------
uno_x = 4.5;
uno_y = 4.5;
esp_x = 4.5;
esp_y = 60;            // strip under Uno
sim_x = 78;
sim_y = 5;
sd_x  = 78;
sd_y  = 33;
buck_x = 78;
buck_y = 61;

lcd_x = 8;
lcd_y = (case_W - lcd_W) / 2 - 2;
led_x = 96;
btn_reset_x = 98; btn_reset_y = 58;
btn_demo_x  = 114; btn_demo_y  = 58;
buzzer_x = 106; buzzer_y = 74;

if (part == "base") base();
if (part == "lid") lid();
if (part == "sensor_mount") sensor_mount();
if (part == "all_preview") {
    base();
    translate([0, 0, case_H + 6]) lid();
    translate([case_L + 18, 0, 0]) sensor_mount();
}
if (part == "all_export") {
    base();
    translate([case_L + 16, 0, 0]) lid();
    translate([0, case_W + 16, 0]) sensor_mount();
    translate([50, case_W + 16, 0]) sensor_mount(); // spare for multi-zone story
}

// ============================================================
// SHARED SHAPES
// ============================================================
module rounded_rect(l, w, h, r) {
    hull() {
        for (x = [r, l - r])
            for (y = [r, w - r])
                translate([x, y, 0])
                    cylinder(h = h, r = r);
    }
}

module shell_outer(h) {
    rounded_rect(case_L, case_W, h, fillet);
}

module shell_inner(h) {
    translate([wall, wall, 0])
        rounded_rect(case_L - 2*wall, case_W - 2*wall, h, max(1.5, fillet - wall));
}

module platform(l, w, h = 2.6, label = "") {
    // solid deck + corner pegs so modules sit flat
    rounded_rect(l, w, h, 1.8);
    for (p = [[2.2, 2.2], [l - 2.2, 2.2], [2.2, w - 2.2], [l - 2.2, w - 2.2]])
        translate([p[0], p[1], h])
            cylinder(h = 1.4, d = 2.2);
    if (label != "") {
        translate([l/2, w/2, h + 0.05])
            linear_extrude(0.45)
                text(label, size = 2.4, font = brand_font,
                     halign = "center", valign = "center");
    }
}

module standoff(h = 5.5, od = 5.6, id = 2.4) {
    difference() {
        cylinder(h = h, d = od);
        translate([0, 0, -0.1]) cylinder(h = h + 0.2, d = id);
    }
}

module grill_slots(n = 5, pitch = 5.5, slot_w = 2.2, slot_h = 12) {
    for (i = [0:n - 1])
        translate([0, i * pitch, 0])
            hull() {
                translate([0, slot_w/2, 0])
                    rotate([0, 90, 0]) cylinder(h = wall + 2, d = slot_w);
                translate([0, slot_w/2, slot_h])
                    rotate([0, 90, 0]) cylinder(h = wall + 2, d = slot_w);
            }
}

module brand_engrave(size = 5.5, depth = 0.7) {
    linear_extrude(depth)
        text("AeroGuard", size = size, font = brand_font,
             halign = "center", valign = "center");
}

module mark_engrave(size = 3.2, depth = 0.55) {
    linear_extrude(depth)
        text("X1", size = size, font = brand_font,
             halign = "center", valign = "center");
}

// ============================================================
// BASE
// ============================================================
module base() {
    difference() {
        union() {
            // outer shell
            difference() {
                shell_outer(case_H);
                translate([0, 0, wall])
                    shell_inner(case_H);
            }
            // lid seat ledge
            translate([0, 0, case_H - 2.2])
                difference() {
                    shell_outer(2.2);
                    translate([wall + clear, wall + clear, -0.1])
                        rounded_rect(
                            case_L - 2*(wall + clear),
                            case_W - 2*(wall + clear),
                            2.5,
                            max(1, fillet - wall - clear)
                        );
                }
        }

        // ---- cutouts ----
        // Uno USB
        translate([-1, uno_y + 16, wall + 3.5])
            cube([wall + 3, 13, 9]);
        // Uno DC jack
        translate([-1, uno_y + 38, wall + 4])
            rotate([0, 90, 0]) cylinder(h = wall + 3, d = 9);

        // Gas intake grill (left wall) — elegant oval slots
        translate([-1, 18, case_H - 16])
            grill_slots(n = 5, pitch = 6, slot_w = 2.4, slot_h = 11);

        // Flame viewing window (right)
        translate([case_L - wall - 1, 34, case_H - 15])
            hull() {
                translate([0, 0, 0]) rotate([0, 90, 0]) cylinder(h = wall + 2, d = 3);
                translate([0, 14, 0]) rotate([0, 90, 0]) cylinder(h = wall + 2, d = 3);
                translate([0, 0, 9]) rotate([0, 90, 0]) cylinder(h = wall + 2, d = 3);
                translate([0, 14, 9]) rotate([0, 90, 0]) cylinder(h = wall + 2, d = 3);
            }

        // SIM antenna exit (front)
        translate([sim_x + 4, -1, case_H - 9])
            cube([14, wall + 2, 4.5]);
        // Spare-shelf cable / access slot (front-left) — leave empty for demo
        translate([esp_x + 8, case_W - wall - 1, case_H - 10])
            cube([16, wall + 2, 5]);

        // Outside brand — recessed on front skirt
        translate([case_L/2, 0.55, 11])
            rotate([90, 0, 0])
                brand_engrave(size = 4.6, depth = 0.85);

        // Subtle X1 badge on right skirt
        translate([case_L - 0.55, case_W/2, 12])
            rotate([90, 0, 90])
                mark_engrave(size = 4.0, depth = 0.85);
    }

    // ---- floor mounts ----
    // Uno standoffs (official hole pattern)
    uno_holes = [[15.24, 2.54], [15.24, 50.8], [66.04, 17.78], [66.04, 45.72]];
    for (h = uno_holes)
        translate([uno_x + h[0], uno_y + h[1], wall])
            standoff();

    // Labeled platforms
    translate([esp_x, esp_y, wall])
        platform(esp_L + 2, esp_W + 1.5, 2.4, "SPARE");
    translate([sim_x, sim_y, wall])
        platform(sim_L + 3, sim_W + 3, 2.4, "SIM");
    translate([sd_x, sd_y, wall])
        platform(sd_L + 3, sd_W + 3, 2.4, "SD");
    translate([buck_x, buck_y, wall])
        platform(buck_L + 2, buck_W + 2, 2.4, "4V");

    // Corner lid screw bosses (M2 / self-tap)
    for (p = [[6, 6], [case_L - 6, 6], [6, case_W - 6], [case_L - 6, case_W - 6]])
        translate([p[0], p[1], wall])
            standoff(h = case_H - wall - 2.4, od = 7, id = 1.8);

    // Inside floor branding (visible when lid is open)
    translate([case_L/2, case_W/2 + 2, wall + 0.15])
        linear_extrude(0.55)
            text("AeroGuard-X1", size = 3.4, font = brand_font,
                 halign = "center", valign = "center");
    translate([case_L/2, case_W/2 - 4, wall + 0.15])
        linear_extrude(0.45)
            text("LPG safety", size = 2.4, font = "Inter",
                 halign = "center", valign = "center");
}

// ============================================================
// LID
// ============================================================
module lid() {
    difference() {
        union() {
            // main lid plate with soft rim
            shell_outer(lid_H);
            // inset lip that drops into base ledge
            translate([wall + clear + 0.15, wall + clear + 0.15, -2.0])
                rounded_rect(
                    case_L - 2*(wall + clear + 0.15),
                    case_W - 2*(wall + clear + 0.15),
                    2.0,
                    max(1, fillet - wall - clear)
                );
        }

        // hollow the inset lip so it is a thin ring
        translate([wall + clear + 1.3, wall + clear + 1.3, -2.2])
            rounded_rect(
                case_L - 2*(wall + clear + 1.3),
                case_W - 2*(wall + clear + 1.3),
                2.6,
                max(0.8, fillet - wall - clear - 1)
            );

        // LCD window
        translate([lcd_x, lcd_y, -3])
            rounded_rect(lcd_L, lcd_W, lid_H + 6, 1.2);

        // status LEDs (green / yellow / red)
        for (i = [0:2])
            translate([led_x, 16 + i * 11, -1])
                cylinder(h = lid_H + 4, d = led_d);

        // Demo + Reset
        translate([btn_reset_x, btn_reset_y, -1])
            cylinder(h = lid_H + 4, d = button_d);
        translate([btn_demo_x, btn_demo_y, -1])
            cylinder(h = lid_H + 4, d = button_d);

        // Buzzer
        translate([buzzer_x, buzzer_y, -1])
            cylinder(h = lid_H + 4, d = buzzer_d);
        // buzzer acoustic ring
        for (a = [0:60:300])
            translate([buzzer_x + cos(a)*4.2, buzzer_y + sin(a)*4.2, -1])
                cylinder(h = lid_H + 4, d = 1.4);

        // Outside top branding (recessed, elegant)
        translate([40, case_W - 10, lid_H - 0.65])
            brand_engrave(size = 5.2, depth = 0.75);
        translate([40, 9, lid_H - 0.55])
            linear_extrude(0.6)
                text("LPG · LEAK · FIRE", size = 2.6, font = "Inter",
                     halign = "center", valign = "center");

        // Button captions
        translate([btn_reset_x, btn_reset_y - 9, lid_H - 0.5])
            linear_extrude(0.55)
                text("RESET", size = 2.1, font = brand_font,
                     halign = "center", valign = "center");
        translate([btn_demo_x, btn_demo_y - 9, lid_H - 0.5])
            linear_extrude(0.55)
                text("DEMO", size = 2.1, font = brand_font,
                     halign = "center", valign = "center");

        // LED captions
        translate([led_x + 7, 16, lid_H - 0.45])
            linear_extrude(0.5)
                text("G", size = 2.0, font = brand_font, halign = "left");
        translate([led_x + 7, 27, lid_H - 0.45])
            linear_extrude(0.5)
                text("Y", size = 2.0, font = brand_font, halign = "left");
        translate([led_x + 7, 38, lid_H - 0.45])
            linear_extrude(0.5)
                text("R", size = 2.0, font = brand_font, halign = "left");

        // screw holes through lid into bosses
        for (p = [[6, 6], [case_L - 6, 6], [6, case_W - 6], [case_L - 6, case_W - 6]])
            translate([p[0], p[1], -3])
                cylinder(h = lid_H + 6, d = 2.4);
        // screw head countersinks
        for (p = [[6, 6], [case_L - 6, 6], [6, case_W - 6], [case_L - 6, case_W - 6]])
            translate([p[0], p[1], lid_H - 1.1])
                cylinder(h = 1.3, d1 = 2.4, d2 = 4.2);
    }

    // LCD underside retainer lip
    translate([lcd_x - 2, lcd_y - 2, -1.8])
        difference() {
            rounded_rect(lcd_L + 4, lcd_W + 4, 1.8, 1.4);
            translate([1.5, 1.5, -0.2])
                rounded_rect(lcd_L + 1, lcd_W + 1, 2.4, 1);
        }

    // Inside lid branding (reads when you open the box)
    translate([case_L/2, case_W/2, -1.95])
        linear_extrude(0.5)
            text("AeroGuard", size = 4.0, font = brand_font,
                 halign = "center", valign = "center");
}

// ============================================================
// GAS SENSOR CLIP MOUNT (print 1–2)
// ============================================================
module sensor_mount() {
    mount_L = 34;
    mount_W = 24;
    mount_H = 14;
    mq_L = 31.5;

    difference() {
        union() {
            rounded_rect(mount_L, mount_W, wall, 2);
            rounded_rect(wall + 0.6, mount_W, mount_H, 0.6);
            translate([mount_L - wall - 0.6, 0, 0])
                rounded_rect(wall + 0.6, mount_W, mount_H, 0.6);
            // engraved side label
            translate([mount_L/2, 1.0, wall])
                linear_extrude(0.4)
                    text("MQ", size = 2.4, font = brand_font,
                         halign = "center", valign = "bottom");
        }
        translate([wall - 0.4, -1, wall])
            cube([mq_L + 0.8, mount_W + 2, mount_H]);
    }
    // peg into case airflow wall
    translate([mount_L/2, mount_W - 0.5, 0])
        cylinder(h = wall + 7, d = 3.6);
}
