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

export function SetupFlow({
  device,
  onDone,
}: {
  device: AeroDevice;
  onDone: (data: {
    name: string;
    place: PlaceKind;
    ownerPhone: string;
    backupPhone: string;
  }) => void;
}) {
  const [name, setName] = useState("");
  const [place, setPlace] = useState<PlaceKind>("Home kitchen");
  const [owner, setOwner] = useState("");
  const [backup, setBackup] = useState("");

  const canSave =
    name.trim().length >= 2 && owner.trim().length >= 9 && backup.trim().length >= 9;

  return (
    <>
      <header className="flex items-center gap-3 px-4 pb-2 pt-11">
        <Link
          href="/"
          className="glass-chip pressable flex h-10 w-10 items-center justify-center rounded-full"
        >
          <ArrowLeft className="h-4 w-4" />
        </Link>
        <div className="min-w-0 flex-1">
          <p className="text-[11px] text-muted-foreground">First-time setup</p>
          <h1 className="font-[family-name:var(--font-display)] text-xl font-bold">
            Name this unit
          </h1>
        </div>
        <ThemeToggle />
      </header>

      <main className="flex min-h-0 flex-1 flex-col overflow-y-auto px-5 pb-28">
        <div className="glass fade-up mb-4 flex items-center gap-3 rounded-[1.4rem] p-4">
          <BrandMark size={48} />
          <div>
            <p className="font-semibold">{device.bleName}</p>
            <p className="font-mono text-[11px] text-muted-foreground">
              {device.serial}
            </p>
          </div>
        </div>

        <div className="glass fade-up space-y-4 rounded-[1.5rem] p-4">
          <div>
            <Label htmlFor="setup-name">Device name</Label>
            <Input
              id="setup-name"
              className="mt-2 h-12 rounded-2xl"
              placeholder="e.g. Block B kitchen"
              value={name}
              onChange={(e) => setName(e.target.value)}
            />
          </div>
          <div>
            <Label>Place type</Label>
            <div className="mt-2 grid grid-cols-2 gap-2">
              {PLACES.map((p) => (
                <button
                  key={p}
                  type="button"
                  onClick={() => setPlace(p)}
                  className={`rounded-2xl px-3 py-3 text-left text-[13px] font-medium transition ${
                    place === p
                      ? "bg-primary text-primary-foreground shadow-md shadow-primary/25"
                      : "bg-foreground/[0.04] dark:bg-white/[0.06]"
                  }`}
                >
                  {p}
                </button>
              ))}
            </div>
          </div>
          <div>
            <Label htmlFor="setup-owner">Owner phone (SMS + call)</Label>
            <Input
              id="setup-owner"
              className="mt-2 h-12 rounded-2xl"
              placeholder="024…"
              inputMode="tel"
              value={owner}
              onChange={(e) => setOwner(e.target.value)}
            />
          </div>
          <div>
            <Label htmlFor="setup-backup">Backup phone (SMS after 3 min)</Label>
            <Input
              id="setup-backup"
              className="mt-2 h-12 rounded-2xl"
              placeholder="020…"
              inputMode="tel"
              value={backup}
              onChange={(e) => setBackup(e.target.value)}
            />
          </div>
        </div>
      </main>

      <div className="absolute inset-x-0 bottom-0 z-20 px-5 pb-6 pt-3">
        <div className="glass-strong rounded-[1.4rem] p-3">
          <Button
            type="button"
            disabled={!canSave}
            className="h-12 w-full rounded-2xl text-[15px] font-semibold"
            onClick={() =>
              onDone({
                name: name.trim(),
                place,
                ownerPhone: owner.trim(),
                backupPhone: backup.trim(),
              })
            }
          >
            Save & open device
          </Button>
        </div>
      </div>
    </>
  );
}
