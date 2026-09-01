"use client";

import type { CSSProperties } from "react";
import { STAGE_META, type Stage } from "@/lib/types";

const STAGE_PCT: Record<Stage, number> = {
  SAFE: 8,
  LOW: 28,
  MEDIUM: 52,
  CRITICAL: 78,
  FIRE: 100,
};

export function StatusRing({
  stage,
  size = 168,
  onClick,
  subtitle,
}: {
  stage: Stage;
  size?: number;
  onClick?: () => void;
  subtitle?: string;
}) {
  const meta = STAGE_META[stage];
  const pct = STAGE_PCT[stage];
  const r = 54;
  const c = 2 * Math.PI * r;
  const dash = (pct / 100) * c;
  const style = {
    width: size,
    height: size,
    ["--ring-color" as string]: meta.color,
  } as CSSProperties;
  const className = `relative mx-auto flex items-center justify-center stage-pop ${onClick ? "pressable" : ""}`;

  const inner = (
    <>
      <svg
        width={size}
        height={size}
        viewBox="0 0 140 140"
        className="absolute inset-0 -rotate-90"
      >
        <circle
          cx="70"
          cy="70"
          r={r}
          fill="none"
          stroke="currentColor"
          strokeWidth="10"
          className="text-foreground/10"
        />
        <circle
          cx="70"
          cy="70"
          r={r}
          fill="none"
          stroke={meta.color}
          strokeWidth="10"
          strokeLinecap="round"
          strokeDasharray={`${dash} ${c}`}
          className="transition-all duration-500 ease-out"
        />
      </svg>
      <div
        className={`glass absolute inset-[18%] flex flex-col items-center justify-center rounded-full ${stage !== "SAFE" ? "pulse-ring" : ""}`}
        style={{ ["--ring-color" as string]: meta.color } as CSSProperties}
      >
        <p
          className="font-[family-name:var(--font-display)] text-2xl font-bold tracking-tight"
          style={{ color: meta.color }}
        >
          {meta.label}
        </p>
        <p className="mt-0.5 max-w-[6.5rem] px-1 text-center font-mono text-[10px] leading-tight text-muted-foreground">
          {subtitle ?? `LED ${meta.led}`}
        </p>
      </div>
    </>
  );

  if (onClick) {
    return (
      <button
        type="button"
        key={stage}
        onClick={onClick}
        className={className}
        style={style}
      >
        {inner}
      </button>
    );
  }

  return (
    <div key={stage} className={className} style={style}>
      {inner}
    </div>
  );
}
