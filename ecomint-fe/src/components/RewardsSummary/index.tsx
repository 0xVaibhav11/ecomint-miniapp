"use client";

import { useEffect, useRef, useState } from "react";
import { Coins, Recycle } from "lucide-react";

type Props = {
  coins?: number; // total coins earned
  bottles?: number; // bottles collected
};

export const RewardsSummary = ({ coins = 12.5, bottles = 37 }: Props) => {
  const [coinsDisplay, setCoinsDisplay] = useState(0);
  const [bottlesDisplay, setBottlesDisplay] = useState(0);
  const rafRef = useRef<number | null>(null);

  useEffect(() => {
    const durationMs = 600;
    const start = performance.now();

    const tick = (now: number) => {
      const t = Math.min(1, (now - start) / durationMs);
      // Ease-out cubic
      const eased = 1 - Math.pow(1 - t, 3);
      setCoinsDisplay(coins * eased);
      setBottlesDisplay(Math.round(bottles * eased));
      if (t < 1) rafRef.current = requestAnimationFrame(tick);
    };

    rafRef.current = requestAnimationFrame(tick);
    return () => {
      if (rafRef.current) cancelAnimationFrame(rafRef.current);
    };
  }, [coins, bottles]);

  return (
    <div className="w-full">
      <div className="relative overflow-hidden rounded-2xl p-4 text-white bg-gradient-to-br from-blue-700 via-indigo-600 to-fuchsia-600">
        <div className="absolute -right-8 -top-8 h-32 w-32 rounded-full bg-white/10 blur-2xl" />
        <div className="absolute left-6 top-6 h-16 w-16 rounded-full bg-lime-300/30 blur-xl" />

        <div className="flex items-center justify-between">
          <p className="text-base font-semibold tracking-wide">Your Rewards</p>
        </div>

        <div className="mt-4 grid grid-cols-2 gap-3">
          <div className="rounded-xl bg-white/10 backdrop-blur-sm p-3 flex items-center gap-3">
            <div className="shrink-0 rounded-lg bg-white/15 p-2">
              <Coins className="text-yellow-300" size={22} />
            </div>
            <div className="leading-tight">
              <div className="text-xs opacity-80">Coins earned</div>
              <div className="text-2xl font-bold">
                {coinsDisplay.toFixed(2)}
              </div>
            </div>
          </div>

          <div className="rounded-xl bg-white/10 backdrop-blur-sm p-3 flex items-center gap-3">
            <div className="shrink-0 rounded-lg bg-white/15 p-2">
              <Recycle className="text-lime-300" size={22} />
            </div>
            <div className="leading-tight">
              <div className="text-xs opacity-80">Bottles collected</div>
              <div className="text-2xl font-bold">{bottlesDisplay}</div>
            </div>
          </div>
        </div>

        <div className="mt-4 h-2 w-full overflow-hidden rounded-full bg-white/20">
          <div
            className="h-full bg-lime-300"
            style={{ width: `${Math.min(100, (bottles / 50) * 100)}%` }}
          />
        </div>
        <div className="mt-1 text-[10px] opacity-80">
          {Math.min(50, bottlesDisplay)} / 50 bottles this month
        </div>
      </div>
    </div>
  );
};

export default RewardsSummary;
