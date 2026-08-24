"use client";

import { STAGE_META, Stage } from "@/lib/types";

export function StatusDot({
  stage,
  size = "sm",
}: {
  stage: Stage;
  size?: "sm" | "md";
}) {
  const c = STAGE_META[stage].color;
  const dim = size === "md" ? "h-3.5 w-3.5" : "h-2.5 w-2.5";
  return (
    <span
      className={`inline-block rounded-full ${dim}`}
      style={{
        background: c,
        boxShadow: stage === "SAFE" ? "none" : `0 0 10px ${c}`,
      }}
    />
  );
}
