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

export function DeviceHub({ device }: { device: AeroDevice }) {
  const router = useRouter();
  const store = useDevices();
  const [tab, setTab] = useState<DeviceTab>("status");
  const [toast, setToast] = useState<string | null>(null);

  function flash(msg: string) {
    setToast(msg);
    window.setTimeout(() => setToast(null), 2200);
  }

  if (!device.setupComplete) {
    return (
      <PhoneShell>
        <SetupFlow
          device={device}
          onDone={(data) => {
            store.completeSetup(device.id, data);
            flash("Setup saved");
          }}
        />
      </PhoneShell>
    );
  }

  return (
    <PhoneShell>
      <header className="sticky top-0 z-20 px-4 pb-3 pt-11">
        <div className="glass-strong rounded-[1.35rem] px-3 py-2.5">
          <div className="flex items-center gap-2">
            <Link
              href="/"
              className="glass-chip pressable flex h-10 w-10 items-center justify-center rounded-full"
              aria-label="Back"
            >
              <ArrowLeft className="h-4 w-4" />
            </Link>
            <div className="min-w-0 flex-1">
              <p className="truncate font-[family-name:var(--font-display)] text-[17px] font-bold tracking-tight">
                {device.name}
              </p>
              <p className="truncate text-[11px] text-muted-foreground">
                {device.place} · {device.serial}
              </p>
            </div>
            <ThemeToggle />
          </div>
          <nav className="mt-2.5 flex gap-1 rounded-2xl bg-foreground/[0.04] p-1 dark:bg-white/[0.06]">
            {TABS.map((t) => (
              <button
                key={t.id}
                type="button"
                onClick={() => setTab(t.id)}
                className={`min-h-10 flex-1 rounded-xl px-1 text-[12px] font-semibold transition ${
                  tab === t.id
                    ? "bg-background/90 text-foreground shadow-sm dark:bg-white/15"
                    : "text-muted-foreground"
                }`}
              >
                {t.label}
              </button>
            ))}
          </nav>
        </div>
      </header>

      <main className="min-h-0 flex-1 overflow-y-auto px-5 pb-10">
        {tab === "status" && (
          <StatusTab
            device={device}
            onDemo={() => {
              store.advanceDemo(device.id);
              flash("Demo advanced");
            }}
            onReset={() => {
              store.resetDevice(device.id);
              flash("Reset to Safe");
            }}
          />
        )}
        {tab === "activity" && <ActivityTab device={device} />}
        {tab === "vents" && (
          <VentsTab
            device={device}
            onAdd={(name, kind) => {
              store.addVent(device.id, name, kind);
              flash(`Linked ${kind}`);
            }}
            onToggle={(ventId) => store.toggleVent(device.id, ventId)}
            onRemove={(ventId) => {
              store.removeVent(device.id, ventId);
              flash("Removed link");
            }}
          />
        )}
        {tab === "device" && (
          <DeviceTab
            device={device}
            onRename={(name, place) => {
              store.rename(device.id, name, place);
              flash("Name saved");
            }}
            onContacts={(owner, backup) => {
              store.updateContacts(device.id, owner, backup);
              flash("Contacts saved");
            }}
            onUnpair={() => {
              store.unpair(device.id);
              router.push("/");
            }}
          />
        )}
      </main>

      {toast && (
        <div className="pointer-events-none absolute bottom-8 left-1/2 z-50 -translate-x-1/2 fade-up">
          <div className="glass-strong rounded-full px-5 py-2.5 text-[13px] font-medium shadow-lg">
            {toast}
          </div>
        </div>
      )}
    </PhoneShell>
  );
}

export { SetupFlow } from "./device-hub-setup";
export { StatusTab } from "./device-hub-status";
export { ActivityTab } from "./device-hub-activity";
export { VentsTab } from "./device-hub-vents";
export { DeviceTab } from "./device-hub-settings";
