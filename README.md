# AeroGuard-X1

A small box that watches for **LPG leaks and fire**, then **calls and texts** the owner.

**Start here:** [`AeroGuard-X1_Build_Guide.md`](AeroGuard-X1_Build_Guide.md)  
Visual walkthrough: [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) (open in a browser)

## What you assemble

Arduino Uno (the brain) + MQ gas nose + flame eye + LCD screen + green/yellow/red lights + buzzer + Demo/Reset buttons + SD log + **SIM800L** (the phone chip) + 4V buck + printed case.

Upload **only** [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) to the Uno. USB **data** cable. Serial Monitor **9600**. Put owner numbers in that file before a live demo.

## Box and app

The **Vercel contest app** is a screen for judges. Pairing and status are simulated. It does not talk to the hardware.

**Real alerts** are GSM SMS and calls from the SIM in the box.

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
| [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad) | Case to print (126×90×36 mm) |
| [`companion/`](companion/) | Contest phone UI |

```bash
cd companion
npm install
npm run dev -- --hostname 127.0.0.1 --port 43123
```
