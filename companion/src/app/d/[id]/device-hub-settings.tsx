"use client";

import { useState } from "react";
import { Check, Flame, Phone, Radio, Settings2, Trash2 } from "lucide-react";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { PLACES, type AeroDevice, type PlaceKind } from "@/lib/types";

export function DeviceTab({
  device,
  onRename,
  onContacts,
  onUnpair,
}: {
  device: AeroDevice;
  onRename: (name: string, place: PlaceKind) => void;
  onContacts: (owner: string, backup: string) => void;
  onUnpair: () => void;
}) {
  const [name, setName] = useState(device.name);
  const [place, setPlace] = useState<PlaceKind>(device.place);
  const [owner, setOwner] = useState(device.ownerPhone);
  const [backup, setBackup] = useState(device.backupPhone);
  const [confirmRemove, setConfirmRemove] = useState(false);

  return (
    <div className="fade-up space-y-4">
      <div className="glass rounded-[1.5rem] p-4">
        <div className="flex items-center gap-3">
          <div className="flex h-12 w-12 items-center justify-center rounded-2xl bg-primary/15 text-primary">
            <Settings2 className="h-6 w-6" />
          </div>
          <div>
            <p className="text-[15px] font-semibold">This AeroGuard only</p>
            <p className="text-[13px] text-muted-foreground">
              Name, place, and alert phones stay on this device.
            </p>
          </div>
        </div>
      </div>

      <div className="glass space-y-3 rounded-[1.5rem] p-4">
        <div>
          <Label htmlFor="d-name">Device name</Label>
          <Input
            id="d-name"
            className="mt-2 h-12 rounded-2xl"
            value={name}
            onChange={(e) => setName(e.target.value)}
          />
        </div>
        <div>
          <Label>Place type</Label>
          <div className="mt-2 grid grid-cols-2 gap-2">
            {PLACES.map((p) => (
              <button
                key={p}
                type="button"
                onClick={() => setPlace(p)}
                className={`rounded-2xl px-3 py-3 text-left text-[13px] font-medium ${
                  place === p
                    ? "bg-primary text-primary-foreground"
                    : "bg-foreground/[0.04] dark:bg-white/[0.06]"
                }`}
              >
                {p}
              </button>
            ))}
          </div>
        </div>
        <Button
          type="button"
          className="h-11 w-full rounded-2xl"
          onClick={() => onRename(name.trim() || device.name, place)}
        >
          Save name & place
        </Button>
      </div>

      <div className="glass space-y-3 rounded-[1.5rem] p-4">
        <p className="text-[15px] font-semibold">Alert contacts</p>
        <div>
          <Label htmlFor="d-owner">Owner phone</Label>
          <Input
            id="d-owner"
            className="mt-2 h-12 rounded-2xl"
            value={owner}
            onChange={(e) => setOwner(e.target.value)}
          />
        </div>
        <div>
          <Label htmlFor="d-backup">Backup phone</Label>
          <Input
            id="d-backup"
            className="mt-2 h-12 rounded-2xl"
            value={backup}
            onChange={(e) => setBackup(e.target.value)}
          />
          <p className="mt-1.5 text-[12px] text-muted-foreground">
            SMS only after 3 minutes if the owner has not cleared the alert.
          </p>
        </div>
        <Button
          type="button"
          variant="secondary"
          className="h-11 w-full rounded-2xl"
          onClick={() => onContacts(owner.trim(), backup.trim())}
        >
          Save contacts
        </Button>
      </div>

      <div className="glass rounded-[1.35rem] px-4 py-1">
        <InfoRow icon={Radio} label="Link" value="Bluetooth (contest sim)" />
        <InfoRow icon={Phone} label="GSM" value="SIM800L path" />
        <InfoRow icon={Flame} label="Sensors" value="Gas + flame" />
        <InfoRow icon={Check} label="Firmware" value="AeroGuard-X1 v1" last />
      </div>

      {!confirmRemove ? (
        <Button
          type="button"
          variant="destructive"
          className="h-12 w-full rounded-2xl"
          onClick={() => setConfirmRemove(true)}
        >
          <Trash2 className="mr-2 h-4 w-4" />
          Remove from Home
        </Button>
      ) : (
        <div className="glass space-y-3 rounded-[1.5rem] p-4">
          <p className="text-[14px] text-muted-foreground">
            Remove {device.name}? You can pair again later.
          </p>
          <div className="grid grid-cols-2 gap-2">
            <Button
              type="button"
              variant="secondary"
              className="h-11 rounded-2xl"
              onClick={() => setConfirmRemove(false)}
            >
              Cancel
            </Button>
            <Button
              type="button"
              variant="destructive"
              className="h-11 rounded-2xl"
              onClick={onUnpair}
            >
              Remove
            </Button>
          </div>
        </div>
      )}
    </div>
  );
}

function InfoRow({
  icon: Icon,
  label,
  value,
  last,
}: {
  icon: typeof Radio;
  label: string;
  value: string;
  last?: boolean;
}) {
  return (
    <div
      className={`flex items-center gap-3 py-3 ${
        last ? "" : "border-b border-border/60"
      }`}
    >
      <Icon className="h-4 w-4 text-muted-foreground" />
      <span className="flex-1 text-[14px]">{label}</span>
      <span className="text-[13px] text-muted-foreground">{value}</span>
    </div>
  );
}
