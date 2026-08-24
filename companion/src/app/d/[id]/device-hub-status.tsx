"use client";

import { useMemo, useState } from "react";
import Link from "next/link";
import { useRouter } from "next/navigation";
import {
  ArrowLeft,
  Check,
  Flame,
  Phone,
  Plus,
  Radio,
  Settings2,
  ShieldAlert,
  Trash2,
  Wind,
  AppWindow,
} from "lucide-react";
import { BrandMark } from "@/components/brand-mark";
import { ThemeToggle } from "@/components/theme-toggle";
import { StatusRing } from "@/components/status-ring";
import { PhoneShell } from "@/components/phone-shell";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { useDevices } from "@/lib/store";
import {
  PLACES,
  STAGE_META,
  type AeroDevice,
  type DeviceTab,
  type PlaceKind,
  type SmartVent,
  type VentKind,
} from "@/lib/types";

const TABS: { id: DeviceTab; label: string }[] = [
  { id: "status", label: "Status" },
  { id: "activity", label: "Activity" },
  { id: "vents", label: "Vents" },
  { id: "device", label: "Device" },
];

const STAGE_HINT: Record<string, string> = {
  SAFE: "Sensors quiet. Green path idle — monitoring only.",
  LOW: "Slight rise above baseline. Green LED on the box.",
  MEDIUM: "Owner SMS + vent open intent. Yellow LED.",
  CRITICAL: "Owner call + SMS. Red LED. Clear the kitchen.",
  FIRE: "Flame path active. Evacuate and call for help.",
};

export function StatusTab({
  device,
  onDemo,
  onReset,
}: {
  device: AeroDevice;
  onDemo: () => void;
  onReset: () => void;
}) {
  const meta = STAGE_META[device.stage];

  const demoNext =
    device.stage === "SAFE"
      ? "Low"
      : device.stage === "LOW"
        ? "Medium"
        : device.stage === "MEDIUM"
          ? "Critical"
          : device.stage === "CRITICAL"
            ? "Fire"
            : "Low";

  return (
    <div className="fade-up space-y-4">
      <div className="glass rounded-[1.7rem] p-5">
        <div className="mb-3 flex items-center justify-between">
          <div>
            <p className="text-[11px] font-semibold uppercase tracking-[0.14em] text-muted-foreground">
              Live status
            </p>
            <p className="mt-1 font-[family-name:var(--font-display)] text-[22px] font-bold tracking-tight">
              {meta.label}
            </p>
          </div>
          <span className="rounded-full bg-emerald-500/15 px-2.5 py-1 text-[11px] font-semibold text-emerald-700 dark:text-emerald-300">
            Online · sim
          </span>
        </div>
        <StatusRing
          stage={device.stage}
          onClick={onDemo}
          subtitle={`LED ${meta.led} · tap to demo`}
        />
        <p className="mt-4 text-center text-[13px] leading-relaxed text-muted-foreground">
          {STAGE_HINT[device.stage]}
        </p>
      </div>

      <div className="grid grid-cols-2 gap-3">
        <Metric
          label="Gas reading"
          value={String(device.gasReading)}
          hint={`Baseline ${device.baseline}`}
        />
        <Metric
          label="Alert path"
          value={meta.tone}
          hint={device.demoMode ? "Demo cycle" : "Live path"}
        />
        <Metric
          label="Owner"
          value={device.stage === "SAFE" ? "Standby" : "Armed"}
          hint={device.ownerPhone || "No number"}
        />
        <Metric
          label="Backup SMS"
          value="After 3 min"
          hint={device.backupPhone || "No number"}
        />
      </div>

      <div className="glass rounded-[1.5rem] p-4">
        <div className="flex items-start gap-3">
          <div className="flex h-10 w-10 items-center justify-center rounded-2xl bg-primary/15 text-primary">
            <ShieldAlert className="h-5 w-5" />
          </div>
          <div>
            <p className="text-[15px] font-semibold">What the box does</p>
            <p className="mt-1 text-[13px] leading-relaxed text-muted-foreground">
              {meta.led === "Off"
                ? "No LED stage lit. Sensors keep sampling."
                : `${meta.led} LED on hardware. ${meta.tone}.`}{" "}
              GSM SMS/call use the phones saved on Device.
            </p>
          </div>
        </div>
      </div>

      <div className="glass rounded-[1.5rem] p-4">
        <p className="text-[11px] font-semibold uppercase tracking-[0.12em] text-muted-foreground">
          Contest demo
        </p>
        <p className="mt-2 text-[14px] leading-relaxed text-muted-foreground">
          Tap the ring or Next to cycle Low → Medium → Critical → Fire — same idea
          as the Demo button on the box. GSM still targets this unit&apos;s contacts.
        </p>
        <div className="mt-4 grid grid-cols-2 gap-2">
          <Button type="button" className="h-12 rounded-2xl" onClick={onDemo}>
            Next: {demoNext}
          </Button>
          <Button
            type="button"
            variant="secondary"
            className="h-12 rounded-2xl"
            onClick={onReset}
          >
            Reset Safe
          </Button>
        </div>
      </div>
    </div>
  );
}

export function Metric({
  label,
  value,
  hint,
}: {
  label: string;
  value: string;
  hint: string;
}) {
  return (
    <div className="glass rounded-[1.35rem] p-3.5">
      <p className="text-[10px] font-medium uppercase tracking-wide text-muted-foreground">
        {label}
      </p>
      <p className="mt-1 truncate font-[family-name:var(--font-display)] text-[15px] font-semibold">
        {value}
      </p>
      <p className="mt-0.5 truncate text-[11px] text-muted-foreground">{hint}</p>
    </div>
  );
}
