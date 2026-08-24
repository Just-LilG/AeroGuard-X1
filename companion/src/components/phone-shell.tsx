"use client";

export function PhoneShell({ children }: { children: React.ReactNode }) {
  return (
    <div className="min-h-dvh bg-[var(--shell-outer)] md:flex md:items-center md:justify-center md:p-6">
      <div className="phone-aurora relative mx-auto flex h-dvh w-full max-w-[430px] flex-col overflow-hidden text-foreground md:h-[min(860px,100dvh)] md:rounded-[2.2rem] md:shadow-[0_30px_80px_rgba(15,23,42,0.25)] md:ring-1 md:ring-white/30 dark:md:ring-white/10">
        {children}
      </div>
    </div>
  );
}
