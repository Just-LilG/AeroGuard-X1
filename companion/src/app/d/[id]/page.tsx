"use client";

import { use } from "react";
import Link from "next/link";
import { PhoneShell } from "@/components/phone-shell";
import { DeviceHub } from "./device-hub";
import { useDevices } from "@/lib/store";

export default function DevicePage({
  params,
}: {
  params: Promise<{ id: string }>;
}) {
  const { id } = use(params);
  const { ready, devices } = useDevices();
  const device = devices.find((d) => d.id === id);

  if (!ready) {
    return (
      <PhoneShell>
        <p className="p-8 text-sm text-muted-foreground">Loading…</p>
      </PhoneShell>
    );
  }

  if (!device) {
    return (
      <PhoneShell>
        <div className="flex flex-1 flex-col items-center justify-center gap-3 px-8 text-center">
          <p className="font-[family-name:var(--font-display)] text-xl font-bold">
            Unit not on this phone
          </p>
          <Link href="/" className="text-sm text-primary underline">
            Back to paired list
          </Link>
        </div>
      </PhoneShell>
    );
  }

  return <DeviceHub device={device} />;
}
