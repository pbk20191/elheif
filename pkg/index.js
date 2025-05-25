import MainModuleFactory from './elheif-wasm'

let _res = null
const _readyPromise = new Promise((res) => { _res = res })
let _ready = false
let _invokeEnsureInitialized = false
const elheif = {}

export async function ensureInitialized() {
    if (_invokeEnsureInitialized) {
        return _readyPromise
    }
    _invokeEnsureInitialized = true

    elheif.onRuntimeInitialized = () => {
        _ready = true
        _res()
        _res = null
    }

    
    
    return MainModuleFactory(elheif)
}

function checkReady() {
    if (!_ready) {
        throw Error("should call ensureInitialized and wait ready")
    }
}

export function jsDecodeImage(buf) {
    checkReady()
    return elheif.jsDecodeImage(buf)
}

export function jsEncodeImage(buf, width, height) {
    checkReady()
    return elheif.jsEncodeImage(buf, width, height)
}
