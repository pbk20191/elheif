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
        playwrightLauncher({ product: 'firefox' }),
        ...(process.env.TEST_WEBKIT ? [playwrightLauncher({ product: 'webkit' })] : []),
    ],
    plugins: [
        esbuildPlugin({ ts: true }),
    ],
    coverage: true,
    coverageConfig: {
        reporters: ['lcovonly', 'clover'],
    }
};