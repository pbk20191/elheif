import fs from 'fs'
import path from 'path'

const wasmJsPath = path.resolve(import.meta.dirname, '../pkg/elheif-wasm.js')


let content = fs.readFileSync(wasmJsPath, 'utf-8')
content = `globalThis.__init__ELHEIF_MODULE = function (__ELHEIF_MODULE) {${content}};`
fs.writeFileSync(wasmJsPath, content)
console.log('elheif-wasm.js patched')