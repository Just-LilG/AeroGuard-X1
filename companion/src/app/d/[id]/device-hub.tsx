"use client";

import { useState } from "react";
import Link from "next/link";
import { useRouter } from "next/navigation";
import {
  ArrowLeft,
  Activity,
  Gauge,
  Settings2,
  Wind,
} from "lucide-react";
import { ThemeToggle } from "@/components/theme-toggle";
import { PhoneShell } from "@/components/phone-shell";
import { useDevices } from "@/lib/store";
import { type AeroDevice, type DeviceTab as DeviceTabId } from "@/lib/types";
import { SetupFlow } from "./device-hub-setup";
import { StatusTab } from "./device-hub-status";
import { ActivityTab } from "./device-hub-activity";
import { VentsTab } from "./device-hub-vents";
import { DeviceTab } from "./device-hub-settings";

const TABS: { id: DeviceTabId; label: string; icon: typeof Gauge }[] = [
  { id: "status", label: "Status", icon: Gauge },
  { id: "activity", label: "Activity", icon: Activity },
  { id: "vents", label: "Vents", icon: Wind },
  { id: "device", label: "Device", icon: Settings2 },
];

export function DeviceHub({ device }: { device: AeroDevice }) {
  const router = useRouter();
  const store = useDevices();
  const [tab, setTab] = useState<DeviceTabId>("status");
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
      <header className="flex items-center gap-2 px-4 pb-2 pt-11">
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
      </header>

      <main className="min-h-0 flex-1 overflow-y-auto px-5 pb-28">
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

      <nav className="absolute inset-x-3 bottom-3 z-30">
        <div className="glass-strong grid grid-cols-4 gap-1 rounded-[1.5rem] p-1.5 shadow-lg">
          {TABS.map((t) => {
            const Icon = t.icon;
            const active = tab === t.id;
            return (
              <button
                key={t.id}
                type="button"
                data-tab={t.id}
                aria-current={active ? "page" : undefined}
                onClick={() => setTab(t.id)}
                className={`flex min-h-14 flex-col items-center justify-center gap-0.5 rounded-[1.15rem] px-1 text-[11px] font-semibold transition ${
                  active
                    ? "bg-primary text-primary-foreground shadow-md shadow-primary/30"
                    : "text-muted-foreground active:bg-foreground/5"
                }`}
              >
                <Icon className="h-5 w-5" />
                {t.label}
              </button>
            );
          })}
        </div>
      </nav>

      {toast && (
        <div className="pointer-events-none absolute bottom-24 left-1/2 z-50 -translate-x-1/2 fade-up">
          <div className="glass-strong rounded-full px-5 py-2.5 text-[13px] font-medium shadow-lg">
            {toast}
          </div>
        </div>
      )}
    </PhoneShell>
  );
}
