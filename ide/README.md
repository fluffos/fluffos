# FluffOS IDE

A modern web-based Integrated Development Environment for FluffOS MUD development, featuring real-time file editing, LPC compilation, object inspection, and an integrated terminal.

## Features

### 📁 File Management
- **File Browser**: Navigate the mudlib directory structure
- **Syntax Highlighting**: Full LPC syntax highlighting with Monaco editor
- **Live Editing**: Edit files directly in the browser
- **Auto-save**: Changes saved automatically via WebSocket

### ⚡ Compilation
- **Real-time Compilation**: Compile LPC files with instant feedback
- **Error Display**: Clear error messages with file/line references
- **Call Stack**: View compilation call stack on errors

### 🔍 Object Inspection
- **Live Objects List**: View all loaded objects in the MUD
- **Auto-refresh**: Object list updates every 5 seconds
- **Object Details**: Inspect variables, functions, and inheritance
  - Variable names, values, and types
  - Function list
  - Inheritance chain
- **One-click Inspection**: Click any object to see full details

### 💻 Terminal
- **Integrated Terminal**: xterm.js-based terminal emulator
- **Command Execution**: Execute MUD commands (backend pending)
- **Syntax Highlighting**: Color-coded output

## Architecture

### Frontend
- **Framework**: React 18 with Vite
- **UI Components**: Ant Design
- **Editor**: Monaco Editor (VSCode's editor)
- **Terminal**: xterm.js
- **Layout**: Allotment (resizable split panes)

### Backend
- **WebSocket Protocol**: Custom `debug` subprotocol
- **API Handler**: `src/net/ws_debug.cc`
- **Commands**:
  - `files` - List files in directory
  - `file_get` - Get file contents
  - `file_post` - Save file contents
  - `compile` - Compile LPC file
  - `objects` - List all loaded objects
  - `object_inspect` - Get object details

### Communication
```
Frontend (React) <--WebSocket--> Proxy (Node.js) <--WebSocket--> FluffOS Driver
   Port 5000           Port 5000                    Port 5001 (debug)
```

## Development Setup

### Prerequisites
- Node.js 18+ (use `nvm use --lts`)
- FluffOS built in Debug mode
- WebSocket debug port enabled (port 5001)

### Step-by-Step Setup

#### 1. Configure FluffOS
Enable debug port in your config file (`testsuite/etc/config.test`):
```
external_port_5 : debug 5001
```

#### 2. Build the Driver
```bash
cd build
make -j driver
```

#### 3. Install IDE Dependencies
```bash
cd ../ide
nvm use --lts  # Ensure Node.js is available
npm install
```

#### 4. Start the Development Environment

**Terminal 1 - Start IDE Dev Server:**
```bash
cd ide
npm run dev
```
This starts the IDE on http://localhost:5000

**Terminal 2 - Start FluffOS Driver:**
```bash
cd testsuite
../build/src/driver etc/config.test
```

**Terminal 3 (Optional) - Rebuild on Changes:**
```bash
cd build
make -j driver
```

#### 5. Access the IDE
Open http://localhost:5000 in your browser

## Production Build

```bash
cd ide
npm run build

# Copy to driver's www directory
rsync -a dist/. ../testsuite/www/
```

The driver will serve the IDE from its HTTP server.

## WebSocket API

### Request Format
```json
{
  "command": "command_name",
  "requestId": 123,
  ...additional parameters
}
```

### Response Format
```json
{
  "requestId": 123,
  ...response data or error
}
```

### Available Commands

#### `files`
List files in a directory
```json
{
  "command": "files",
  "path": ".",
  "requestId": 1
}
```

#### `file_get`
Get file contents
```json
{
  "command": "file_get",
  "path": "command/goto.c",
  "requestId": 2
}
```

#### `file_post`
Save file contents
```json
{
  "command": "file_post",
  "path": "command/goto.c",
  "content": "...",
  "requestId": 3
}
```

#### `compile`
Compile an LPC file
```json
{
  "command": "compile",
  "file": "/command/goto",
  "requestId": 4
}
```

#### `objects`
List all loaded objects
```json
{
  "command": "objects",
  "requestId": 5
}
```

Response includes `obname`, `load_time`, `flags`, `ref_count` for each object.

#### `object_inspect`
Get detailed object information
```json
{
  "command": "object_inspect",
  "obname": "/single/master",
  "requestId": 6
}
```

Response includes variables, functions, and inheritance information.

## File Structure

```
ide/
├── src/
│   ├── components/
│   │   ├── FileTree.jsx        # File browser
│   │   ├── FileEditor.jsx      # Monaco editor wrapper
│   │   ├── CompilationView.jsx # Compilation results
│   │   ├── ObjectList.jsx      # Object list viewer
│   │   ├── ObjectInspector.jsx # Object details panel
│   │   └── Terminal.jsx        # Terminal emulator
│   ├── App.jsx                 # Main application
│   ├── WebSocketContext.jsx   # WebSocket connection
│   └── api.js                  # API helpers
├── server.js                   # Dev proxy server
├── package.json
└── vite.config.js
```

## Security

The IDE API includes path traversal protection via `resolve_project_path()` in `ws_debug.cc`. All file operations are restricted to the mudlib directory.

## Troubleshooting

### WebSocket Connection Fails
- Ensure debug port is enabled in FluffOS config
- Check that driver is running
- Verify port 5001 is accessible

### Files Not Loading
- Check mudlib directory configuration
- Verify file permissions
- Check browser console for errors

### Terminal Not Displaying
- Clear browser cache
- Check for JavaScript errors
- Ensure xterm.js loaded correctly

## Contributing

When adding new features:
1. Add backend handler in `src/net/ws_debug.cc`
2. Add command to WebSocket callback
3. Update frontend API in `WebSocketContext.jsx`
4. Create/update React components
5. Test with live driver

## License

Part of FluffOS project - see main repository for license information.
