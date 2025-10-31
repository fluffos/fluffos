const WebSocket = require('ws');
const ws = new WebSocket('ws://localhost:5001/debug', ['debug']);

ws.on('open', function open() {
    console.log('Connected');
    // Test objects command
    ws.send(JSON.stringify({ command: 'objects', requestId: 1 }));
});

ws.on('message', function message(data) {
    console.log('received:', data.toString());
    const response = JSON.parse(data);
    if (response.objects) {
        console.log(`\nFound ${response.objects.length} objects:`);
        response.objects.slice(0, 5).forEach(obj => {
            console.log(`  - ${obj.obname} (flags: 0x${obj.flags.toString(16)}, refs: ${obj.ref_count})`);
        });
    }
    ws.close();
});

ws.on('error', function error(err) {
    console.error('Error:', err);
    process.exit(1);
});
