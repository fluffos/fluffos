const WebSocket = require('ws');
const ws = new WebSocket('ws://localhost:3001/api', ['api']);

ws.on('open', function open() {
    console.log('Connected');
    ws.send(JSON.stringify({ command: 'files', path: '.' }));
});

ws.on('message', function message(data) {
    console.log('received: %s', data);
    ws.close();
});

ws.on('error', function error(err) {
    console.error('Error:', err);
    process.exit(1);
});
