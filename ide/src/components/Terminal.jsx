import React, { useEffect, useRef, useState } from 'react';
import { Terminal as XTerm } from 'xterm';
import { FitAddon } from 'xterm-addon-fit';
import { WebLinksAddon } from 'xterm-addon-web-links';
import 'xterm/css/xterm.css';
import { message } from 'antd';
import { handleCommand } from './terminalCommands';

function Terminal() {
  const terminalRef = useRef(null);
  const xtermRef = useRef(null);
  const fitAddonRef = useRef(null);
  const wsRef = useRef(null);
  const [isConnected, setIsConnected] = useState(false);

  useEffect(() => {
    // Initialize xterm.js
    const term = new XTerm({
      cursorBlink: true,
      fontSize: 14,
      fontFamily: 'Consolas, Monaco, "Courier New", monospace',
      theme: {
        background: '#1e1e1e',
        foreground: '#cccccc',
        cursor: '#cccccc',
        cursorAccent: '#1e1e1e',
        black: '#000000',
        red: '#cd3131',
        green: '#0dbc79',
        yellow: '#e5e510',
        blue: '#2472c8',
        magenta: '#bc3fbc',
        cyan: '#11a8cd',
        white: '#e5e5e5',
        brightBlack: '#666666',
        brightRed: '#f14c4c',
        brightGreen: '#23d18b',
        brightYellow: '#f5f543',
        brightBlue: '#3b8eea',
        brightMagenta: '#d670d6',
        brightCyan: '#29b8db',
        brightWhite: '#e5e5e5',
      },
      rows: 20,
      cols: 80,
      scrollback: 1000,
      allowProposedApi: true,
    });

    const fitAddon = new FitAddon();
    const webLinksAddon = new WebLinksAddon();

    term.loadAddon(fitAddon);
    term.loadAddon(webLinksAddon);

    if (terminalRef.current) {
      term.open(terminalRef.current);
      // Delay fit to ensure container has dimensions
      setTimeout(() => {
        try {
          fitAddon.fit();
        } catch (err) {
          console.error('Error fitting terminal on init:', err);
        }
      }, 100);
    }

    xtermRef.current = term;
    fitAddonRef.current = fitAddon;

    // Display welcome message
    term.writeln('\x1b[1;36m╔═══════════════════════════════════════════════════════════╗\x1b[0m');
    term.writeln('\x1b[1;36m║         FluffOS IDE - Integrated Terminal                ║\x1b[0m');
    term.writeln('\x1b[1;36m╚═══════════════════════════════════════════════════════════╝\x1b[0m');
    term.writeln('');
    term.writeln('\x1b[1;33mWelcome to the FluffOS IDE Terminal!\x1b[0m');
    term.writeln('');
    term.writeln('\x1b[90mThis terminal provides a direct interface to your MUD environment.\x1b[0m');
    term.writeln('\x1b[90mYou can execute commands, test LPC code, and interact with the driver.\x1b[0m');
    term.writeln('');
    term.writeln('\x1b[1;32m✓ Terminal initialized\x1b[0m');
    term.writeln('\x1b[1;33m⚠ Backend terminal handler pending implementation\x1b[0m');
    term.writeln('');
    term.writeln('\x1b[90mType commands and press Enter to execute...\x1b[0m');
    term.writeln('');
    term.write('\x1b[1;32m$\x1b[0m ');

    let currentLine = '';

    // Handle user input
    term.onData((data) => {
      const code = data.charCodeAt(0);

      if (code === 13) { // Enter key
        term.write('\r\n');
        if (currentLine.trim()) {
          handleCommand(currentLine.trim(), term);
          currentLine = '';
        }
        term.write('\x1b[1;32m$\x1b[0m ');
      } else if (code === 127) { // Backspace
        if (currentLine.length > 0) {
          currentLine = currentLine.slice(0, -1);
          term.write('\b \b');
        }
      } else if (code >= 32 && code < 127) { // Printable characters
        currentLine += data;
        term.write(data);
      }
    });

    setIsConnected(true);

    // Handle window resize
    const handleResize = () => {
      if (fitAddonRef.current) {
        try {
          fitAddonRef.current.fit();
        } catch (err) {
          console.error('Error fitting terminal:', err);
        }
      }
    };

    window.addEventListener('resize', handleResize);
    // Also fit when parent container size changes
    const resizeObserver = new ResizeObserver(handleResize);
    if (terminalRef.current) {
      resizeObserver.observe(terminalRef.current);
    }

    // Cleanup
    return () => {
      window.removeEventListener('resize', handleResize);
      if (terminalRef.current) {
        resizeObserver.unobserve(terminalRef.current);
      }
      term.dispose();
    };
  }, []);

  return (
    <div style={{
      height: '100%',
      width: '100%',
      backgroundColor: '#1e1e1e',
      display: 'flex',
      flexDirection: 'column',
    }}>
      <div style={{
        padding: '8px 16px',
        backgroundColor: '#2d2d30',
        borderBottom: '1px solid #3e3e42',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'space-between',
      }}>
        <div style={{
          color: '#cccccc',
          fontSize: '13px',
          fontWeight: 500,
          display: 'flex',
          alignItems: 'center',
          gap: '8px',
        }}>
          Terminal
          <span style={{
            display: 'inline-block',
            width: '8px',
            height: '8px',
            borderRadius: '50%',
            backgroundColor: isConnected ? '#89d185' : '#f48771',
          }} />
        </div>
      </div>
      <div
        ref={terminalRef}
        style={{
          flexGrow: 1,
          padding: '8px',
          overflow: 'hidden',
        }}
      />
    </div>
  );
}

export default Terminal;
