import { execSync } from "child_process";
import * as path from 'path'

const EMSDK_VERSION = '4.0.9'

const ROOT = path.resolve(import.meta.dirname, '../')
const EMSDK_ROOT = path.resolve(ROOT, './emsdk')

execSync('git clone https://github.com/emscripten-core/emsdk.git', {
    stdio: 'inherit',
    cwd: ROOT,
})
execSync(`./emsdk install ${EMSDK_VERSION}`, {
    stdio: 'inherit',
    cwd: EMSDK_ROOT,
})
execSync(`./emsdk activate ${EMSDK_VERSION}`, {
    stdio: 'inherit',
    cwd: EMSDK_ROOT,
})