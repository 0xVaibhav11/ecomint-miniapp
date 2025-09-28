import { auth } from "@/auth";
import { Navigation } from "@/components/Navigation";
import { Page } from "@/components/PageLayout";

export default async function TabsLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  // const session = await auth();

  // If the user is not authenticated, redirect to the login page
  // if (!session) {
  //   console.log("Not authenticated");
  //   // redirect("/");
  // }

  return <Page>{children}</Page>;
}
