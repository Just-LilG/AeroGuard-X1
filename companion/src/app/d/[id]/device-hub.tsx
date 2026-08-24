use client";

import { useMemo, useState } from "react";
import { useRouter } from "next/navigation";
import { PhoneShell } from "@/components/phone-shell";
import { StatusDot } from "@/components/status-dot";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { Switch } from "@/components/ui/switch";
import {
  AeroDevice,
  DeviceTab,
  PLACES,
  STAGE_META,
  VentKind,
} from "@/lib/types";
import { useDevices } from "@/lib/store";
import { Activity, ArrowLeft, Gauge, Settings2, Wind } from "lucide-react";

function formatWhen(ts: number) {
  return new Date(ts).toLocaleString(undefined, {
    hour: "2-digit",
    minute: "2-digit",
    day: "numeric",
    month: "short",
  });
}

export function DeviceHub({ device }: { device: AeroDevice }) {
  if (!device.setupComplete) return <SetupWizard device={device} />;
  return <PairedHub device={device} />;
}

function SetupWizard({ device }: { device: AeroDevice }) {
  const { completeSetup } = useDevices();
  const [name, setName] = useState("Kitchen");
  const [place, setPlace] = useState(device.place);
  const [owner, setOwner] = useState("");
  const [backup, setBackup] = useState("");
  const [err, setErr] = useState("");

  return (
    <PhoneShell>
      <header className="px-5 pb-3 pt-10">
        <p className="font-mono text-[10px] tracking-[0.2em] text-primary uppercase">
          First time · {device.serial}
        </p>
        <h1 className="font-[family-name:var(--font-display)] text-2xl font-extrabold">
          Name this AeroGuard
        </h1>
        <p className="mt-1 text-sm text-muted-foreground">
          Contacts live on the device, not in general app settings — so a hostel
          kitchen and a chop bar can alert different people.
        </p>
      </header>
      <form
        className="flex min-h-0 flex-1 flex-col"
        onSubmit={(e) => {
          e.preventDefault();
          const digits = owner.replace(/\D/g, "");
          if (!name.trim() || digits.length < 9) {
            setErr("Give it a name and a real owner number (at least 9 digits).");
            return;
          }
          completeSetup(device.id, {
            name: name.trim(),
            place,
            ownerPhone: owner.trim(),
            backupPhone: backup.trim(),
          });
        }}
      >
        <div className="min-h-0 flex-1 space-y-4 overflow-y-auto px-5 pb-3">
          <div className="space-y-1.5">
            <Label htmlFor="ag-name">Display name</Label>
            <Input
              id="ag-name"
              value={name}
              onChange={(e) => setName(e.target.value)}
              placeholder="Kitchen, Block A, Cylinder bay…"
              className="h-12 border-[#3a3428] bg-[#161410]"
            />
          </div>
          <div className="space-y-1.5">
            <Label>Where it sits</Label>
            <div className="grid grid-cols-2 gap-2">
              {PLACES.map((p) => (
                <button
                  key={p}
                  type="button"
                  onClick={() => setPlace(p)}
                  className={`min-h-11 rounded-xl px-3 text-sm ${
                    place === p
                      ? "bg-primary text-primary-foreground"
                      : "border border-[#3a3428] text-muted-foreground"
                  }`}
                >
                  {p}
                </button>
              ))}
            </div>
          </div>
          <div className="space-y-1.5">
            <Label htmlFor="ag-owner">Owner number (call + SMS)</Label>
            <Input
              id="ag-owner"
              name="owner"
              type="tel"
              inputMode="tel"
              autoComplete="tel"
              value={owner}
              onChange={(e) => setOwner(e.target.value)}
              placeholder="+233…"
              className="h-12 border-[#3a3428] bg-[#161410]"
            />
          </div>
          <div className="space-y-1.5">
            <Label htmlFor="ag-backup">Backup SMS (optional)</Label>
            <Input
              id="ag-backup"
              name="backup"
              type="tel"
              inputMode="tel"
              value={backup}
              onChange={(e) => setBackup(e.target.value)}
              placeholder="Roommate / warden"
              className="h-12 border-[#3a3428] bg-[#161410]"
            />
            <p className="text-[11px] text-muted-foreground">
              Backup gets a text if the alarm is still up after 3 minutes — not
              asked to walk into a leak.
            </p>
          </div>
          {err && <p className="text-sm text-destructive">{err}</p>}
        </div>
        <div className="border-t border-[#3a3428] bg-[#0e0d0b] p-4">
          <Button
            type="submit"
            className="h-12 w-full rounded-full font-semibold"
          >
            Save and open unit
          </Button>
        </div>
      </form>
    </PhoneShell>
  );
}

function PairedHub({ device }: { device: AeroDevice }) {
  const [tab, setTab] = useState<DeviceTab>("status");
  const router = useRouter();
  const meta = STAGE_META[device.stage];

  return (
    <PhoneShell>
      <header className="flex items-center gap-3 px-4 pb-2 pt-9">
        <button
          type="button"
          onClick={() => router.push("/")}
          className="flex h-10 w-10 items-center justify-center rounded-full border border-[#3a3428]"
        >
          <ArrowLeft className="h-4 w-4" />
        </button>
        <div className="min-w-0 flex-1">
          <p className="truncate font-[family-name:var(--font-display)] text-lg font-bold">
            {device.name}
          </p>
          <p className="font-mono text-[10px] text-muted-foreground">
            {device.serial} · {device.place}
          </p>
        </div>
        <StatusDot stage={device.stage} size="md" />
      </header>

      <div
        className="mx-4 mb-3 h-1 rounded-full"
        style={{ background: meta.color, opacity: 0.85 }}
      />

      <div className="min-h-0 flex-1 overflow-y-auto overscroll-contain px-4 pb-8">
        {tab === "status" && <StatusPane device={device} />}
        {tab === "activity" && <ActivityPane device={device} />}
        {tab === "vents" && <VentsPane device={device} />}
        {tab === "device" && <SettingsPane device={device} />}
      </div>

      <nav className="relative z-20 grid grid-cols-4 border-t border-[#3a3428] bg-[#120f0c] px-2 py-3">
        {(
          [
            ["status", "Status", Gauge],
            ["activity", "Activity", Activity],
            ["vents", "Vents", Wind],
            ["device", "Device", Settings2],
          ] as const
        ).map(([id, label, Icon]) => (
          <button
            key={id}
            type="button"
            onClick={() => setTab(id)}
            className={`flex h-12 flex-col items-center justify-center gap-1 rounded-xl px-1 text-[11px] ${
              tab === id
                ? "bg-primary/15 text-primary"
                : "text-muted-foreground"
            }`}
          >
            <Icon className="h-4 w-4" />
            {label}
          </button>
        ))}
      </nav>
    </PhoneShell>
  );
}

function StatusPane({ device }: { device: AeroDevice }) {
  const { advanceDemo, resetDevice } = useDevices();
  const meta = STAGE_META[device.stage];
  const pct = Math.min(
    100,
    Math.round(((device.gasReading - device.baseline) / device.baseline) * 100)
  );

  return (
    <div className="flex flex-col gap-4 pb-2">
      <button
        type="button"
        onClick={() => advanceDemo(device.id)}
        className="w-full rounded-[1.6rem] border border-[#3a3428] bg-[#161410] p-5 text-center"
      >
        <p className="font-mono text-[10px] tracking-[0.2em] text-muted-foreground uppercase">
          {device.demoMode ? "Demo sync · tap to advance" : "Tap to demo leak"}
        </p>
        <div
          className="mx-auto mt-4 flex h-36 w-36 items-center justify-center rounded-full border-[6px]"
          style={{ borderColor: meta.color }}
        >
          <div>
            <p
              className="font-[family-name:var(--font-display)] text-2xl font-extrabold"
              style={{ color: meta.color }}
            >
              {meta.label}
            </p>
            <p className="font-mono text-[10px] text-muted-foreground">
              LED {meta.led}
            </p>
          </div>
        </div>
        <p className="mt-3 text-sm text-muted-foreground">{meta.tone}</p>
        <p className="mt-4 font-mono text-xs text-muted-foreground">
          Gas {device.gasReading} · baseline {device.baseline} · {pct}% over
        </p>
      </button>

      <div className="relative z-10 grid grid-cols-2 gap-2">
        <button
          type="button"
          className="h-12 rounded-2xl bg-primary px-3 text-sm font-semibold text-primary-foreground"
          onClick={() => advanceDemo(device.id)}
        >
          Demo next stage
        </button>
        <button
          type="button"
          className="h-12 rounded-2xl border border-[#3a3428] px-3 text-sm font-semibold"
          onClick={() => resetDevice(device.id)}
        >
          Reset / mute
        </button>
      </div>
      <p className="text-[11px] leading-relaxed text-muted-foreground">
        Matches the hardware demo button: Low (green) → Medium (yellow, SMS +
        vents) → Critical (red, real call) → Fire. Reset clears demo the same
        way the box button does.
      </p>
    </div>
  );
}

function ActivityPane({ device }: { device: AeroDevice }) {
  return (
    <ul className="flex flex-col gap-2 pb-4">
      {device.log.map((item) => (
        <li
          key={item.id}
          className="rounded-2xl border border-[#3a3428] bg-[#161410] px-3 py-3"
        >
          <div className="flex items-baseline justify-between gap-2">
            <p className="text-sm font-semibold">{item.title}</p>
            <p className="shrink-0 font-mono text-[10px] text-muted-foreground">
              {formatWhen(item.at)}
            </p>
          </div>
          <p className="mt-1 text-xs text-muted-foreground">{item.detail}</p>
        </li>
      ))}
    </ul>
  );
}

function VentsPane({ device }: { device: AeroDevice }) {
  const { addVent, toggleVent, removeVent } = useDevices();
  const [name, setName] = useState("");
  const [kind, setKind] = useState<VentKind>("vent");

  return (
    <div className="flex flex-col gap-3 pb-4">
      <p className="text-sm text-muted-foreground">
        Link smart vents or windows to this AeroGuard. At Medium and above the
        app asks them to open together — nothing motorized on the box.
      </p>
      <div className="rounded-2xl border border-[#3a3428] bg-[#161410] p-3">
        <Label>Add to this unit</Label>
        <Input
          value={name}
          onChange={(e) => setName(e.target.value)}
          placeholder="Kitchen window, extractor…"
          className="mt-2 h-10 border-[#3a3428] bg-[#0e0d0b]"
        />
        <div className="mt-2 flex gap-2">
          {(["vent", "window"] as const).map((k) => (
            <button
              key={k}
              type="button"
              onClick={() => setKind(k)}
              className={`rounded-full px-3 py-1 text-xs capitalize ${
                kind === k
                  ? "bg-primary text-primary-foreground"
                  : "border border-[#3a3428]"
              }`}
            >
              {k}
            </button>
          ))}
          <Button
            size="sm"
            className="ml-auto rounded-full"
            onClick={() => {
              if (!name.trim()) return;
              addVent(device.id, name.trim(), kind);
              setName("");
            }}
          >
            Link
          </Button>
        </div>
      </div>
      {device.vents.length === 0 ? (
        <p className="rounded-2xl border border-dashed border-[#3a3428] px-3 py-8 text-center text-sm text-muted-foreground">
          No vents linked yet.
        </p>
      ) : (
        device.vents.map((v) => (
          <div
            key={v.id}
            className="flex items-center gap-3 rounded-2xl border border-[#3a3428] bg-[#161410] px-3 py-3"
          >
            <div className="min-w-0 flex-1">
              <p className="font-medium">{v.name}</p>
              <p className="font-mono text-[10px] text-muted-foreground">
                {v.kind} · auto-open on Medium+ {v.autoOnAlert ? "on" : "off"}
              </p>
            </div>
            <div className="flex items-center gap-2">
              <span className="text-[10px] text-muted-foreground">
                {v.open ? "Open" : "Shut"}
              </span>
              <Switch
                checked={v.open}
                onCheckedChange={() => toggleVent(device.id, v.id)}
              />
            </div>
            <button
              type="button"
              className="text-[10px] text-destructive"
              onClick={() => removeVent(device.id, v.id)}
            >
              Remove
            </button>
          </div>
        ))
      )}
    </div>
  );
}

function SettingsPane({ device }: { device: AeroDevice }) {
  const { updateContacts, rename, unpair } = useDevices();
  const router = useRouter();
  const [name, setName] = useState(device.name);
  const [place, setPlace] = useState(device.place);
  const [owner, setOwner] = useState(device.ownerPhone);
  const [backup, setBackup] = useState(device.backupPhone);

  const savedHint = useMemo(() => "", []);

  return (
    <div className="flex flex-col gap-4 pb-6">
      <section className="space-y-2">
        <p className="font-mono text-[10px] tracking-widest text-primary uppercase">
          This AeroGuard
        </p>
        <Input
          value={name}
          onChange={(e) => setName(e.target.value)}
          className="h-11 border-[#3a3428] bg-[#161410]"
        />
        <div className="flex flex-wrap gap-1.5">
          {PLACES.map((p) => (
            <button
              key={p}
              type="button"
              onClick={() => setPlace(p)}
              className={`rounded-full px-2.5 py-1 text-[11px] ${
                place === p
                  ? "bg-primary text-primary-foreground"
                  : "border border-[#3a3428] text-muted-foreground"
              }`}
            >
              {p}
            </button>
          ))}
        </div>
        <Button
          variant="secondary"
          className="w-full"
          onClick={() => rename(device.id, name.trim() || device.name, place)}
        >
          Save name & place
        </Button>
      </section>

      <section className="space-y-2">
        <p className="font-mono text-[10px] tracking-widest text-primary uppercase">
          Alert numbers
        </p>
        <Label>Owner</Label>
        <Input
          value={owner}
          onChange={(e) => setOwner(e.target.value)}
          className="h-11 border-[#3a3428] bg-[#161410]"
        />
        <Label>Backup SMS</Label>
        <Input
          value={backup}
          onChange={(e) => setBackup(e.target.value)}
          className="h-11 border-[#3a3428] bg-[#161410]"
        />
        <Button
          className="w-full"
          onClick={() => updateContacts(device.id, owner.trim(), backup.trim())}
        >
          Save numbers
        </Button>
        <p className="text-[11px] text-muted-foreground">
          These should match the numbers in the Arduino sketch for demo day.
        </p>
      </section>

      <section className="rounded-2xl border border-[#3a3428] bg-[#161410] p-3 text-xs text-muted-foreground">
        <p className="font-medium text-foreground">Link</p>
        <p className="mt-1">
          BLE (HM-10) is reserved on A1/A3. This contest build uses Demo sync.
          GSM calls still leave the SIM800L on the box.
        </p>
      </section>

      <Button
        variant="destructive"
        className="h-12 rounded-full"
        onClick={() => {
          if (confirm(`Unpair ${device.name}?`)) {
            unpair(device.id);
            router.push("/");
          }
        }}
      >
        Unpair this AeroGuard
      </Button>
      <p className="hidden">{savedHint}</p>
    </div>
  );
}
