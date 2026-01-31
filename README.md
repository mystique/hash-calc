# HashCalc

A modern Windows GUI application for computing and verifying file hashes using various cryptographic algorithms.

## Features

- **Multiple Hash Algorithms**: Support for industry-standard hashing algorithms including MD5, SHA-1, SHA-256, SHA-512, and more
- **File Hashing**: Compute hashes for any file quickly and efficiently
- **Text Hashing**: Calculate hashes for text input
- **Hash Verification**: Verify file integrity by comparing computed hashes with known values
- **User-Friendly Interface**: Intuitive Windows GUI built with Win32++
- **Batch Operations**: Process multiple files in one session

## System Requirements

- **OS**: Windows 7 or later (Windows 10/11 recommended)
- **Architecture**: x64 or x86
- **Processor**: Any modern CPU with 1 GHz or higher
- **Memory**: 50 MB RAM minimum
- **Disk Space**: 10 MB for installation

## Installation

### Prerequisites

- Visual Studio 2019 or later (with C++ workload)
- CMake 3.20 or later
- vcpkg (for dependency management)

### Build Instructions

1. **Clone the repository**:
   ```bash
   git clone https://github.com/mystique/hash-calc.git
   cd hash-calc
   ```

2. **Install dependencies** using vcpkg:
   ```bash
   vcpkg install cryptopp:x64-windows
   ```

3. **Configure and build** with CMake:
   ```bash
   cmake --preset default
   cmake --build build --config Release
   ```

4. **Run the application**:
   ```bash
   ./build/Release/HashCalc.exe
   ```

## Usage

### Computing File Hash

1. Launch HashCalc
2. Select desired hash algorithm from the dropdown menu
3. Click "Browse" or drag-and-drop a file
4. The computed hash will be displayed automatically

### Verifying File Integrity

1. Open HashCalc
2. Select the file and hash algorithm
3. Paste or enter the expected hash value
4. The application will compare and display whether the hash matches

### Computing Text Hash

1. Switch to the "Text" tab
2. Enter or paste your text
3. Select hash algorithm
4. View the computed hash

## Technical Details

- **Language**: C++ 17
- **Framework**: Win32++ (Windows wrapper library)
- **Cryptography**: Crypto++ library
- **Build System**: CMake
- **GUI Framework**: Native Windows (Win32)

## Architecture

The application follows a layered architecture:

```
UI Layer (HashCalcDialog, HoverButton, etc.)
   ↓
Core Layer (HashAlgorithmFactory, IHashAlgorithm implementations)
   ↓
Crypto++ Library (Cryptographic operations)
```

### Key Components

- **HashAlgorithmFactory**: Creates instances of different hash algorithm implementations
- **IHashAlgorithm**: Abstract interface for hash algorithm implementations
- **HashCalcDialog**: Main UI window and user interaction handler

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

## Support

If you encounter any issues or have questions:

1. Check existing issues on GitHub
2. Create a new issue with detailed description and steps to reproduce
3. Include your Windows version and architecture in bug reports

## Changelog

### Version 1.0.0 (Initial Release)
- Initial release with core hashing functionality
- Support for multiple cryptographic algorithms
- File and text hashing capabilities
- Hash verification feature
- Windows GUI application

## Acknowledgments

- Built with [Win32++](https://sourceforge.net/projects/win32xx/) - A modern C++ wrapper for Windows
- Cryptography provided by [Crypto++](https://www.cryptopp.com/) library
