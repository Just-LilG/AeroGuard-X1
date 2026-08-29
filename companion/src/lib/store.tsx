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
  PLACES,
  STAGE_META,
  STAGE_ORDER,
  Stage,
  SmartVent,
  VentKind,
} from "./types";

const KEY = "aeroguard.devices.v1";
const DEMO_CYCLE: Stage[] = ["LOW", "MEDIUM", "CRITICAL", "FIRE"];
const ALERT_STAGES: Stage[] = ["MEDIUM", "CRITICAL", "FIRE"];

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
  };
}

function asStage(value: unknown): Stage {
  return STAGE_ORDER.includes(value as Stage) ? (value as Stage) : "SAFE";
}

function asPlace(value: unknown): PlaceKind {
  return PLACES.includes(value as PlaceKind) ? (value as PlaceKind) : "Home kitchen";
}

function normalizeVent(raw: unknown): SmartVent | null {
  if (!raw || typeof raw !== "object") return null;
  const v = raw as Record<string, unknown>;
  if (typeof v.id !== "string" || !v.id) return null;
  if (typeof v.name !== "string" || !v.name) return null;
  return {
    id: v.id,
    name: v.name,
    kind: v.kind === "window" ? "window" : "vent",
    open: Boolean(v.open),
    autoOnAlert: v.autoOnAlert !== false,
  };
}

function normalizeLog(raw: unknown): LogItem | null {
  if (!raw || typeof raw !== "object") return null;
  const item = raw as Record<string, unknown>;
  if (typeof item.id !== "string" || !item.id) return null;
  const stage =
    item.stage === "SYSTEM" ? "SYSTEM" : asStage(item.stage);
  return {
    id: item.id,
    at: typeof item.at === "number" && Number.isFinite(item.at) ? item.at : Date.now(),
    stage,
    title: typeof item.title === "string" ? item.title : "Event",
    detail: typeof item.detail === "string" ? item.detail : "",
  };
}

function normalizeDevice(raw: unknown): AeroDevice | null {
  if (!raw || typeof raw !== "object") return null;
  const d = raw as Record<string, unknown>;
  if (typeof d.id !== "string" || !d.id) return null;
  if (typeof d.serial !== "string" || !d.serial) return null;
  const vents = Array.isArray(d.vents)
    ? d.vents.map(normalizeVent).filter((v): v is SmartVent => v !== null)
    : [];
  const log = Array.isArray(d.log)
    ? d.log.map(normalizeLog).filter((item): item is LogItem => item !== null)
    : [];
  const baseline =
    typeof d.baseline === "number" && Number.isFinite(d.baseline) ? d.baseline : 310;
  const gasReading =
    typeof d.gasReading === "number" && Number.isFinite(d.gasReading)
      ? d.gasReading
      : baseline;
  return {
    id: d.id,
    serial: d.serial,
    bleName: typeof d.bleName === "string" && d.bleName ? d.bleName : "AeroGuard-X1",
    name: typeof d.name === "string" ? d.name : "",
    place: asPlace(d.place),
    ownerPhone: typeof d.ownerPhone === "string" ? d.ownerPhone : "",
    backupPhone: typeof d.backupPhone === "string" ? d.backupPhone : "",
    setupComplete: Boolean(d.setupComplete),
    stage: asStage(d.stage),
    demoMode: Boolean(d.demoMode),
    gasReading,
    baseline,
    vents,
    log,
    pairedAt:
      typeof d.pairedAt === "number" && Number.isFinite(d.pairedAt)
        ? d.pairedAt
        : Date.now(),
  };
}

export function parseStoredDevices(raw: string | null): AeroDevice[] {
  if (!raw) return [];
  try {
    const data = JSON.parse(raw);
    if (!Array.isArray(data)) return [];
    return data
      .map(normalizeDevice)
      .filter((d): d is AeroDevice => d !== null);
  } catch {
    return [];
  }
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
  addVent: (id: string, name: string, kind: VentKind) => boolean;
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

  const alertOpen = ALERT_STAGES.includes(stage);

  return {
    ...d,
    stage,
    demoMode: demo,
    gasReading: gas,
    vents: d.vents.map((v) => {
      if (!v.autoOnAlert) return v;
      if (alertOpen) return { ...v, open: true };
      if (stage === "SAFE") return { ...v, open: false };
      return v;
    }),
    log: [...extra, ...d.log].slice(0, 40),
  };
}

export function DeviceProvider({ children }: { children: React.ReactNode }) {
  const [ready, setReady] = useState(false);
  const [devices, setDevices] = useState<AeroDevice[]>([]);
  const devicesRef = useRef<AeroDevice[]>([]);

  const commit = useCallback((next: AeroDevice[]) => {
    devicesRef.current = next;
    setDevices(next);
  }, []);

  useEffect(() => {
    const loaded = parseStoredDevices(localStorage.getItem(KEY));
    commit(loaded);
    setReady(true);
  }, [commit]);

  useEffect(() => {
    if (!ready) return;
    localStorage.setItem(KEY, JSON.stringify(devices));
  }, [devices, ready]);

  const patch = useCallback(
    (id: string, fn: (d: AeroDevice) => AeroDevice) => {
      commit(devicesRef.current.map((d) => (d.id === id ? fn(d) : d)));
    },
    [commit]
  );

  const pair = useCallback(
    (serial: string, bleName: string) => {
      const found = devicesRef.current.find((d) => d.serial === serial);
      if (found) return found;
      const created = createDevice(serial, bleName);
      commit([created, ...devicesRef.current]);
      return created;
    },
    [commit]
  );

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
      patch(id, (d) => {
        const hadOpen = d.vents.some((v) => v.open);
        const next = applyStage(d, "SAFE", false);
        const extra: LogItem[] = [
          nowLog("SYSTEM", "Reset", "Alarm muted. Demo off. Recalibrating."),
        ];
        if (hadOpen) {
          extra.push(
            nowLog(
              "SYSTEM",
              "Vents closed",
              "Linked vents/windows sealed after reset."
            )
          );
        }
        return {
          ...next,
          demoMode: false,
          vents: next.vents.map((v) => ({ ...v, open: false })),
          log: [...extra, ...d.log].slice(0, 40),
        };
      });
    },
    [patch]
  );

  const addVent: Store["addVent"] = useCallback(
    (id, name, kind) => {
      const current = devicesRef.current.find((d) => d.id === id);
      if (!current) return false;
      if (
        current.vents.some((v) => v.name.toLowerCase() === name.toLowerCase())
      ) {
        return false;
      }
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
      return true;
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

  const unpair: Store["unpair"] = useCallback(
    (id) => {
      commit(devicesRef.current.filter((d) => d.id !== id));
    },
    [commit]
  );

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
