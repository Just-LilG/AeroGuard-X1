// ============================================================
// AEROGUARD-X1 — COMPACT PRODUCT CASE (v3)
// Redesigned for the current demo build (Uno + SIM800L, SD to
// come later) and for TWO 6x6mm TACTILE SWITCHES on the lid.
//
// v3 goals:
//   * 6x6mm tactile switches held in the LID, with printable caps
//   * Friendlier to print: thicker walls, no sub-2mm holes,
//     bigger/fewer engravings, chunkier screw towers with gussets
//   * Ships as ready-to-slice STL files (see EXPORT below)
//
// Fits: Arduino Uno R3, LCD1602 I2C, MQ gas sensor, KY-026 flame,
//       SIM800L + antenna, LM2596 4V buck, status LEDs, buzzer,
//       Demo + Reset 6x6 tactile switches. microSD shelf is kept
//       empty and ready for when that board is bought.
//
// HOW TO USE / EXPORT:
//   Render each part on its own and export STL, e.g. from a shell:
//     openscad -D 'part="base"'        -o aeroguard_x1_case_base.stl   aeroguard_x1_case.scad
//     openscad -D 'part="lid"'         -o aeroguard_x1_case_lid.stl    aeroguard_x1_case.scad
//     openscad -D 'part="sensor_mount"'-o aeroguard_x1_sensor_mount.stl aeroguard_x1_case.scad
//     openscad -D 'part="button_cap"'  -o aeroguard_x1_button_cap.stl  aeroguard_x1_case.scad
//   Or open in OpenSCAD, pick a part below, press F6, export STL.
//   Slice: PLA/PETG, 0.2mm layers, ~20% infill. All parts print flat.
//   Print TWO button caps.
// ============================================================

part = "all_preview"; // base | lid | sensor_mount | button_cap | all_preview | all_export

$fn = 56;
wall = 2.4;          // thicker shell = fewer thin-wall print fails
fillet = 7;          // outer corner radius look
clear = 0.4;         // lid seating clearance

// Outer envelope
case_L = 126;
case_W = 90;
case_H = 36;
lid_H  = 3.2;
mrg    = 0.8;         // how far add-on features sink into their parent so
                     // booleans FUSE into one solid (no loose shells that
                     // make CAD tools like Onshape report import faults)

// ---- 6x6mm tactile switch (Demo / Reset) ----
sw_body      = 6.0;          // switch body is 6.0 x 6.0 mm
sw_pocket    = sw_body + 0.4;// square pocket with a little clearance
sw_pocket_h  = 3.8;          // how deep the body drops into the lid collar
sw_collar_w  = 2.0;          // wall around the pocket
plunger_hole = 4.6;          // clearance for the plunger + the cap stem
// button cap (own print, x2)
cap_top_d      = 9.0;
cap_top_h      = 2.6;
cap_stem_d     = 4.2;
cap_cbore_d    = 9.6;        // recess in the lid so the cap sits flush-ish
cap_cbore_h    = 1.4;

// Module footprints (mm)
uno_L = 68.6;  uno_W = 53.4;
lcd_L = 72;    lcd_W = 25;     // visible window
sd_L = 28;     sd_W = 24;
sim_L = 25;    sim_W = 24;
buck_L = 43;   buck_W = 21;
led_d = 5.2;
buzzer_d = 12;

brand_font = "Inter:style=Bold";
plain_font = "Inter";

// ---------- base-floor layout origins (from outer 0,0) ----------
uno_x = 4.5;   uno_y = 4.5;
sim_x = 78;    sim_y = 5;
sd_x  = 78;    sd_y  = 33;
buck_x = 78;   buck_y = 61;

// ---------- lid (top panel) layout ----------
lcd_x = 8;
lcd_y = (case_W - lcd_W) / 2 - 2;
led_x = 90;    led_ys = [14, 24, 34];
buzzer_x = 110; buzzer_y = 22;
sw_reset = [95, 50];    // Reset switch centre
sw_demo  = [113, 50];   // Demo switch centre

screw_pts = [[6, 6], [case_L - 6, 6], [6, case_W - 6], [case_L - 6, case_W - 6]];

// ============================================================
// PART SELECTOR
// ============================================================
if (part == "base") base();
if (part == "lid") lid();
if (part == "sensor_mount") sensor_mount();
if (part == "button_cap") button_cap();
if (part == "all_preview") {
    base();
    translate([0, 0, case_H + 8]) lid();
    translate([case_L + 18, 0, 0]) sensor_mount();
    translate([case_L + 18, 34, 0]) button_cap();
    translate([case_L + 34, 34, 0]) button_cap();
}
if (part == "all_export") {
    base();
    translate([case_L + 16, 0, 0]) lid();
    translate([0, case_W + 16, 0]) sensor_mount();
    translate([46, case_W + 16, 0]) button_cap();
    translate([66, case_W + 16, 0]) button_cap();
}
// Everything on ONE bed, ready to print in a single job (no supports).
// Base sits flat; the lid and caps are flipped top-down so they print
// clean. Footprint ~174 x 190 mm -> needs a ~220x220 bed (e.g. Ender 3).
if (part == "print_plate") {
    base();                                                   // bottom on the bed
    translate([0, 2*case_W + 10, lid_H]) rotate([180, 0, 0]) lid();  // lid, top-down
    translate([case_L + 14, 4, 0]) sensor_mount();            // gas clip
    translate([case_L + 24, 44, cap_top_h]) rotate([180, 0, 0]) button_cap();
    translate([case_L + 38, 44, cap_top_h]) rotate([180, 0, 0]) button_cap();
}

// ============================================================
// SHARED SHAPES
// ============================================================
module rounded_rect(l, w, h, r) {
    hull() for (x = [r, l - r]) for (y = [r, w - r])
        translate([x, y, 0]) cylinder(h = h, r = r);
}

module shell_outer(h) { rounded_rect(case_L, case_W, h, fillet); }

module shell_inner(h) {
    translate([wall, wall, 0])
        rounded_rect(case_L - 2*wall, case_W - 2*wall, h, max(1.5, fillet - wall));
}

// board rest deck with locating pegs (pegs enlarged for reliable printing)
module platform(l, w, h = 2.6, label = "") {
    rounded_rect(l, w, h, 1.8);
    // locating pegs dip into the deck so they fuse (no loose shell)
    for (p = [[3, 3], [l - 3, 3], [3, w - 3], [l - 3, w - 3]])
        translate([p[0], p[1], h - 0.6]) cylinder(h = 1.8, d = 3.0);
    // label sits proud but roots into the deck
    if (label != "")
        translate([l/2, w/2, h - 0.3])
            linear_extrude(0.8)
                text(label, size = 3.2, font = brand_font,
                     halign = "center", valign = "center");
}

// screw tower with a cone gusset at its foot so tall towers print solid
module boss(h, od = 8, id = 2.0) {
    difference() {
        union() {
            cylinder(h = h, d = od);
            cylinder(h = min(h, 4.5), d1 = od + 4, d2 = od); // gusset foot
        }
        translate([0, 0, -0.1]) cylinder(h = h + 0.2, d = id);
    }
}

module standoff(h = 5.5, od = 5.8, id = 2.4) {
    difference() {
        cylinder(h = h, d = od);
        translate([0, 0, -0.1]) cylinder(h = h + 0.2, d = id);
    }
}

module grill_slots(n = 5, pitch = 6, slot_w = 2.4, slot_h = 11) {
    for (i = [0:n - 1])
        translate([0, i * pitch, 0])
            hull() {
                translate([0, slot_w/2, 0]) rotate([0, 90, 0]) cylinder(h = wall + 2, d = slot_w);
                translate([0, slot_w/2, slot_h]) rotate([0, 90, 0]) cylinder(h = wall + 2, d = slot_w);
            }
}

module brand_engrave(size = 5.2, depth = 0.8) {
    linear_extrude(depth)
        text("AeroGuard", size = size, font = brand_font,
             halign = "center", valign = "center");
}

// ---- switch collar (added under the lid) ----
module switch_collar_pos() {
    o = sw_pocket + 2*sw_collar_w;
    // extend up into the plate (0..mrg) so the collar fuses with it
    translate([-o/2, -o/2, -sw_pocket_h])
        rounded_rect(o, o, sw_pocket_h + mrg, 1.2);
}
// ---- switch voids (pocket + plunger hole) cut from the lid ----
module switch_collar_neg() {
    // square body pocket, open downward, capped by the plate
    translate([-sw_pocket/2, -sw_pocket/2, -sw_pocket_h - 0.01])
        cube([sw_pocket, sw_pocket, sw_pocket_h + 0.01]);
    // plunger + cap-stem clearance through the plate
    translate([0, 0, -sw_pocket_h - 1])
        cylinder(h = sw_pocket_h + lid_H + 2, d = plunger_hole);
    // top recess for the cap
    translate([0, 0, lid_H - cap_cbore_h])
        cylinder(h = cap_cbore_h + 1, d = cap_cbore_d);
}

// ============================================================
// BASE
// ============================================================
module base() {
    difference() {
        union() {
            // outer shell (hollow)
            difference() {
                shell_outer(case_H);
                translate([0, 0, wall]) shell_inner(case_H);
            }
            // lid seat ledge
            translate([0, 0, case_H - 2.4])
                difference() {
                    shell_outer(2.4);
                    translate([wall + clear, wall + clear, -0.1])
                        rounded_rect(case_L - 2*(wall + clear),
                                     case_W - 2*(wall + clear), 2.7,
                                     max(1, fillet - wall - clear));
                }
        }

        // ---- wall cutouts ----
        // Uno USB
        translate([-1, uno_y + 16, wall + 3.5]) cube([wall + 3, 13, 9]);
        // Uno DC jack
        translate([-1, uno_y + 38, wall + 4]) rotate([0, 90, 0]) cylinder(h = wall + 3, d = 9);
        // Gas intake grill (left wall)
        translate([-1, 18, case_H - 16]) grill_slots(n = 5, pitch = 6, slot_w = 2.6, slot_h = 11);
        // Flame viewing window (right wall)
        translate([case_L - wall - 1, 34, case_H - 15])
            hull() {
                translate([0, 0, 0]) rotate([0, 90, 0]) cylinder(h = wall + 2, d = 3.2);
                translate([0, 14, 0]) rotate([0, 90, 0]) cylinder(h = wall + 2, d = 3.2);
                translate([0, 0, 9]) rotate([0, 90, 0]) cylinder(h = wall + 2, d = 3.2);
                translate([0, 14, 9]) rotate([0, 90, 0]) cylinder(h = wall + 2, d = 3.2);
            }
        // SIM antenna exit (front wall)
        translate([sim_x + 4, -1, case_H - 9]) cube([14, wall + 2, 4.5]);
        // Cable access slot (front-left)
        translate([uno_x + 8, case_W - wall - 1, case_H - 10]) cube([16, wall + 2, 5]);

        // Outside brand on the front skirt (recessed)
        translate([case_L/2, 0.6, 12]) rotate([90, 0, 0]) brand_engrave(size = 5.0, depth = 0.9);
        // X1 badge on the right skirt
        translate([case_L - 0.6, case_W/2, 13]) rotate([90, 0, 90])
            linear_extrude(0.9) text("X1", size = 4.4, font = brand_font,
                                     halign = "center", valign = "center");
    }

    // ---- floor mounts ----
    // Every mount starts BELOW the floor top (wall - mrg) and is made that
    // much taller, so it overlaps the floor and fuses into one solid while
    // its top face stays at the same height boards rest on.
    // Uno standoffs (official hole pattern)
    uno_holes = [[15.24, 2.54], [15.24, 50.8], [66.04, 17.78], [66.04, 45.72]];
    for (h = uno_holes) translate([uno_x + h[0], uno_y + h[1], wall - mrg]) standoff(h = 5.5 + mrg);

    // module decks
    translate([sim_x, sim_y, wall - mrg]) platform(sim_L + 3, sim_W + 3, 2.4 + mrg, "SIM");
    translate([sd_x, sd_y, wall - mrg])   platform(sd_L + 3,  sd_W + 3,  2.4 + mrg, "SD");
    translate([buck_x, buck_y, wall - mrg]) platform(buck_L + 2, buck_W + 2, 2.4 + mrg, "4V");

    // corner lid-screw towers (chunky, gusseted)
    for (p = screw_pts) translate([p[0], p[1], wall - mrg]) boss(case_H - wall - 2.4 + mrg, od = 8, id = 2.0);

    // inside-floor branding (roots into the floor so it fuses)
    translate([case_L/2, case_W/2 + 2, wall - 0.3])
        linear_extrude(0.6 + 0.3) text("AeroGuard-X1", size = 4.0, font = brand_font,
                                 halign = "center", valign = "center");
    translate([case_L/2, case_W/2 - 5, wall - 0.3])
        linear_extrude(0.5 + 0.3) text("LPG safety", size = 3.0, font = plain_font,
                                 halign = "center", valign = "center");
}

// ============================================================
// LID
// ============================================================
module lid() {
    difference() {
        union() {
            // main plate + soft rim
            shell_outer(lid_H);
            // locating lip that drops into the base ledge (kept chunky).
            // Extends up into the plate (+mrg) so it fuses instead of just touching.
            translate([wall + clear + 0.15, wall + clear + 0.15, -1.8])
                difference() {
                    rounded_rect(case_L - 2*(wall + clear + 0.15),
                                 case_W - 2*(wall + clear + 0.15), 1.8 + mrg,
                                 max(1, fillet - wall - clear));
                    translate([1.8, 1.8, -0.2])
                        rounded_rect(case_L - 2*(wall + clear + 1.95),
                                     case_W - 2*(wall + clear + 1.95), 2.2 + mrg,
                                     max(0.8, fillet - wall - clear - 1.6));
                }
            // switch collars under the plate
            translate([sw_reset[0], sw_reset[1], 0]) switch_collar_pos();
            translate([sw_demo[0],  sw_demo[1],  0]) switch_collar_pos();
        }

        // LCD window
        translate([lcd_x, lcd_y, -3]) rounded_rect(lcd_L, lcd_W, lid_H + 6, 1.2);

        // status LEDs (green / yellow / red)
        for (yy = led_ys) translate([led_x, yy, -1]) cylinder(h = lid_H + 4, d = led_d);

        // 6x6 switch voids
        translate([sw_reset[0], sw_reset[1], 0]) switch_collar_neg();
        translate([sw_demo[0],  sw_demo[1],  0]) switch_collar_neg();

        // buzzer opening: one centre hole + a ring of >=2.6mm holes (print-safe)
        translate([buzzer_x, buzzer_y, -1]) cylinder(h = lid_H + 4, d = 3.0);
        for (a = [0:60:300])
            translate([buzzer_x + cos(a)*4.4, buzzer_y + sin(a)*4.4, -1])
                cylinder(h = lid_H + 4, d = 2.6);

        // top branding
        translate([40, case_W - 11, lid_H - 0.7]) brand_engrave(size = 5.4, depth = 0.8);
        translate([40, 9, lid_H - 0.6])
            linear_extrude(0.6) text("LPG - LEAK - FIRE", size = 3.2, font = plain_font,
                                     halign = "center", valign = "center");

        // switch captions
        translate([sw_reset[0], sw_reset[1] - 8.5, lid_H - 0.6])
            linear_extrude(0.6) text("RESET", size = 3.0, font = brand_font,
                                     halign = "center", valign = "center");
        translate([sw_demo[0], sw_demo[1] - 8.5, lid_H - 0.6])
            linear_extrude(0.6) text("DEMO", size = 3.0, font = brand_font,
                                     halign = "center", valign = "center");

        // LED captions
        translate([led_x + 6.5, led_ys[0], lid_H - 0.5]) linear_extrude(0.55) text("G", size = 3.0, font = brand_font, halign = "left", valign = "center");
        translate([led_x + 6.5, led_ys[1], lid_H - 0.5]) linear_extrude(0.55) text("Y", size = 3.0, font = brand_font, halign = "left", valign = "center");
        translate([led_x + 6.5, led_ys[2], lid_H - 0.5]) linear_extrude(0.55) text("R", size = 3.0, font = brand_font, halign = "left", valign = "center");

        // screw holes + countersinks
        for (p = screw_pts) translate([p[0], p[1], -3]) cylinder(h = lid_H + 6, d = 2.6);
        for (p = screw_pts) translate([p[0], p[1], lid_H - 1.2]) cylinder(h = 1.4, d1 = 2.6, d2 = 4.6);
    }

    // LCD underside retainer lip (extends up into the plate so it fuses)
    translate([lcd_x - 2, lcd_y - 2, -1.8])
        difference() {
            rounded_rect(lcd_L + 4, lcd_W + 4, 1.8 + mrg, 1.4);
            translate([1.6, 1.6, -0.2]) rounded_rect(lcd_L + 1, lcd_W + 1, 2.4 + mrg, 1);
        }

    // inside lid branding — placed on SOLID plate below the LCD window
    // (not over the window hole) and rooted in, so every letter fuses.
    translate([case_L/2, 13, -0.8])
        linear_extrude(0.8 + mrg) text("AeroGuard", size = 4.0, font = brand_font,
                                  halign = "center", valign = "center");
}

// ============================================================
// PRINTABLE BUTTON CAP  (print x2)
// ============================================================
module button_cap() {
    stem_len = lid_H + 0.8;   // reaches down to the switch plunger
    union() {
        // top disc with a gentle chamfer for a nicer press
        cylinder(h = cap_top_h - 0.8, d = cap_top_d);
        translate([0, 0, cap_top_h - 0.8]) cylinder(h = 0.8, d1 = cap_top_d, d2 = cap_top_d - 1.6);
        // stem (overlaps up into the disc so the two fuse into one solid)
        translate([0, 0, -stem_len]) cylinder(h = stem_len + mrg, d = cap_stem_d);
    }
}

// ============================================================
// GAS SENSOR CLIP MOUNT (print 1)
// ============================================================
module sensor_mount() {
    mount_L = 34; mount_W = 24; mount_H = 14; mq_L = 31.5;
    difference() {
        union() {
            rounded_rect(mount_L, mount_W, wall, 2);
            rounded_rect(wall + 0.8, mount_W, mount_H, 0.6);
            translate([mount_L - wall - 0.8, 0, 0]) rounded_rect(wall + 0.8, mount_W, mount_H, 0.6);
        }
        translate([wall - 0.4, -1, wall]) cube([mq_L + 0.8, mount_W + 2, mount_H]);
    }
    // peg into the airflow wall
    translate([mount_L/2, mount_W - 0.6, 0]) cylinder(h = wall + 7, d = 4.0);
}
