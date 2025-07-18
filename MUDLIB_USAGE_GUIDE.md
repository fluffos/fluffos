# FluffOS Mudlib Usage Guide

## Overview

FluffOS is an LPMUD driver based on the last release of MudOS (v22.2b14), enhanced with 10+ years of bug fixes and performance improvements. This guide provides comprehensive instructions for using the mudlib that comes with FluffOS.

## What is FluffOS?

FluffOS is a game engine commonly used for building MUDs (Multi-User Dungeons). It consists of three main components:

- **LPC Interpreter**: Parse and execute LPC-based programs
- **Network Server**: Manage client connections via TELNET, TLS, and WebSocket protocols
- **Glue Interface**: EFUN (built-in functions) and Apply (callback functions) for LPC integration

## Documentation Sources

### Primary Documentation

1. **Official Website**: https://www.fluffos.info
   - Comprehensive documentation, build guides, and API references
   - Primary source for all FluffOS documentation

2. **Main README.md**: Overview and quick start information
   - Features overview
   - Build instructions summary
   - Community links and support resources

3. **Build Documentation**: `docs/build.md`
   - Detailed build instructions for Ubuntu LTS, macOS, and Windows
   - Platform-specific dependencies and requirements
   - Docker setup instructions

### Local Documentation Structure

```
docs/
├── build.md              # Build instructions
├── index.md              # Main documentation index
├── docker-ubuntu.md      # Docker setup guide
├── efun/                 # Built-in function documentation
├── apply/                # Callback function documentation
├── concepts/             # Core concepts and guides
└── lpc/                  # LPC language documentation
```

## Getting Started with the Test Mudlib

### The "Lil" Mudlib

FluffOS includes a minimal test mudlib called "Lil" located in the `testsuite/` directory:

**Purpose**: 
- Provides a working example to bootstrap mudlib development
- Allows the driver to boot and accept logins immediately
- Serves as a foundation for building custom mudlibs

**Key Components**:
```
testsuite/
├── README                # Mudlib overview and usage
├── etc/config.test       # Main configuration file
├── single/               # Core objects (master, simul_efun)
├── std/                  # Standard library components
├── include/              # Header files
├── command/              # User commands
└── inherit/              # Inheritable objects
```

### Configuration

The main configuration file is located at `testsuite/etc/config.test` and includes:

- **Server Settings**: Port numbers, IP binding, protocols
- **File Paths**: Mudlib directory, log directory, include paths
- **Runtime Parameters**: Memory limits, timing settings, security options
- **Feature Toggles**: ANSI support, tracing, error handling

**Key Configuration Options**:
```
name : Testsuite
mud ip : 0.0.0.0
port number : 4000
mudlib directory : ./
master file : /single/master
simulated efun file : /single/simul_efun
```

### Running the Test Mudlib

1. **Build FluffOS** (see build instructions in `docs/build.md`)
2. **Navigate to the FluffOS directory**
3. **Run the driver**:
   ```bash
   ./bin/driver testsuite/etc/config.test
   ```
4. **Connect to the mud**:
   - Telnet: `telnet localhost 4000`
   - WebSocket: Connect to `ws://localhost:4001`

## Features and Capabilities

### Language Features
- **UTF-8 Support**: Native UTF-8 string operations with emoji support
- **64-bit Runtime**: LPC int is 64-bit, LPC float is C double
- **Type Checking**: Enhanced compile-time type checking
- **Modern LPC**: Updated language features since MudOS

### Network Support
- **Multiple Protocols**: TELNET, TLS, WebSocket
- **IPv6 Support**: Optional IPv6 connectivity
- **Telnet Extensions**: MXP, GMCP, ZMP, MSSP, MSP, MSDP

### Database Integration
- **Multiple Databases**: MySQL, PostgreSQL, SQLite
- **Async Operations**: Non-blocking I/O operations
- **External Programs**: Integration with external commands

### Development Tools
- **Memory Tracking**: Detailed memory accounting via `mud_status(1)`
- **Debugging**: LPC tracing and memory leak detection
- **Testing**: Comprehensive test suite for all EFUNs

## Testing and Development

### EFUN Testing
The testsuite includes tests for all built-in functions:

**Location**: `testsuite/single/efuns/`
**Usage**: Login to the mud and type `efun function_name`
**Example**: `efun capitalize`

### Adding Tests
Each EFUN test should:
- Test all code branches of the function
- Include boundary condition testing
- Verify full functionality
- Follow the naming convention (file named after the efun)

## Community and Support

### Official Support Channels

1. **Discord**: #fluffos channel on LPC Discord
   - Join: https://discord.gg/2a6qqG6Cdj
   - Real-time community support

2. **Forum**: https://forum.fluffos.info
   - Structured discussions and documentation
   - Long-form help and tutorials

3. **QQ Group**: 451819151 (Chinese community)

### Development Resources

- **GitHub Repository**: https://github.com/fluffos/fluffos
- **Issue Tracking**: Report bugs and request features
- **Contributing**: See contribution guidelines in the repository

## Advanced Configuration

### Build Options

**Platform Support**:
- Ubuntu 22.04+ (primary platform)
- macOS (latest versions)
- Windows (MSYS2/mingw64)
- WSL with Ubuntu 22.04

**Compiler Requirements**:
- C++17 and C11 support
- GCC 7+ or LLVM clang 4+

**Dependencies**:
- ICU (UTF-8 support)
- jemalloc (memory management)
- OpenSSL (TLS support)
- PCRE (regular expressions)
- MySQL/PostgreSQL client libraries

### Runtime Configuration

**Memory Management**:
- Configurable heap limits
- Automatic garbage collection
- Memory leak detection

**Performance Tuning**:
- Evaluation cost limits
- Call depth restrictions
- Array and string size limits

**Security Features**:
- ANSI code stripping
- Input validation
- Crash handling

## Mudlib Development Tips

### Starting a New Mudlib

1. **Use Lil as Base**: Copy the testsuite directory as your starting point
2. **Customize Configuration**: Modify `etc/config.test` for your needs
3. **Implement Core Objects**: 
   - Master object (`/single/master`)
   - Simulated efuns (`/single/simul_efun`)
   - User object template

### Best Practices

1. **Code Organization**: Use the standard directory structure
2. **Error Handling**: Implement robust error handlers
3. **Testing**: Write tests for your mudlib functions
4. **Documentation**: Comment your code and maintain documentation
5. **Version Control**: Use git for mudlib development

## Troubleshooting

### Common Issues

1. **Driver Won't Start**: Check configuration file syntax
2. **Connection Problems**: Verify port settings and firewall
3. **Compilation Errors**: Ensure all dependencies are installed
4. **Memory Issues**: Adjust memory limits in configuration

### Debug Resources

- **Debug Log**: Check `log/debug.log` for error messages
- **Console Mode**: Use `-C` flag for interactive debugging
- **Tracing**: Enable LPC tracing for code debugging

## Migration from MudOS

FluffOS is designed to be backward-compatible with MudOS:

1. **Minimal Code Changes**: Most existing mudlibs work with little modification
2. **Configuration Updates**: Update config file format if needed
3. **Feature Upgrades**: Take advantage of new FluffOS features
4. **Testing**: Use the comprehensive test suite to verify compatibility

## Examples and References

### Sample Projects Using FluffOS

- **ThresholdRPG**: https://wiki.thresholdrpg.com/
- **SWmud**: http://www.swmud.org/
- **Merentha**: https://www.merentha.com/
- Various Chinese MUDs

### Learning Resources

1. **LPC Programming**: Study files in `testsuite/` for examples
2. **EFUN Documentation**: Browse `docs/efun/` for function references
3. **Apply Documentation**: Check `docs/apply/` for callback functions
4. **Community Code**: Examine existing mudlib implementations

## Contributing

### Ways to Contribute

1. **Bug Reports**: Use GitHub issues for bug reports
2. **Feature Requests**: Suggest new features via GitHub
3. **Code Contributions**: Submit pull requests for improvements
4. **Documentation**: Help improve documentation and guides
5. **Testing**: Contribute to the test suite

### Development Workflow

1. **Fork Repository**: Create your own fork on GitHub
2. **Create Branch**: Work on feature branches
3. **Test Changes**: Ensure all tests pass
4. **Submit PR**: Create pull request with clear description

## License

FluffOS is released under the MIT License. See the `Copyright` file for full license details.

---

*This guide covers the essential aspects of using FluffOS mudlib. For the most current information, always refer to the official documentation at https://www.fluffos.info*
