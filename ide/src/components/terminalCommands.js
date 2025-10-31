// Terminal command handlers
export const handleCommand = (command, term) => {
  const parts = command.split(' ');
  const cmd = parts[0].toLowerCase();
  const args = parts.slice(1);

  switch (cmd) {
    case 'help':
      term.writeln('\x1b[1;36mAvailable Commands:\x1b[0m');
      term.writeln('  \x1b[1;33mhelp\x1b[0m         - Show this help message');
      term.writeln('  \x1b[1;33mclear\x1b[0m        - Clear the terminal screen');
      term.writeln('  \x1b[1;33mecho\x1b[0m [text]  - Echo text back');
      term.writeln('  \x1b[1;33mdate\x1b[0m         - Display current date and time');
      term.writeln('  \x1b[1;33mpwd\x1b[0m          - Print working directory');
      term.writeln('  \x1b[1;33minfo\x1b[0m         - Show IDE information');
      term.writeln('');
      term.writeln('\x1b[90mNote: Full terminal support requires backend implementation\x1b[0m');
      break;

    case 'clear':
      term.clear();
      break;

    case 'echo':
      term.writeln(args.join(' '));
      break;

    case 'date':
      term.writeln(new Date().toString());
      break;

    case 'pwd':
      term.writeln('/mudlib');
      break;

    case 'info':
      term.writeln('\x1b[1;36mFluffOS IDE Terminal\x1b[0m');
      term.writeln('Version: 1.0.0');
      term.writeln('Terminal Emulator: xterm.js');
      term.writeln('Status: Frontend ready, backend pending');
      term.writeln('');
      term.writeln('\x1b[90mTo enable full terminal functionality, implement the\x1b[0m');
      term.writeln('\x1b[90mWebSocket terminal handler in the C++ backend.\x1b[0m');
      break;

    case '':
      // Empty command, do nothing
      break;

    default:
      term.writeln(`\x1b[1;31mCommand not found: ${cmd}\x1b[0m`);
      term.writeln('Type \x1b[1;33mhelp\x1b[0m for available commands');
      break;
  }
};
