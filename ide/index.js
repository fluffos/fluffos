const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const fs = require('fs');
const path = require('path');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

// The root directory to serve files from, passed as a command-line argument.
const rootDir = process.argv[2] || '.';

app.use(express.json());

// Endpoint to list files in a directory.
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

// Endpoint to read the content of a file.
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

server.listen(3001, () => {
  console.log(`Server listening on port 3001, serving files from ${path.resolve(rootDir)}`);
});
