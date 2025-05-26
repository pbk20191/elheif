import { default as HeifModuleFactory } from "../pkg";
import { expect } from '@esm-bundle/chai';

declare function it(...args: any[]): void;

function base64ToArrayBuffer(base64: string) {
    var binaryString = atob(base64);
    var bytes = new Uint8Array(binaryString.length);
    for (var i = 0; i < binaryString.length; i++) {
        bytes[i] = binaryString.charCodeAt(i);
    }
    return bytes;
}

function loadHeicImage() {
    const DATA =
        "AAAAHGZ0eXBoZWljAAAAAG1pZjFoZWljbWlhZgAAApRtZXRhAAAAAAAAACFoZGxyAAAAAAAAAABwaWN0AAAAAAAAAAAAAAAAAAAAAA5waXRtAAAAAAABAAAANGlsb2MAAAAAREAAAgABAAAAAAK4AAEAAAAAAAAAMgACAAAAAALqAAEAAAAAAAAAIwAAADhpaW5mAAAAAAACAAAAFWluZmUCAAAAAAEAAGh2YzEAAAAAFWluZmUCAAAAAAIAAGh2YzEAAAAB02lwcnAAAAGsaXBjbwAAAHZodmNDAQNwAAAAAAAAAAAAHvAA/P34+AAADwMgAAEAGEABDAH//wNwAAADAJAAAAMAAAMAHroCQCEAAQAqQgEBA3AAAAMAkAAAAwAAAwAeoCCBBZbqrprm4CGgwIAAAAMAgAAAAwCEIgABAAZEAcFzwYkAAAAUaXNwZQAAAAAAAABAAAAAQAAAAChjbGFwAAAACgAAAAEAAAAKAAAAAf///8oAAAAC////ygAAAAIAAAAQcGl4aQAAAAADCAgIAAAAcWh2Y0MBBAgAAAAAAAAAAAAe8AD8/Pj4AAAPAyAAAQAXQAEMAf//BAgAAAMAn/gAAAMAAB66AkAhAAEAJkIBAQQIAAADAJ/4AAADAAAewIIEFluqumubAgAAAwACAAADAAIQIgABAAZEAcFzwYkAAAAUaXNwZQAAAAAAAABAAAAAQAAAAChjbGFwAAAACgAAAAEAAAAKAAAAAf///8oAAAAC////ygAAAAIAAAAOcGl4aQAAAAABCAAAACdhdXhDAAAAAHVybjptcGVnOmhldmM6MjAxNTphdXhpZDoxAAAAAB9pcG1hAAAAAAAAAAIAAQSBAoMEAAIFhQaHCIkAAAAaaXJlZgAAAAAAAAAOYXV4bAACAAEAAQAAAF1tZGF0AAAALigBrxMhYmNA9Sci//75Mn/pHyf9QdlhZ3K6wVvy+sD2ZJvA86qRnoCHaacwFXgAAAAfKAGuJkJKJOfXDbP+G8cXYVVzU7JsIGJEKRKAY/X0rg==";
    return base64ToArrayBuffer(DATA)
}

it('decode', async () => {
    const module = await HeifModuleFactory();

    const buf = loadHeicImage();
    const res = module.jsDecodeImage(buf);
    console.log(res);
    expect(res.error).eq(undefined);
    expect(res.data!.length).eq(1);
    const frame1 = res.data![0];
    expect(res.data![0].width).eq(10);
    expect(res.data![0].height).eq(10);
})

it('encode', async () => {
    const module = await HeifModuleFactory();

    const buf = loadHeicImage();
    const bitmap = module.jsDecodeImage(buf).data![0];
    const encoded = module.jsEncodeImages([bitmap], undefined);

    expect(encoded.error).eq(undefined);
    expect(encoded.data!.byteLength).eq(1074);
});
