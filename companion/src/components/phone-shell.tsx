export function PhoneShell({ children }: { children: React.ReactNode }) {
  return (
    <div className="min-h-dvh bg-[#1a1814] md:flex md:items-center md:justify-center md:p-6">
      <div className="mx-auto flex h-dvh w-full max-w-[430px] flex-col overflow-hidden bg-[#0e0d0b] text-[#efe8d8] shadow-[0_0_0_1px_#3a3428] md:h-[min(860px,100dvh)] md:rounded-[2rem]">
        {children}
      </div>
    </div>
  );
}
