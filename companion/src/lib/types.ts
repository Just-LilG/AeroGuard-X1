export type Stage = "SAFE" | "LOW" | "MEDIUM" | "CRITICAL" | "FIRE";

export type PlaceKind =
  | "Home kitchen"
  | "Hostel kitchen"
  | "Chop bar"
  | "Cylinder store"
  | "Lab"
  | "Apartment";

export type VentKind = "vent" | "window";

export type DeviceTab = "status" | "activity" | "vents" | "device";

export interface SmartVent {
  id: string;
  name: string;
  kind: VentKind;
  open: boolean;
  autoOnAlert: boolean;
}

export interface LogItem {
  id: string;
  at: number;
  stage: Stage | "SYSTEM";
  title: string;
  detail: string;
}

export interface AeroDevice {
  id: string;
  serial: string;
  bleName: string;
  name: string;
  place: PlaceKind;
  ownerPhone: string;
  backupPhone: string;
  setupComplete: boolean;
  stage: Stage;
  demoMode: boolean;
  gasReading: number;
  baseline: number;
  vents: SmartVent[];
  log: LogItem[];
  pairedAt: number;
}

export interface NearbyUnit {
  serial: string;
  bleName: string;
  rssi: number;
  hint: string;
}

export const STAGE_ORDER: Stage[] = [
  "SAFE",
  "LOW",
  "MEDIUM",
  "CRITICAL",
  "FIRE",
];

export const STAGE_META: Record<
  Stage,
  { label: string; led: string; tone: string; color: string }
> = {
  SAFE: {
    label: "Safe",
    led: "Off",
    tone: "Monitoring",
    color: "#8a9a86",
  },
  LOW: {
    label: "Low",
    led: "Green",
    tone: "Quiet warning",
    color: "#3f8f4a",
  },
  MEDIUM: {
    label: "Medium",
    led: "Yellow",
    tone: "SMS + vents",
    color: "#d4a017",
  },
  CRITICAL: {
    label: "Critical",
    led: "Red",
    tone: "Call + SMS",
    color: "#c23b22",
  },
  FIRE: {
    label: "Fire",
    led: "Red",
    tone: "Evacuate",
    color: "#7a1515",
  },
};

export const PLACES: PlaceKind[] = [
  "Home kitchen",
  "Hostel kitchen",
  "Chop bar",
  "Cylinder store",
  "Lab",
  "Apartment",
];

export const NEARBY_POOL: NearbyUnit[] = [
  {
    serial: "AGX1-7A2C",
    bleName: "AeroGuard-X1",
    rssi: -41,
    hint: "Strong · likely this room",
  },
  {
    serial: "AGX1-B91E",
    bleName: "AeroGuard-X1",
    rssi: -67,
    hint: "Weaker · next room / corridor",
  },
  {
    serial: "AGX1-F004",
    bleName: "AeroGuard-X1",
    rssi: -78,
    hint: "Faint · farther unit",
  },
];
