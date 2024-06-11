import { esbuildPlugin } from '@web/dev-server-esbuild';
import { playwrightLauncher } from '@web/test-runner-playwright';

export default {
    concurrency: 10,
    nodeResolve: true,
    files: ['js-tests/**/*.test.ts'],
    rootDir: './',
    browsers: [
        playwrightLauncher({
            product: 'chromium',
        }),
    ],
    plugins: [
        esbuildPlugin({ ts: true }),
    ],
    middleware: [add_headers],
    coverage: true,
    coverageConfig: {
        reporters: ['lcovonly', 'clover'],
    }
};