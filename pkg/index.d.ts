interface DecodeImageResult {
    err: string,
    data: Array<ImageData>
}

interface EncodeImageResult {
    err: string,
    data: Uint8Array
}

/** Should be called and wait till promise fulfilled when using other APIs */
export function ensureInitialized(): Promise<void>

/** Convert heic image to RGBA bitmaps */
export function jsDecodeImage(buf: Uint8Array): DecodeImageResult;

/** Convert RGBA bitmap to heic image */
export function jsEncodeImage(buf: ImageData): EncodeImageResult;