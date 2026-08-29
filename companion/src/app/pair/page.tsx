"use client";

import { useEffect, useRef, useState } from "react";
import { useRouter } from "next/navigation";
import Link from "next/link";
import { PhoneShell } from "@/components/phone-shell";
import { ThemeToggle } from "@/components/theme-toggle";
import { BrandMark } from "@/components/brand-mark";
import { NEARBY_POOL } from "@/lib/types";
import { useDevices } from "@/lib/store";
import { ArrowLeft, Bluetooth, Signal } from "lucide-react";

export default function PairPage() {
  const router = useRouter();
  const { pair, devices, ready } = useDevices();
  const [scanning, setScanning] = useState(true);
  const [visible, setVisible] = useState(0);
  const [pairingSerial, setPairingSerial] = useState<string | null>(null);
  const pairingLock = useRef(false);
  const pairedSerials = new Set(devices.map((d) => d.serial));

  useEffect(() => {
    if (!ready) return;
    setScanning(true);
    setVisible(0);
    const t1 = setTimeout(() => setVisible(1), 450);
    const t2 = setTimeout(() => setVisible(2), 950);
    const t3 = setTimeout(() => {
      setVisible(3);
      setScanning(false);
    }, 1500);
    return () => {
      clearTimeout(t1);
      clearTimeout(t2);
      clearTimeout(t3);
    };
  }, [ready]);

  const shown = NEARBY_POOL.slice(0, visible);

  return (
    <PhoneShell>
      <header className="flex items-center gap-3 px-4 pb-2 pt-11">
        <Link
          href="/"
          className="glass-chip pressable flex h-10 w-10 items-center justify-center rounded-full"
          aria-label="Back"
        >
          <ArrowLeft className="h-4 w-4" />
        </Link>
        <div className="min-w-0 flex-1">
          <p className="text-[11px] text-muted-foreground">Add device</p>
          <h1 className="font-[family-name:var(--font-display)] text-xl font-bold">
            Pair AeroGuard
          </h1>
        </div>
        <ThemeToggle />
      </header>

      <main className="flex min-h-0 flex-1 flex-col overflow-y-auto px-5 pb-8">
        <div className="glass fade-up mb-5 flex items-center gap-3 rounded-[1.4rem] p-4">
          <div className="relative flex h-12 w-12 items-center justify-center">
            <span
              className={`absolute inset-0 rounded-full bg-primary/20 ${scanning ? "animate-ping" : ""}`}
            />
            <div className="relative flex h-12 w-12 items-center justify-center rounded-full bg-primary/15 text-primary">
              <Bluetooth className="h-5 w-5" />
            </div>
          </div>
          <div>
            <p className="text-sm font-semibold">
              {scanning ? "Scanning nearby…" : "Select a unit"}
            </p>
            <p className="text-xs text-muted-foreground">
              Contest mode simulates discovery. On hardware, the ESP32 serves WiFi status for remote access.
            </p>
          </div>
        </div>

        <ul className="flex flex-col gap-2.5">
          {shown.map((u, i) => {
            const already = pairedSerials.has(u.serial);
            return (
              <li
                key={u.serial}
                className="fade-up"
                style={{ animationDelay: `${i * 70}ms` }}
              >
                <button
                  type="button"
                  disabled={already || !ready || pairingSerial !== null}
                  onClick={() => {
                    if (!ready || already || pairingLock.current) return;
                    pairingLock.current = true;
                    setPairingSerial(u.serial);
                    const d = pair(u.serial, u.bleName);
                    router.push(`/d/${d.id}`);
                  }}
                  className="glass pressable flex w-full items-center gap-3 rounded-[1.35rem] p-3.5 text-left disabled:opacity-45"
                >
                  <BrandMark size={42} />
                  <div className="min-w-0 flex-1">
                    <p className={'font-semibold'}>{u.bleName}</p>
                    <p className="font-mono text-[11px] text-muted-foreground">
                      {u.serial}
                    </p>
                    <p className="mt-0.5 flex items-center gap-1 text-[11px] text-muted-foreground">
                      <Signal className="h-3 w-3" />
                      {u.rssi} dBm · {u.hint}
                    </p>
                  </div>
                  <span className="rounded-full bg-primary/15 px-2.5 py-1 font-mono text-[10px] font-semibold text-primary">
                    {already ? "PAIRED" : pairingSerial === u.serial ? "…" : "PAIR"}
                  </span>
                </button>
              </li>
            );
          })}
        </ul>

        {visible === 0 && (
          <p className="mt-10 text-center text-sm text-muted-foreground">
            Hold the phone near the device…
          </p>
        )}
      </main>
    </PhoneShell>
  );
}
