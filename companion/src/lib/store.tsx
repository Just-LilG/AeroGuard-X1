"use client";

import {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useRef,
  useState,
} from "react";
import {
  AeroDevice,
  LogItem,
  PlaceKind,
  STAGE_META,
  Stage,
  VentKind,
} from "./types";

const KEY = "aeroguard.devices.v1";
const DEMO_CYCLE: Stage[] = ["LOW", "MEDIUM", "CRITICAL", "FIRE"];
const CALIBRATE_MS = 2200;

function stripCalibrating(devices: AeroDevice[]): AeroDevice[] {
  return devices.map((d) =>
    d.calibrating ? { ...d, calibrating: false } : d
  );
}

function uid(prefix: string) {
  return `${prefix}-${Math.random().toString(36).slice(2, 8)}`;
}

function nowLog(
  stage: LogItem["stage"],
  title: string,
  detail: string
): LogItem {
  return { id: uid("log"), at: Date.now(), stage, title, detail };
}

function createDevice(serial: string, bleName: string): AeroDevice {
  return {
    id: uid("dev"),
    serial,
    bleName,
    name: "",
    place: "Home kitchen",
    ownerPhone: "",
    backupPhone: "",
    setupComplete: false,
    stage: "SAFE",
    demoMode: false,
    gasReading: 312,
    baseline: 310,
    vents: [],
    log: [
      nowLog("SYSTEM", "Paired", `${bleName} ${serial} linked to this phone.`),
    ],
    pairedAt: Date.now(),
    calibrating: false,
  };
}

type Store = {
  ready: boolean;
  devices: AeroDevice[];
  pair: (serial: string, bleName: string) => AeroDevice;
  completeSetup: (
    id: string,
    data: {
      name: string;
      place: PlaceKind;
      ownerPhone: string;
      backupPhone: string;
    }
  ) => void;
  advanceDemo: (id: string) => void;
  resetDevice: (id: string) => void;
  addVent: (id: string, name: string, kind: VentKind) => void;
  toggleVent: (id: string, ventId: string) => void;
  removeVent: (id: string, ventId: string) => void;
  updateContacts: (id: string, ownerPhone: string, backupPhone: string) => void;
  rename: (id: string, name: string, place: PlaceKind) => void;
  unpair: (id: string) => void;
};

const Ctx = createContext<Store | null>(null);

function applyStage(d: AeroDevice, stage: Stage, demo: boolean): AeroDevice {
  const meta = STAGE_META[stage];
  const extra: LogItem[] = [];
  extra.push(
    nowLog(stage, `Stage → ${meta.label}`, `${meta.led} LED · ${meta.tone}`)
  );
  if (stage === "MEDIUM") {
    extra.push(
      nowLog(
        stage,
        "Owner SMS",
        `SIM800L text to ${d.ownerPhone || "owner number"}.`
      )
    );
    extra.push(
      nowLog(stage, "Vent intent", "APP_CMD:VENT_OPEN → linked vents/windows.")
    );
  }
  if (stage === "CRITICAL" || stage === "FIRE") {
    extra.push(
      nowLog(
        stage,
        "Owner call + SMS",
        `GSM ringing ${d.ownerPhone || "owner number"}.`
      )
    );
  }

  const gas =
    stage === "SAFE"
      ? d.baseline
      : stage === "LOW"
        ? Math.round(d.baseline * 1.22)
        : stage === "MEDIUM"
          ? Math.round(d.baseline * 1.45)
          : stage === "CRITICAL"
            ? Math.round(d.baseline * 1.78)
            : Math.round(d.baseline * 1.9);

  return {
    ...d,
    stage,
    demoMode: demo,
    gasReading: gas,
    vents: d.vents.map((v) =>
      v.autoOnAlert &&
      (stage === "MEDIUM" || stage === "CRITICAL" || stage === "FIRE")
        ? { ...v, open: true }
        : v
    ),
    log: [...extra, ...d.log].slice(0, 40),
  };
}

export function DeviceProvider({ children }: { children: React.ReactNode }) {
  const [ready, setReady] = useState(false);
  const [devices, setDevices] = useState<AeroDevice[]>([]);
  const persist = useRef(false);

  useEffect(() => {
    try {
      const raw = localStorage.getItem(KEY);
      if (raw) {
        const parsed = JSON.parse(raw);
        setDevices(Array.isArray(parsed) ? stripCalibrating(parsed) : []);
      }
    } catch {
      /* ignore */
    }
    persist.current = true;
    setReady(true);
  }, []);

  useEffect(() => {
    if (!ready || !persist.current) return;
    localStorage.setItem(KEY, JSON.stringify(stripCalibrating(devices)));
  }, [devices, ready]);

  const patch = useCallback((id: string, fn: (d: AeroDevice) => AeroDevice) => {
    setDevices((prev) => prev.map((d) => (d.id === id ? fn(d) : d)));
  }, []);

  const pair = useCallback((serial: string, bleName: string) => {
    let created = createDevice(serial, bleName);
    setDevices((prev) => {
      const found = prev.find((d) => d.serial === serial);
      if (found) {
        created = found;
        return prev;
      }
      return [created, ...prev];
    });
    return created;
  }, []);

  const completeSetup: Store["completeSetup"] = useCallback(
    (id, data) => {
      patch(id, (d) => ({
        ...d,
        ...data,
        setupComplete: true,
        log: [
          nowLog(
            "SYSTEM",
            "Setup complete",
            `${data.name} · ${data.place} · ${data.ownerPhone}`
          ),
          ...d.log,
        ],
      }));
    },
    [patch]
  );

  const advanceDemo: Store["advanceDemo"] = useCallback(
    (id) => {
      patch(id, (d) => {
        const i = DEMO_CYCLE.indexOf(d.stage);
        const next = i < 0 ? "LOW" : DEMO_CYCLE[(i + 1) % DEMO_CYCLE.length];
        return applyStage(d, next, true);
      });
    },
    [patch]
  );

  const resetDevice: Store["resetDevice"] = useCallback(
    (id) => {
      patch(id, (d) => ({
        ...applyStage(d, "SAFE", false),
        demoMode: false,
        calibrating: true,
        log: [
          nowLog("SYSTEM", "Reset", "Alarm muted. Demo off. Recalibrating."),
          ...d.log,
        ],
      }));
      window.setTimeout(() => {
        patch(id, (d) => ({ ...d, calibrating: false }));
      }, CALIBRATE_MS);
    },
    [patch]
  );

  const addVent: Store["addVent"] = useCallback(
    (id, name, kind) => {
      patch(id, (d) => ({
        ...d,
        vents: [
          {
            id: uid("vent"),
            name,
            kind,
            open: false,
            autoOnAlert: true,
          },
          ...d.vents,
        ],
        log: [nowLog("SYSTEM", `Linked ${kind}`, name), ...d.log],
      }));
    },
    [patch]
  );

  const toggleVent: Store["toggleVent"] = useCallback(
    (id, ventId) => {
      patch(id, (d) => ({
        ...d,
        vents: d.vents.map((v) =>
          v.id === ventId ? { ...v, open: !v.open } : v
        ),
      }));
    },
    [patch]
  );

  const removeVent: Store["removeVent"] = useCallback(
    (id, ventId) => {
      patch(id, (d) => ({
        ...d,
        vents: d.vents.filter((v) => v.id !== ventId),
      }));
    },
    [patch]
  );

  const updateContacts: Store["updateContacts"] = useCallback(
    (id, ownerPhone, backupPhone) => {
      patch(id, (d) => ({ ...d, ownerPhone, backupPhone }));
    },
    [patch]
  );

  const rename: Store["rename"] = useCallback(
    (id, name, place) => {
      patch(id, (d) => ({ ...d, name, place }));
    },
    [patch]
  );

  const unpair: Store["unpair"] = useCallback((id) => {
    setDevices((prev) => prev.filter((d) => d.id !== id));
  }, []);

  const value = useMemo(
    () => ({
      ready,
      devices,
      pair,
      completeSetup,
      advanceDemo,
      resetDevice,
      addVent,
      toggleVent,
      removeVent,
      updateContacts,
      rename,
      unpair,
    }),
    [
      ready,
      devices,
      pair,
      completeSetup,
      advanceDemo,
      resetDevice,
      addVent,
      toggleVent,
      removeVent,
      updateContacts,
      rename,
      unpair,
    ]
  );

  return <Ctx.Provider value={value}>{children}</Ctx.Provider>;
}

export function useDevices() {
  const ctx = useContext(Ctx);
  if (!ctx) throw new Error("useDevices outside provider");
  return ctx;
}
