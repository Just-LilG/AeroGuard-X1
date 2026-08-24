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

export function ActivityTab({ device }: { device: AeroDevice }) {
  const logs = useMemo(
    () => [...device.log].sort((a, b) => b.at - a.at),
    [device.log]
  );

  return (
    <div className="fade-up space-y-3">
      <div className="glass rounded-[1.5rem] p-4">
        <p className="font-[family-name:var(--font-display)] text-[15px] font-semibold">
          Incident timeline
        </p>
        <p className="mt-1 text-[13px] text-muted-foreground">
          Stage changes, GSM actions, vent links, and setup for this AeroGuard.
        </p>
      </div>

      {logs.length === 0 ? (
        <div className="glass rounded-[1.5rem] px-4 py-10 text-center">
          <p className="text-[15px] font-medium">No activity yet</p>
          <p className="mt-1 text-[13px] text-muted-foreground">
            Run a demo cycle from Status to fill this feed.
          </p>
        </div>
      ) : (
        logs.map((log, i) => {
          const color =
            log.stage === "SYSTEM"
              ? undefined
              : STAGE_META[log.stage as keyof typeof STAGE_META]?.color;
          return (
            <div
              key={log.id}
              className="glass fade-up flex gap-3 rounded-[1.35rem] p-3.5"
              style={{ animationDelay: `${i * 35}ms` }}
            >
              <div
                className="mt-0.5 flex h-10 w-10 shrink-0 items-center justify-center rounded-2xl bg-primary/12 text-primary"
                style={
                  color
                    ? { background: `${color}22`, color }
                    : undefined
                }
              >
                {log.stage === "SYSTEM" ? (
                  <Radio className="h-4 w-4" />
                ) : log.title.toLowerCase().includes("call") ||
                  log.title.toLowerCase().includes("sms") ? (
                  <Phone className="h-4 w-4" />
                ) : log.title.toLowerCase().includes("vent") ? (
                  <Wind className="h-4 w-4" />
                ) : (
                  <Flame className="h-4 w-4" />
                )}
              </div>
              <div className="min-w-0 flex-1">
                <div className="flex items-start justify-between gap-2">
                  <p className="text-[14px] font-semibold">{log.title}</p>
                  <time className="shrink-0 text-[11px] text-muted-foreground">
                    {new Date(log.at).toLocaleTimeString([], {
                      hour: "2-digit",
                      minute: "2-digit",
                    })}
                  </time>
                </div>
                <p className="mt-0.5 text-[13px] leading-snug text-muted-foreground">
                  {log.detail}
                </p>
              </div>
            </div>
          );
        })
      )}
    </div>
  );
}
