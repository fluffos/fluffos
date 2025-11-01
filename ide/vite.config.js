import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import path from 'path';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [react()],
  server: {
    port: 5173, // Ensure Vite runs on its own port
  },
  resolve: {
    dedupe: ['monaco-editor', 'vscode'],
    alias: {
      'vscode': path.resolve(__dirname, 'node_modules/@codingame/monaco-vscode-standalone-languages'),
    }
  },
  optimizeDeps: {
    include: [
      '@codingame/monaco-vscode-api',
      '@codingame/monaco-vscode-files-service-override',
      '@codingame/monaco-vscode-explorer-view',
    ]
  }
});
