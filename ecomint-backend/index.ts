import express from "express";
import cors from "cors";
import { createHash } from "crypto";

const app = express();
const PORT = 3000;

// Middleware
app.use(cors());
app.use(express.json());

// POST /hash endpoint
app.post("/hash", (req, res) => {
  try {
    const { bottleNumber } = req.body;

    if (!bottleNumber) {
      return res.status(400).json({ error: "bottleNumber is required" });
    }

    // Generate hash from bottle number
    const hash = createHash("sha256")
      .update(bottleNumber.toString())
      .digest("hex");

    res.json({
      bottleNumber,
      hash,
      timestamp: new Date().toISOString(),
    });
  } catch (error) {
    res.status(500).json({ error: "Invalid JSON or server error" });
  }
});

// Handle other routes
app.use("*", (req, res) => {
  res.status(405).json({ error: "Method not allowed. Use POST /hash" });
});

app.listen(PORT, () => {
  console.log(`🚀 Bottle Hash API running on http://localhost:${PORT}`);
  console.log(
    '📝 Usage: POST /hash with {"bottleNumber": "your_bottle_number"}'
  );
});
