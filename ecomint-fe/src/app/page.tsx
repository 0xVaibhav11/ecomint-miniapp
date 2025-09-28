import { Page } from "@/components/PageLayout";
import { AuthButton } from "../components/AuthButton";
import Image from "next/image";
import { BrushCleaning, IndianRupee, Users } from "lucide-react";
import { MoneySquare } from "iconoir-react";

export default function Home() {
  return (
    <Page>
      <Page.Main className="flex flex-col items-center justify-center">
        <div className="flex flex-col w-full items-start justify-center">
          <Image src={"/ecomint.png"} alt="Ecomint" width={100} height={100} />
          <p className="text-4xl font-bold text-start mt-4">
            Help us make India clean
          </p>
        </div>

        <div className="flex flex-col w-full items-start justify-center">
          <div className="flex flex-col gap-6 mt-8 w-full max-w-xl">
            <div className="flex items-start gap-4">
              <div className="flex-shrink-0 rounded-full bg-gray-100 p-2">
                {/* Clean bin icon */}
                <BrushCleaning className="w-6 h-6" color="#6B7280" />
              </div>
              <div>
                <p className="text-lg font-medium text-gray-400">
                  Recycle the plastic and make India clean
                </p>
              </div>
            </div>
            <div className="flex items-start gap-4">
              <div className="flex-shrink-0 rounded-full bg-gray-100 p-2">
                {/* People icon */}
                <IndianRupee className="w-6 h-6" color="#6B7280" />
              </div>
              <div>
                <p className="text-lg font-medium text-gray-400">
                  Make India clean and earn money
                </p>
              </div>
            </div>
            <div className="flex items-start gap-4">
              <div className="flex-shrink-0 rounded-full bg-gray-100 p-2">
                {/* Globe icon */}
                <Users className="w-6 h-6" color="#6B7280" />
              </div>
              <div>
                <p className="text-lg font-medium text-gray-400">
                  Together we can make India a better place
                </p>
              </div>
            </div>
          </div>
        </div>
        <div className="flex flex-col w-full items-center justify-center mt-8">
          <AuthButton />
        </div>
      </Page.Main>
    </Page>
  );
}
