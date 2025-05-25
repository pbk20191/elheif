import fs from "node:fs/promises"
import path from "node:path"

import { fileURLToPath } from "url"; 

const __dirname = fileURLToPath(new URL(".", import.meta.url));
const srcDir = path.join(__dirname, "..", "build", "wasm");
const dstDir = path.join(__dirname, "..", "pkg");

try {
  const allFiles = await fs.readdir(srcDir);
  const wasmFiles = allFiles.filter(name => name.startsWith("elheif-wasm."));

  if (wasmFiles.length === 0) {
    console.warn("No elheif-wasm.* files found in build/wasm");
    process.exitCode = 1;
  }

  for (const file of wasmFiles) {
    const src = path.join(srcDir, file);
    const dst = path.join(dstDir, file);
    await fs.copyFile(src, dst);
  }
} catch (err) {
  console.error("Error copying wasm files:", err);
  process.exit(1);
}
