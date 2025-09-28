import { Elysia, t } from "elysia";
import {
  createPublicClient,
  createWalletClient,
  http,
  Hex,
  Address,
} from "viem";
import { privateKeyToAccount } from "viem/accounts";
import { z } from "zod";
import abiJson from "@/abi/BottleTracker.sol/BottleTracker.json";

// --- Config ---
const RPC_URL = Bun.env.RPC_URL || "http://localhost:8545";
const CONTRACT_ADDRESS = (Bun.env.BOTTLE_TRACKER_ADDRESS ||
  "0x0000000000000000000000000000000000000000") as Address;
const PRIVATE_KEY = Bun.env.PRIVATE_KEY as Hex | undefined;

const abi = (abiJson as any).abi;

// viem clients
const publicClient = createPublicClient({ transport: http(RPC_URL) });
const walletClient = PRIVATE_KEY
  ? createWalletClient({
      account: privateKeyToAccount(PRIVATE_KEY),
      transport: http(RPC_URL),
    })
  : null;

function requireSigner() {
  if (!walletClient) throw new Error("Signer not configured. Set PRIVATE_KEY.");
  return walletClient;
}

const app = new Elysia()
  .get("/", () => ({ ok: true }))
  // --- Reads ---
  .get("/bottles/address/:user", async ({ params }) => {
    const user = params.user as Address;
    const count = await publicClient.readContract({
      abi,
      address: CONTRACT_ADDRESS,
      functionName: "getBottleCount",
      args: [user],
    });
    return { user, count };
  })
  .get("/bottles/id/:userId", async ({ params }) => {
    const userId = params.userId as Hex; // bytes32 hex string
    const count = await publicClient.readContract({
      abi,
      address: CONTRACT_ADDRESS,
      functionName: "getBottleCountById",
      args: [userId],
    });
    return { userId, count };
  })
  .get("/total", async () => {
    const total = await publicClient.readContract({
      abi,
      address: CONTRACT_ADDRESS,
      functionName: "totalBottlesCollected",
      args: [],
    });
    return { total };
  })
  .get("/owner", async () => {
    const owner = await publicClient.readContract({
      abi,
      address: CONTRACT_ADDRESS,
      functionName: "owner",
      args: [],
    });
    return { owner };
  })
  .get("/roles/verifier/:addr", async ({ params }) => {
    const addr = params.addr as Address;
    const isVerifier = await publicClient.readContract({
      abi,
      address: CONTRACT_ADDRESS,
      functionName: "isVerifier",
      args: [addr],
    });
    return { address: addr, isVerifier };
  })
  .get("/roles/machine/:addr", async ({ params }) => {
    const addr = params.addr as Address;
    const isMachine = await publicClient.readContract({
      abi,
      address: CONTRACT_ADDRESS,
      functionName: "isMachine",
      args: [addr],
    });
    return { address: addr, isMachine };
  })
  // --- Admin/Verifier writes ---
  .post(
    "/admin/set-verifier",
    async ({ body, set }) => {
      const schema = z.object({ verifier: z.string(), active: z.boolean() });
      const parsed = schema.safeParse(body);
      if (!parsed.success) {
        set.status = 400;
        return parsed.error.flatten();
      }
      const { verifier, active } = parsed.data;
      const hash = await requireSigner().writeContract({
        abi,
        address: CONTRACT_ADDRESS,
        functionName: "setVerifier",
        args: [verifier as Address, active],
        chain: null,
      });
      return { hash };
    },
    {
      body: t.Object({ verifier: t.String(), active: t.Boolean() }),
    }
  )
  .post(
    "/admin/set-machine",
    async ({ body, set }) => {
      const schema = z.object({ machine: z.string(), active: z.boolean() });
      const parsed = schema.safeParse(body);
      if (!parsed.success) {
        set.status = 400;
        return parsed.error.flatten();
      }
      const { machine, active } = parsed.data;
      const hash = await requireSigner().writeContract({
        abi,
        address: CONTRACT_ADDRESS,
        functionName: "setMachine",
        args: [machine as Address, active],
        chain: null,
      });
      return { hash };
    },
    {
      body: t.Object({ machine: t.String(), active: t.Boolean() }),
    }
  )
  .post(
    "/verifier/set",
    async ({ body, set }) => {
      const schema = z.object({
        user: z.string(),
        newCount: z.coerce.bigint(),
      });
      const parsed = schema.safeParse(body);
      if (!parsed.success) {
        set.status = 400;
        return parsed.error.flatten();
      }
      const { user, newCount } = parsed.data;
      const hash = await requireSigner().writeContract({
        abi,
        address: CONTRACT_ADDRESS,
        functionName: "setBottleCount",
        args: [user as Address, newCount],
        chain: null,
      });
      return { hash };
    },
    {
      body: t.Object({ user: t.String(), newCount: t.String() }),
    }
  )
  .post(
    "/verifier/increment",
    async ({ body, set }) => {
      const schema = z.object({ user: z.string(), amount: z.coerce.bigint() });
      const parsed = schema.safeParse(body);
      if (!parsed.success) {
        set.status = 400;
        return parsed.error.flatten();
      }
      const { user, amount } = parsed.data;
      const hash = await requireSigner().writeContract({
        abi,
        address: CONTRACT_ADDRESS,
        functionName: "incrementBottleCount",
        args: [user as Address, amount],
        chain: null,
      });
      return { hash };
    },
    {
      body: t.Object({ user: t.String(), amount: t.String() }),
    }
  )
  // --- Relay redeem ---
  .post(
    "/redeem",
    async ({ body, set }) => {
      const schema = z.object({
        userId: z.string(), // 0x bytes32
        amount: z.coerce.bigint(),
        nonce: z.coerce.bigint(),
        deadline: z.coerce.bigint(),
        dataHash: z.string(), // 0x bytes32
        signature: z.string(), // 0x sig
      });
      const parsed = schema.safeParse(body);
      if (!parsed.success) {
        set.status = 400;
        return parsed.error.flatten();
      }
      const { userId, amount, nonce, deadline, dataHash, signature } =
        parsed.data;
      const hash = await requireSigner().writeContract({
        abi,
        address: CONTRACT_ADDRESS,
        functionName: "redeemMachineSignedIncrement",
        args: [
          userId as Hex,
          amount,
          nonce,
          deadline,
          dataHash as Hex,
          signature as Hex,
        ],
        chain: null,
      });
      return { hash };
    },
    {
      body: t.Object({
        userId: t.String(),
        amount: t.String(),
        nonce: t.String(),
        deadline: t.String(),
        dataHash: t.String(),
        signature: t.String(),
      }),
    }
  )
  // --- Helpers ---
  .post(
    "/tools/hashMachineIncrement",
    async ({ body, set }) => {
      const schema = z.object({
        userId: z.string(),
        amount: z.coerce.bigint(),
        nonce: z.coerce.bigint(),
        deadline: z.coerce.bigint(),
      });
      const parsed = schema.safeParse(body);
      if (!parsed.success) {
        set.status = 400;
        return parsed.error.flatten();
      }
      const { userId, amount, nonce, deadline } = parsed.data;
      const result = await publicClient.readContract({
        abi,
        address: CONTRACT_ADDRESS,
        functionName: "hashMachineIncrement",
        args: [userId as Hex, amount, nonce, deadline],
      });
      return { structHash: result };
    },
    {
      body: t.Object({
        userId: t.String(),
        amount: t.String(),
        nonce: t.String(),
        deadline: t.String(),
      }),
    }
  )
  .post(
    "/tools/hashToSign",
    async ({ body, set }) => {
      const schema = z.object({ structHash: z.string() });
      const parsed = schema.safeParse(body);
      if (!parsed.success) {
        set.status = 400;
        return parsed.error.flatten();
      }
      const { structHash } = parsed.data;
      const result = await publicClient.readContract({
        abi,
        address: CONTRACT_ADDRESS,
        functionName: "hashToSign",
        args: [structHash as Hex],
      });
      return { digest: result };
    },
    { body: t.Object({ structHash: t.String() }) }
  )
  .listen(8080);

console.log(
  `🦊 Elysia is running at http://${app.server?.hostname}:${app.server?.port}`
);
