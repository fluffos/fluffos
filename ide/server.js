const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const fs = require('fs');
const path = require('path');

async function startServer() {
  const app = express();
  const server = http.createServer(app);
  const wss = new WebSocket.Server({ server });

  const rootDir = process.argv[2] || '.';

  app.use(express.json());

  // API Endpoints must be registered before the Vite middleware
  app.get('/files', (req, res) => {
    const dirPath = path.resolve(rootDir, req.query.path || '.');
    if (!dirPath.startsWith(path.resolve(rootDir))) {
      return res.status(403).send('Forbidden');
    }
    fs.readdir(dirPath, { withFileTypes: true }, (err, files) => {
      if (err) {
        return res.status(500).send('Error reading directory');
      }
      const fileList = files.map(file => ({
        name: file.name,
        isDirectory: file.isDirectory(),
      }));
      res.json(fileList);
    });
  });

  app.get('/file', (req, res) => {
    const filePath = path.resolve(rootDir, req.query.path);
    if (!filePath.startsWith(path.resolve(rootDir))) {
      return res.status(403).send('Forbidden');
    }
    fs.readFile(filePath, 'utf8', (err, data) => {
      if (err) {
        return res.status(500).send('Error reading file');
      }
      res.send(data);
    });
  });

  app.post('/file', (req, res) => {
    const filePath = path.resolve(rootDir, req.body.path);
    if (!filePath.startsWith(path.resolve(rootDir))) {
      return res.status(403).send('Forbidden');
    }
    fs.writeFile(filePath, req.body.content, 'utf8', err => {
      if (err) {
        return res.status(500).send('Error writing file');
      }
      res.send('File saved successfully');
    });
  });

  if (process.env.NODE_ENV !== 'production') {
    console.log('Running in development mode');
    const { createServer: createViteServer } = require('vite');
    const vite = await createViteServer({
      server: { middlewareMode: true },
      appType: 'spa',
    });
    // Use vite's connect instance as middleware
    app.use(vite.middlewares);
  } else {
    console.log('Running in production mode');
    app.use(express.static(path.join(__dirname, 'dist')));
    app.get('*', (req, res) => {
      res.sendFile(path.join(__dirname, 'dist', 'index.html'));
    });
  }

  server.listen(3001, () => {
    console.log(`Server listening on port 3001, serving files from ${path.resolve(rootDir)}`);
  });
}

startServer();
