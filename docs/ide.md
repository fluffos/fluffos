# FluffOS Web IDE

FluffOS now includes a modern web-based IDE for MUD development. The IDE provides real-time file editing, compilation, object inspection, and more.

## Quick Start

Enable the debug port in your config file:
```
external_port_5 : debug 5001
```

Start the backend and frontend:
```bash
# Terminal 1: Build and start FluffOS
cd build && make -j driver
cd ../testsuite
../build/src/driver etc/config.test

# Terminal 2: Start IDE dev server
cd ide
npm install
npm run dev
```

Then open http://localhost:5000 in your browser.

## Features

- **File Browser & Editor** - Navigate and edit LPC files with syntax highlighting
- **Live Compilation** - Compile files and see errors in real-time
- **Object Inspector** - View all loaded objects and inspect their variables, functions, and inheritance
- **Integrated Terminal** - Execute commands directly from the browser
- **Auto-refresh** - Object list updates every 5 seconds to reflect MUD state

## Configuration

Enable the debug port in your config file:

```
external_port_5 : debug 5001
```

## Documentation

See [ide/README.md](../ide/README.md) for detailed documentation including:
- Development setup
- WebSocket API reference
- Architecture overview
- Security considerations

## Production Deployment

Build and serve from the driver:

```bash
cd ide
npm run build
rsync -a dist/. ../testsuite/www/
```

Then access via the driver's HTTP server (default port 5000).
