interface DecodeImageResult {
    err: string,
    data: Array<ImageData>
    error: heif_error
}
interface heif_error {
      // main error category
  code:{
    constructor: () => any,
    value:number
  };

  // more detailed error code
  subcode:{
    constructor: () => any,
    value:number
  };

  // textual error message (is always defined, you do not have to check for NULL)
  message:str;
}

interface EncodeImageResult {
    err: string,
    data: Uint8Array
    error: heif_error
}

/** Should be called and wait till promise fulfilled when using other APIs */
export function ensureInitialized(): Promise<void>

/** Convert heic image to RGBA bitmaps */
export function jsDecodeImage(buf: Uint8Array): DecodeImageResult;

/** Convert RGBA bitmap to heic image */
export function jsEncodeImage(buf: ImageData): EncodeImageResult;