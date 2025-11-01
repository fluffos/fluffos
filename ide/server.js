const express = require('express');
const { createProxyMiddleware } = require('http-proxy-middleware');
const path = require('path');
const vite = require('vite');
const fs = require('fs');

async function createServer() {
  const app = express();
  const port = 3000; // Node.js server port
  const apiServerPort = 3001; // C++ API server port
  const isProd = process.env.NODE_ENV === 'production';

  // Proxy API requests to the C++ API server
  app.use(
    '/api',
    createProxyMiddleware({
      target: `http://localhost:${apiServerPort}`,
      changeOrigin: true,
      pathRewrite: {
        '^/api': '/api', // Keep the /api prefix for all API paths
      },
    })
  );

  if (!isProd) {
    const viteServer = await vite.createServer({
      server: { middlewareMode: true },
      appType: 'custom' // don't inject Vite's own HTML transform middleware
    });
    app.use(viteServer.middlewares);

    // Serve index.html and apply Vite HTML transforms
    app.use('*', async (req, res, next) => {
      if (req.url === '/' || req.url.endsWith('.html')) {
        try {
          let template = fs.readFileSync(path.resolve(__dirname, 'index.html'), 'utf-8');
          template = await viteServer.transformIndexHtml(req.url, template);
          res.status(200).set({ 'Content-Type': 'text/html' }).end(template);
        } catch (e) {
          viteServer.ssrFixStacktrace(e);
          next(e);
        }
      } else {
        next();
      }
    });

  } else {
    app.use(express.static(path.join(__dirname, 'dist')));
    app.get('*', (req, res) => {
      res.sendFile(path.join(__dirname, 'dist', 'index.html'));
    });
    console.log(`Serving production frontend from /dist`);
  }

  app.listen(port, () => {
    console.log(`Node.js proxy server listening on port ${port}`);
    console.log(`API requests proxied to C++ server on port ${apiServerPort}`);
  });
}

createServer();
