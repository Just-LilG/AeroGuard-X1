"use client";

import { useMemo } from "react";
import { useRouter } from "next/navigation";
import { PhoneShell } from "@/components/phone-shell";
import { BrandMark } from "@/components/brand-mark";
import { ThemeToggle } from "@/components/theme-toggle";
import { StatusDot } from "@/components/status-dot";
import { STAGE_META } from "@/lib/types";
import { useDevices } from "@/lib/store";
import {
  Plus,
  ShieldCheck,
  Wind,
  Radio,
  ChevronRight,
  Sparkles,
} from "lucide-react";

export default function HomePage() {
  const { ready, devices, advanceDemo, resetDevice } = useDevices();
  const router = useRouter();
  const setupNeeded = devices.filter((d) => !d.setupComplete).length;
  const alerting = devices.filter(
    (d) => d.setupComplete && d.stage !== "SAFE"
  ).length;
  const openVents = devices.reduce(
    (n, d) => n + d.vents.filter((v) => v.open).length,
    0
  );
  const readyUnits = devices.filter((d) => d.setupComplete);

  const greeting = useMemo(() => {
    const h = new Date().getHours();
    if (h < 12) return "Good morning";
    if (h < 18) return "Good afternoon";
    return "Good evening";
  }, []);

  return (
    <PhoneShell>
      <header className="fade-up flex items-start justify-between px-5 pb-3 pt-11">
        <div className="flex items-center gap-3">
          <BrandMark size={40} />
          <div>
            <p className="text-[13px] text-muted-foreground">{greeting}</p>
            <h1 className="font-[family-name:var(--font-display)] text-[28px] font-bold leading-none tracking-tight">
              AeroGuard
            </h1>
          </div>
        </div>
        <div className="flex items-center gap-2">
          <ThemeToggle />
          <button
            type="button"
            onClick={() => router.push("/pair")}
            className="glass-chip pressable flex h-10 w-10 items-center justify-center rounded-full text-primary"
            aria-label="Pair AeroGuard"
          >
            <Plus className="h-5 w-5" />
          </button>
        </div>
      </header>

      <main className="flex min-h-0 flex-1 flex-col gap-4 overflow-y-auto px-5 pb-8">
        {!ready ? (
          <div className="glass fade-up rounded-[1.6rem] p-6 text-sm text-muted-foreground">
            Loading home…
          </div>
        ) : (
          <>
            <section className="fade-up grid grid-cols-3 gap-2">
              <StatTile
                icon={<ShieldCheck className="h-4 w-4" />}
                label="Units"
                value={String(devices.length)}
              />
              <StatTile
                icon={<Radio className="h-4 w-4" />}
                label="Alerts"
                value={String(alerting)}
                warn={alerting > 0}
              />
              <StatTile
                icon={<Wind className="h-4 w-4" />}
                label="Vents open"
                value={String(openVents)}
              />
            </section>

            <section className="fade-up glass rounded-[1.6rem] p-4">
              <div className="mb-3 flex items-center justify-between">
                <p className="font-[family-name:var(--font-display)] text-sm font-semibold">
                  Scenes
                </p>
                <Sparkles className="h-3.5 w-3.5 text-primary" />
              </div>
              <div className="grid grid-cols-2 gap-2">
                <SceneChip
                  title="All quiet"
                  detail="Reset every unit to Safe"
                  onClick={() => {
                    if (readyUnits.length === 0) {
                      router.push("/pair");
                      return;
                    }
                    readyUnits.forEach((d) => resetDevice(d.id));
                  }}
                />
                <SceneChip
                  title="Demo leak"
                  detail="Advance first unit one stage"
                  onClick={() => {
                    if (readyUnits.length === 0) {
                      router.push("/pair");
                      return;
                    }
                    advanceDemo(readyUnits[0].id);
                    router.push(`/d/${readyUnits[0].id}`);
                  }}
                />
              </div>
            </section>

            <section className="fade-up glass rounded-[1.6rem] p-4">
              <p className="font-[family-name:var(--font-display)] text-sm font-semibold">
                How this home works
              </p>
              <ul className="mt-2 space-y-2 text-[12px] leading-relaxed text-muted-foreground">
                <li>Each card is one AeroGuard — open it for status, vents, and phones.</li>
                <li>MEDIUM texts the owner; CRITICAL / FIRE also calls.</li>
                <li>Backup gets SMS only after 3 minutes if nobody clears.</li>
              </ul>
            </section>

            {setupNeeded > 0 && (
              <p className="fade-up rounded-2xl border border-amber-400/40 bg-amber-400/15 px-3 py-2 text-xs text-amber-800 dark:text-amber-200">
                {setupNeeded} unit{setupNeeded === 1 ? "" : "s"} still need
                first-time setup.
              </p>
            )}

            <section className="fade-up">
              <div className="mb-3 flex items-end justify-between px-0.5">
                <h2 className="font-[family-name:var(--font-display)] text-lg font-semibold">
                  Devices
                </h2>
                <button
                  type="button"
                  onClick={() => router.push("/pair")}
                  className="text-xs font-medium text-primary"
                >
                  Add device
                </button>
              </div>

              {devices.length === 0 ? (
                <div className="glass flex flex-col items-center rounded-[1.8rem] px-6 py-12 text-center">
                  <div className="mb-4 flex h-16 w-16 items-center justify-center rounded-[1.4rem] bg-primary/15 text-primary">
                    <Radio className="h-7 w-7" />
                  </div>
                  <h3 className="font-[family-name:var(--font-display)] text-xl font-bold">
                    Your home is empty
                  </h3>
                  <p className="mt-2 max-w-[17rem] text-sm text-muted-foreground">
                    Pair an AeroGuard-X1 to see live stage, contacts, and linked
                    smart vents — like a Mi Home device card.
                  </p>
                  <button
                    type="button"
                    onClick={() => router.push("/pair")}
                    className="pressable mt-6 h-12 rounded-full bg-primary px-6 text-sm font-semibold text-primary-foreground shadow-lg shadow-primary/25"
                  >
                    Pair AeroGuard
                  </button>
                </div>
              ) : (
                <ul className="grid grid-cols-2 gap-3">
                  {devices.map((d, i) => {
                    const title = d.setupComplete ? d.name : "Finish setup";
                    const meta = STAGE_META[d.stage];
                    return (
                      <li
                        key={d.id}
                        className="fade-up"
                        style={{ animationDelay: `${i * 60}ms` }}
                      >
                        <button
                          type="button"
                          onClick={() => router.push(`/d/${d.id}`)}
                          className="glass pressable flex h-full min-h-[148px] w-full flex-col items-start rounded-[1.5rem] p-3.5 text-left"
                        >
                          <div className="mb-3 flex w-full items-start justify-between">
                            <div className="flex h-10 w-10 items-center justify-center rounded-2xl bg-primary/12 text-primary">
                              <ShieldCheck className="h-5 w-5" />
                            </div>
                            <StatusDot stage={d.stage} size="md" />
                          </div>
                          <p className="line-clamp-2 font-[family-name:var(--font-display)] text-[15px] font-semibold leading-tight">
                            {title}
                          </p>
                          <p className="mt-1 line-clamp-1 text-[11px] text-muted-foreground">
                            {d.setupComplete ? d.place : d.serial}
                          </p>
                          <div className="mt-auto flex w-full items-center justify-between pt-3">
                            <span
                              className="rounded-full px-2 py-0.5 text-[10px] font-semibold"
                              style={{
                                color: meta.color,
                                background: `${meta.color}22`,
                              }}
                            >
                              {meta.label}
                            </span>
                            <ChevronRight className="h-4 w-4 text-muted-foreground" />
                          </div>
                        </button>
                      </li>
                    );
                  })}
                  <li>
                    <button
                      type="button"
                      onClick={() => router.push("/pair")}
                      className="glass pressable flex min-h-[148px] w-full flex-col items-center justify-center gap-2 rounded-[1.5rem] border-dashed text-muted-foreground"
                    >
                      <Plus className="h-6 w-6" />
                      <span className="text-xs font-medium">Add device</span>
                    </button>
                  </li>
                </ul>
              )}
            </section>

            <p className="pt-2 text-center font-mono text-[10px] text-muted-foreground">
              Contest demo · ESP32 WiFi remote · GSM on the box
            </p>
          </>
        )}
      </main>
    </PhoneShell>
  );
}

function StatTile({
  icon,
  label,
  value,
  warn,
}: {
  icon: React.ReactNode;
  label: string;
  value: string;
  warn?: boolean;
}) {
  return (
    <div className="glass rounded-2xl px-3 py-3">
      <div
        className={`mb-2 flex h-7 w-7 items-center justify-center rounded-full ${warn ? "bg-rose-500/15 text-rose-500" : "bg-primary/12 text-primary"}`}
      >
        {icon}
      </div>
      <p className="font-[family-name:var(--font-display)] text-xl font-bold leading-none">
        {value}
      </p>
      <p className="mt-1 text-[10px] text-muted-foreground">{label}</p>
    </div>
  );
}

function SceneChip({
  title,
  detail,
  onClick,
}: {
  title: string;
  detail: string;
  onClick: () => void;
}) {
  return (
    <button
      type="button"
      onClick={onClick}
      className="pressable rounded-2xl bg-foreground/[0.04] px-3 py-3 text-left dark:bg-white/[0.05]"
    >
      <p className="text-sm font-semibold">{title}</p>
      <p className="mt-0.5 text-[11px] text-muted-foreground">{detail}</p>
    </button>
  );
}
