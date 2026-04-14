import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import tailwindcss from '@tailwindcss/vite';
import cssInjectedByJs from 'vite-plugin-css-injected-by-js';
import { fileURLToPath, URL } from 'node:url';

const servicePort = Number(process.env.PORT || 1102);
const proxyTarget = `http://127.0.0.1:${servicePort}`;

export default defineConfig({
    root: fileURLToPath(new URL('./web', import.meta.url)),
    envDir: fileURLToPath(new URL('.', import.meta.url)),
    plugins: [
        tailwindcss(),
        react(),
        cssInjectedByJs(),
        {
            name: 'lowercase-hash',
            apply: 'build',
            async writeBundle(options, bundle) {
                const { renameSync, readdirSync, readFileSync, writeFileSync } = await import('node:fs');
                const { join } = await import('node:path');
                const assetsDir = options.dir!;
                const renamedFiles: Record<string, string> = {};

                for (const file of readdirSync(assetsDir)) {
                    if (file.endsWith('.js') && file !== file.toLowerCase()) {
                        const lower = file.toLowerCase();
                        renameSync(join(assetsDir, file), join(assetsDir, lower));
                        renamedFiles[file] = lower;
                    }
                }

                if (Object.keys(renamedFiles).length === 0) {
                    return;
                }

                const indexHtmlPath = join(options.dir!, 'index.html');
                let html = readFileSync(indexHtmlPath, 'utf-8');
                for (const [oldName, newName] of Object.entries(renamedFiles)) {
                    html = html.replace(oldName, newName);
                }
                writeFileSync(indexHtmlPath, html);
            },
        },
    ],
    resolve: {
        alias: {
            '@': fileURLToPath(new URL('./web', import.meta.url)),
        },
    },
    server: {
        host: '0.0.0.0',
        port: 5173,
        proxy: {
            '/api': {
                target: proxyTarget,
                changeOrigin: true,
            },
        },
    },
    build: {
        outDir: '../dist/web',
        emptyOutDir: true,
        chunkSizeWarningLimit: 9999,
        rollupOptions: {
            output: {
                // Design intent: ship a single JS bundle (no code splitting).
                codeSplitting: false,
                hashCharacters: 'hex',
                entryFileNames: '[hash].js',
                chunkFileNames: '[hash].js',
            },
        },
    },
});
