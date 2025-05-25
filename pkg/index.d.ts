import { type heif_error, type EmbindModule  } from "./elheif-wasm";

/** Should be called and wait till promise fulfilled when using other APIs */
export function ensureInitialized(): Promise<void>

/** Convert heic image to RGBA bitmaps */
export function jsDecodeImage(buf: Uint8Array): ReturnType<EmbindModule["jsDecodeImage"]>;

/** Convert RGBA bitmap to heic image */
export function jsEncodeImage(buf: ImageData): ReturnType<EmbindModule["jsEncodeImage"]>;;