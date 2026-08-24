export function BrandMark({ size = 36 }: { size?: number }) {
  return (
    <svg
      width={size}
      height={size}
      viewBox="0 0 64 64"
      fill="none"
      aria-hidden
      className="drop-shadow-sm"
    >
      <defs>
        <linearGradient id="ag-g" x1="8" y1="8" x2="56" y2="56" gradientUnits="userSpaceOnUse">
          <stop stopColor="#5B8CFF" />
          <stop offset="0.55" stopColor="#3DDC97" />
          <stop offset="1" stopColor="#FFB020" />
        </linearGradient>
      </defs>
      <rect x="4" y="4" width="56" height="56" rx="18" fill="url(#ag-g)" opacity="0.95" />
      <path
        d="M20 38c4-10 10-16 12-18 2 2 8 8 12 18"
        stroke="white"
        strokeWidth="3.2"
        strokeLinecap="round"
        fill="none"
      />
      <circle cx="32" cy="40" r="4.5" fill="white" />
    </svg>
  );
}
