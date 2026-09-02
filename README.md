# AeroGuard-X1

A small box that watches for **LPG leaks and fire**, then **calls and texts** the owner.

**Start here:** [`AeroGuard-X1_Build_Guide.md`](AeroGuard-X1_Build_Guide.md)  
Pin-for-pin wiring for parts on the table: [`AeroGuard-X1_Pin_Map_Wiring_Reference.md`](AeroGuard-X1_Pin_Map_Wiring_Reference.md)  
Visual walkthrough: [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) (open in a browser)

## What you assemble

**On the table today:** Arduino Uno (the brain) + MQ gas nose + flame eye + LCD screen + green/yellow/red lights + buzzer + Demo/Reset buttons + breadboard + case.

**Still to buy:** **SIM800L** phone chip, **LM2596** 4V buck, **micro SD** module. Do not wire D5, D6, or D10–D13 until those arrive.

Upload **only** [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) to the Uno. USB **data** cable. Serial Monitor **9600**. Keep `GSM_ENABLED` as **false** until the phone chip is wired.

## Box and app

The **Vercel contest app** is a screen for judges. Pairing and status are simulated. It does not talk to the hardware.

**Real alerts** (when the SIM800L is fitted) are GSM SMS and calls from the box. **Today there is no phone chip**, so Demo only drives lights, screen, and beeper.

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
| [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad) | Case to print (126×90×36 mm) |
| [`companion/`](companion/) | Contest phone UI |

```bash
cd companion
npm install
npm run dev -- --hostname 127.0.0.1 --port 43123
```
