"use client";

import { useRouter } from "next/navigation";
import { PhoneShell } from "@/components/phone-shell";
import { StatusDot } from "@/components/status-dot";
import { Button } from "@/components/ui/button";
import { STAGE_META } from "@/lib/types";
import { useDevices } from "@/lib/store";
import { Radio, Plus } from "lucide-react";

export default function HomePage() {
  const { ready, devices } = useDevices();
  const router = useRouter();
  const setupNeeded = devices.filter((d) => !d.setupComplete).length;

  return (
    <PhoneShell>
      <header className="grain flex items-end justify-between px-5 pb-4 pt-10">
        <div>
          <p className="font-mono text-[10px] tracking-[0.22em] text-primary uppercase">
            Companion
          </p>
          <h1 className="font-[family-name:var(--font-display)] text-3xl font-extrabold tracking-tight">
            AeroGuard
          </h1>
        </div>
        <span className="rounded-full border border-[#3a3428] px-2.5 py-1 font-mono text-[10px] text-muted-foreground">
          {devices.length} unit{devices.length === 1 ? "" : "s"}
        </span>
      </header>

      <main className="flex flex-1 flex-col gap-4 px-5 pb-6">
        {!ready ? (
          <p className="text-sm text-muted-foreground">Loading…</p>
        ) : devices.length === 0 ? (
          <div className="flex flex-1 flex-col items-center justify-center rounded-3xl border border-dashed border-[#3a3428] bg-[#14120e] px-6 py-16 text-center">
            <div className="mb-4 flex h-14 w-14 items-center justify-center rounded-2xl bg-[#1f1c16] text-primary">
              <Radio className="h-7 w-7" />
            </div>
            <h2 className="font-[family-name:var(--font-display)] text-xl font-bold">
              No unit paired
            </h2>
            <p className="mt-2 max-w-[16rem] text-sm text-muted-foreground">
              Pair an AeroGuard-X1 to watch status, set numbers, and link smart
              vents — all per device.
            </p>
            <Button
              className="mt-6 h-12 rounded-full px-6 font-semibold"
              onClick={() => router.push("/pair")}
            >
              Pair AeroGuard
            </Button>
          </div>
        ) : (
          <>
            {setupNeeded > 0 && (
              <p className="rounded-2xl border border-primary/30 bg-primary/10 px-3 py-2 text-xs text-primary">
                {setupNeeded} unit{setupNeeded === 1 ? "" : "s"} still need
                first-time setup.
              </p>
            )}
            <ul className="flex flex-col gap-3">
              {devices.map((d) => {
                const title = d.setupComplete ? d.name : "Finish setup";
                const meta = STAGE_META[d.stage];
                return (
                  <li key={d.id}>
                    <button
                      type="button"
                      onClick={() => router.push(`/d/${d.id}`)}
                      className="flex w-full items-center gap-3 rounded-3xl border border-[#3a3428] bg-[#161410] p-4 text-left transition hover:border-primary/50"
                    >
                      <StatusDot stage={d.stage} size="md" />
                      <div className="min-w-0 flex-1">
                        <p className="truncate font-[family-name:var(--font-display)] text-lg font-bold">
                          {title}
                        </p>
                        <p className="truncate font-mono text-[11px] text-muted-foreground">
                          {d.serial}
                          {d.setupComplete ? ` · ${d.place}` : " · not set up"}
                        </p>
                      </div>
                      <div className="text-right">
                        <p
                          className="font-mono text-[11px] font-medium"
                          style={{ color: meta.color }}
                        >
                          {meta.label}
                        </p>
                        <p className="text-[10px] text-muted-foreground">
                          {d.setupComplete ? "Open" : "Setup"}
                        </p>
                      </div>
                    </button>
                  </li>
                );
              })}
            </ul>
            <Button
              variant="outline"
              className="mt-auto h-12 rounded-full border-[#3a3428] bg-transparent"
              onClick={() => router.push("/pair")}
            >
              <Plus className="mr-2 h-4 w-4" />
              Pair another AeroGuard
            </Button>
          </>
        )}
      </main>

      <p className="px-5 pb-6 text-center font-mono text-[10px] text-muted-foreground">
        Contest demo · BLE scan is simulated · GSM alerts stay on the box
      </p>
    </PhoneShell>
  );
}
