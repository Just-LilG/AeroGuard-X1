# AeroGuard-X1

A small box that watches for **LPG leaks and fire**, then **calls and texts** the owner.

**Start here:** [`AeroGuard-X1_Build_Guide.md`](AeroGuard-X1_Build_Guide.md)  
Pin-for-pin wiring for parts on the table: [`AeroGuard-X1_Pin_Map_Wiring_Reference.md`](AeroGuard-X1_Pin_Map_Wiring_Reference.md)  
Visual walkthrough: [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) (open in a browser)

## What you assemble

**On the table today:** Arduino Uno (the brain) + MQ gas nose + flame eye + LCD + lights + buzzer + buttons + **SIM800L** phone chip + 4V buck + case.

**Still to buy:** **micro SD** module only. Leave D10–D13 empty.

Upload **only** [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) to the Uno. USB **data** cable. Serial Monitor **9600**. `GSM_ENABLED` is **true**. Keep `SD_ENABLED` **false** until the SD board arrives. Put owner numbers in the file before a live call.

## Box and app

The **Vercel contest app** is a screen for judges. Pairing and status are simulated. It does not talk to the hardware.

**Real alerts** are GSM SMS and calls from the SIM800L in the box. There is **no SD log** until that module is bought.

| Stage | LED | Box |
|-------|-----|-----|
| LOW | Green | Quiet early warning |
| MEDIUM | Yellow | Buzzer + SMS |
| CRITICAL | Red | Alarm + **call + SMS** |
| FIRE | Red | Alarm + **call + SMS** (fire-service SMS = later) |

**Demo** pretends a leak (LOW → MEDIUM → CRITICAL → FIRE). **Reset** mutes, exits demo, recalibrates. No motorized vent on the box — vents are an app story.

## Files

| Path | Role |
|------|------|
| [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) | Program for the Uno |
| [`AeroGuard-X1_Pin_Map_Wiring_Reference.md`](AeroGuard-X1_Pin_Map_Wiring_Reference.md) | Pin-for-pin wiring for today’s parts |
| [`aeroguard_x1_case_base.stl`](aeroguard_x1_case_base.stl) · [`aeroguard_x1_case_lid.stl`](aeroguard_x1_case_lid.stl) · [`aeroguard_x1_sensor_mount.stl`](aeroguard_x1_sensor_mount.stl) · [`aeroguard_x1_button_cap.stl`](aeroguard_x1_button_cap.stl) | Ready-to-print case (126×90×36 mm). Print the button cap **twice**. |
| [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad) | Editable source for the case (change sizes, re-export STL) |
| [`companion/`](companion/) | Contest phone UI |

```bash
cd companion
npm install
npm run dev -- --hostname 127.0.0.1 --port 43123
```
