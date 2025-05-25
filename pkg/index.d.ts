import Factory from './elheif-wasm';

// TypeScript bindings for emscripten-generated code.  Automatically generated at compile time.
export * from './elheif-wasm';

export interface WasmModuleProp {

    onRuntimeInitialized?: () => void
    onAbort?: (tag:string|number|undefined) => void;
    locateFile?:(file:string) => string;
    monitorRunDependencies?: (left:number) => void;
    onExit?: (code: number) => void;
    preInit?: (() => void);
    preRun?: (() => void);
    postRun?: (() => void);
}

export function MainModuleFactory(options?: WasmModuleProp): ReturnType<typeof Factory>

export default MainModuleFactory;