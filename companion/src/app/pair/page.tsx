"use client";

import { useEffect, useState } from "react";
import { useRouter } from "next/navigation";
import { PhoneShell } from "@/components/phone-shell";
import { Button } from "@/components/ui/button";
import { NEARBY_POOL } from "@/lib/types";
import { useDevices } from "@/lib/store";
import { ArrowLeft, Bluetooth } from "lucide-react";
import Link from "next/link";

export default function PairPage() {
  const router = useRouter();
  const { pair, devices } = useDevices();
  const [scanning, setScanning] = useState(true);
  const [visible, setVisible] = useState(0);
  const pairedSerials = new Set(devices.map((d) => d.serial));

  useEffect(() => {
    setScanning(true);
    setVisible(0);
    const t1 = setTimeout(() => setVisible(1), 500);
    const t2 = setTimeout(() => setVisible(2), 1100);
    const t3 = setTimeout(() => {
      setVisible(3);
      setScanning(false);
    }, 1700);
    return () => {
      clearTimeout(t1);
      clearTimeout(t2);
      clearTimeout(t3);
    };
  }, []);

  const shown = NEARBY_POOL.slice(0, visible);

  return (
    <PhoneShell>
      <header className="flex items-center gap-3 px-4 pb-3 pt-10">
        <Link
          href="/"
          className="flex h-10 w-10 items-center justify-center rounded-full border border-[#3a3428]"
        >
          <ArrowLeft className="h-4 w-4" />
        </Link>
        <div>
          <p className="font-mono text-[10px] tracking-[0.18em] text-primary uppercase">
            Bluetooth
          </p>
          <h1 className="font-[family-name:var(--font-display)] text-2xl font-bold">
            Pair AeroGuard
          </h1>
        </div>
      </header>

      <main className="flex flex-1 flex-col px-5 pb-8">
        <div className="mb-5 flex items-center gap-2 rounded-2xl border border-[#3a3428] bg-[#14120e] px-3 py-3 text-sm text-muted-foreground">
          <Bluetooth className={`h-4 w-4 text-primary ${scanning ? "animate-pulse" : ""}`} />
          {scanning
            ? "Scanning nearby AeroGuard-X1 units…"
            : "Select a unit to pair. Contest mode simulates HM-10 BLE."}
        </div>

        <ul className="flex flex-col gap-2">
          {shown.map((u) => {
            const already = pairedSerials.has(u.serial);
            return (
              <li key={u.serial}>
                <button
                  type="button"
                  disabled={already}
                  onClick={() => {
                    const d = pair(u.serial, u.bleName);
                    router.push(`/d/${d.id}`);
                  }}
                  className="flex w-full items-center justify-between rounded-2xl border border-[#3a3428] bg-[#161410] px-4 py-3 text-left disabled:opacity-40"
                >
                  <div>
                    <p className="font-semibold">{u.bleName}</p>
                    <p className="font-mono text-[11px] text-muted-foreground">
                      {u.serial} · {u.rssi} dBm
                    </p>
                    <p className="text-[11px] text-muted-foreground">{u.hint}</p>
                  </div>
                  <span className="font-mono text-[10px] text-primary">
                    {already ? "PAIRED" : "PAIR"}
                  </span>
                </button>
              </li>
            );
          })}
        </ul>

        {visible === 0 && (
          <p className="mt-6 text-center text-sm text-muted-foreground">
            Hold the phone near the device…
          </p>
        )}

        <Button
          variant="ghost"
          className="mt-auto text-muted-foreground"
          onClick={() => router.push("/")}
        >
          Cancel
        </Button>
      </main>
    </PhoneShell>
  );
}
