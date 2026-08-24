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

export function VentsTab({
  device,
  onAdd,
  onToggle,
  onRemove,
}: {
  device: AeroDevice;
  onAdd: (name: string, kind: VentKind) => void;
  onToggle: (ventId: string) => void;
  onRemove: (ventId: string) => void;
}) {
  const [name, setName] = useState("");
  const [kind, setKind] = useState<VentKind>("window");
  const openCount = device.vents.filter((v) => v.open).length;

  return (
    <div className="fade-up space-y-4">
      <div className="glass overflow-hidden rounded-[1.7rem]">
        <div
          className={`px-5 pb-5 pt-5 transition-colors duration-500 ${
            openCount > 0
              ? "bg-gradient-to-br from-sky-400/20 via-transparent to-cyan-300/10"
              : "bg-gradient-to-br from-slate-500/10 via-transparent to-transparent"
          }`}
        >
          <div className="flex items-start justify-between gap-3">
            <div>
              <p className="text-[11px] font-semibold uppercase tracking-[0.14em] text-muted-foreground">
                Vent scene
              </p>
              <p className="mt-1 font-[family-name:var(--font-display)] text-[24px] font-bold tracking-tight">
                {openCount > 0 ? "Airflow open" : "All sealed"}
              </p>
              <p className="mt-1 text-[14px] text-muted-foreground">
                {device.vents.length === 0
                  ? "No linked windows or vents yet"
                  : `${openCount} open · ${device.vents.length} linked`}
              </p>
            </div>
            <div
              className={`flex h-14 w-14 items-center justify-center rounded-2xl ${
                openCount > 0
                  ? "bg-sky-500/20 text-sky-600 dark:text-sky-300"
                  : "bg-foreground/[0.05] text-muted-foreground"
              }`}
            >
              <Wind className="h-7 w-7" />
            </div>
          </div>
        </div>
      </div>

      {device.vents.length === 0 ? (
        <div className="glass rounded-[1.5rem] px-4 py-8 text-center">
          <p className="font-[family-name:var(--font-display)] text-lg font-semibold">
            Link a window or vent
          </p>
          <p className="mx-auto mt-2 max-w-[16rem] text-[13px] text-muted-foreground">
            Like Xiaomi Home scenes — AeroGuard sends the open command; your smart
            window / vent executes it (simulated for contest).
          </p>
        </div>
      ) : (
        <ul className="space-y-2.5">
          {device.vents.map((v) => (
            <VentRow
              key={v.id}
              vent={v}
              onToggle={() => onToggle(v.id)}
              onRemove={() => onRemove(v.id)}
            />
          ))}
        </ul>
      )}

      <div className="glass space-y-3 rounded-[1.5rem] p-4">
        <p className="text-[15px] font-semibold">Add link</p>
        <div className="grid grid-cols-2 gap-2">
          {(["window", "vent"] as VentKind[]).map((k) => (
            <button
              key={k}
              type="button"
              onClick={() => setKind(k)}
              className={`flex items-center justify-center gap-2 rounded-2xl py-3 text-[13px] font-semibold capitalize ${
                kind === k
                  ? "bg-primary text-primary-foreground"
                  : "bg-foreground/[0.04] dark:bg-white/[0.06]"
              }`}
            >
              {k === "window" ? (
                <AppWindow className="h-4 w-4" />
              ) : (
                <Wind className="h-4 w-4" />
              )}
              {k}
            </button>
          ))}
        </div>
        <Input
          className="h-12 rounded-2xl"
          placeholder={kind === "window" ? "Kitchen window" : "Cookhouse vent"}
          value={name}
          onChange={(e) => setName(e.target.value)}
        />
        <Button
          type="button"
          className="h-11 w-full rounded-2xl"
          disabled={name.trim().length < 2}
          onClick={() => {
            onAdd(name.trim(), kind);
            setName("");
          }}
        >
          <Plus className="mr-1.5 h-4 w-4" />
          Link {kind}
        </Button>
      </div>

      <div className="grid grid-cols-2 gap-2">
        {[
          { label: "Kitchen window", kind: "window" as VentKind },
          { label: "Cookhouse vent", kind: "vent" as VentKind },
          { label: "Corridor window", kind: "window" as VentKind },
          { label: "Roof vent", kind: "vent" as VentKind },
        ].map((preset) => (
          <button
            key={preset.label}
            type="button"
            onClick={() => onAdd(preset.label, preset.kind)}
            className="glass pressable rounded-[1.25rem] p-3 text-left"
          >
            <p className="text-[13px] font-semibold">{preset.label}</p>
            <p className="text-[11px] capitalize text-muted-foreground">
              Quick add · {preset.kind}
            </p>
          </button>
        ))}
      </div>
    </div>
  );
}

export function VentRow({
  vent,
  onToggle,
  onRemove,
}: {
  vent: SmartVent;
  onToggle: () => void;
  onRemove: () => void;
}) {
  return (
    <li className="glass flex items-center gap-3 rounded-[1.35rem] p-3.5">
      <div
        className={`flex h-11 w-11 items-center justify-center rounded-2xl ${
          vent.open
            ? "bg-sky-500/20 text-sky-600 dark:text-sky-300"
            : "bg-foreground/[0.05] text-muted-foreground"
        }`}
      >
        {vent.kind === "window" ? (
          <AppWindow className="h-5 w-5" />
        ) : (
          <Wind className="h-5 w-5" />
        )}
      </div>
      <div className="min-w-0 flex-1">
        <p className="truncate text-[14px] font-semibold">{vent.name}</p>
        <p className="text-[11px] text-muted-foreground">
          {vent.kind} · auto on alert {vent.autoOnAlert ? "on" : "off"}
        </p>
      </div>
      <button
        type="button"
        role="switch"
        aria-checked={vent.open}
        onClick={onToggle}
        className={`relative h-8 w-14 shrink-0 rounded-full transition-colors ${
          vent.open ? "bg-sky-500" : "bg-foreground/15"
        }`}
      >
        <span
          className={`absolute top-1 h-6 w-6 rounded-full bg-white shadow transition-all ${
            vent.open ? "left-7" : "left-1"
          }`}
        />
      </button>
      <button
        type="button"
        onClick={onRemove}
        className="text-muted-foreground hover:text-destructive"
        aria-label="Remove"
      >
        <Trash2 className="h-4 w-4" />
      </button>
    </li>
  );
}
