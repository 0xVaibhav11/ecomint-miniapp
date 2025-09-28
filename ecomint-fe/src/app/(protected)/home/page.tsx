import { auth } from "@/auth";
import { Page } from "@/components/PageLayout";
import { Pay } from "@/components/Pay";
import { ScanReward } from "@/components/ScanReward";
import { RewardsSummary } from "@/components/RewardsSummary";
import { Transaction } from "@/components/Transaction";
import { UserInfo } from "@/components/UserInfo";
import { Verify } from "@/components/Verify";
import { ViewPermissions } from "@/components/ViewPermissions";
import { Marble, TopBar } from "@worldcoin/mini-apps-ui-kit-react";

export default async function Home() {
  const session = await auth();

  return (
    <>
      <Page.Main className="flex flex-col items-center justify-start gap-4 mb-16">
        <UserInfo />
        <ScanReward />
        <RewardsSummary coins={0.6} bottles={6} />
      </Page.Main>
    </>
  );
}
