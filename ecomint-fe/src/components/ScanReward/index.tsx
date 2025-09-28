"use client";

import { Button } from "@worldcoin/mini-apps-ui-kit-react";
import { Scanner as QrScanner } from "@yudiel/react-qr-scanner";
import { QrCode, Xmark } from "iconoir-react";
import { useCallback, useMemo, useRef, useState } from "react";

type Props = {
  onDecoded?: (text: string) => void;
};

export const ScanReward = (props: Props) => {
  const [isOpen, setIsOpen] = useState(false);
  const [lastResult, setLastResult] = useState<string>("");
  const isDecodingRef = useRef(false);

  const handleDecoded = useCallback(
    (text: string) => {
      if (!text || isDecodingRef.current) return;
      isDecodingRef.current = true;
      setLastResult(text);
      props.onDecoded?.(text);
      setTimeout(() => {
        setIsOpen(false);
        isDecodingRef.current = false;
      }, 250);
    },
    [props]
  );

  const constraints = useMemo<MediaTrackConstraints>(
    () => ({ facingMode: { ideal: "environment" }, aspectRatio: 1 }),
    []
  );

  return (
    <div className="w-full grid gap-3">
      <p className="text-lg font-semibold">Claim Reward</p>

      <button
        onClick={() => setIsOpen(true)}
        className="relative w-full h-60 rounded-2xl overflow-hidden"
      >
        <div className="absolute inset-0 bg-gradient-to-b from-indigo-600 to-blue-700" />
        <div className="relative z-10 flex flex-col items-center justify-center h-full gap-3 text-white">
          <div className="rounded-2xl border-4 border-lime-300/90 p-4 bg-white/10">
            <QrCode width={72} height={72} />
          </div>
          <span className="text-xl font-semibold">Tap to scan</span>
          <span className="text-xs opacity-80">Scan QR to claim</span>
        </div>
      </button>

      {lastResult && (
        <div className="text-xs text-gray-500 break-all">
          Last: {lastResult}
        </div>
      )}

      {isOpen && (
        <div className="fixed inset-0 z-50 flex flex-col bg-black/90">
          <div className="flex items-center justify-between p-3">
            <Button
              size="lg"
              variant="tertiary"
              className="!px-3"
              onClick={() => setIsOpen(false)}
            >
              <Xmark />
            </Button>
            <p className="text-white font-semibold">Scan QR</p>
            <div className="w-[40px]" />
          </div>
          <div className="flex-1 flex items-center justify-center p-4">
            <div className="relative w-[92vw] max-w-[600px] aspect-square rounded-2xl overflow-hidden">
              <QrScanner
                onScan={(result) => handleDecoded(result[0].rawValue)}
                onError={(err: any) => console.error("QR error", err)}
                constraints={constraints}
                components={{
                  torch: true,
                  finder: true,
                }}
                styles={{
                  container: { width: "100%", height: "100%" },
                  video: { objectFit: "cover", width: "100%", height: "100%" },
                }}
              />
              <div className="pointer-events-none absolute inset-0 ring-2 ring-lime-300 rounded-2xl" />
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default ScanReward;
