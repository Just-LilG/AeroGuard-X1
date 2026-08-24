export function PhoneShell({ children }: { children: React.ReactNode }) {
  return (
    <div className="min-h-dvh bg-[#1a1814] md:flex md:items-center md:justify-center md:p-8">
      <div className="mx-auto flex min-h-dvh w-full max-w-[430px] flex-col bg-[#0e0d0b] text-[#efe8d8] shadow-[0_0_0_1px_#3a3428] md:min-h-[780px] md:max-h-[860px] md:overflow-hidden md:rounded-[2rem]">
        {children}
      </div>
    </div>
  );
}
