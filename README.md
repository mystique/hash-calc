# HashCalc

<div align="center">

A modern, feature-rich Windows GUI application for computing and verifying file hashes using a comprehensive collection of cryptographic algorithms.

![Platform](https://img.shields.io/badge/platform-Windows-blue)
![C++](https://img.shields.io/badge/C++-17-00599C?logo=c%2B%2B)
![License](https://img.shields.io/badge/license-MIT-green)
![Algorithms](https://img.shields.io/badge/algorithms-50+-orange)

</div>

---

## Table of Contents

- [Features](#features)
  - [Core Functionality](#-core-functionality)
  - [Supported Hash Algorithms](#-supported-hash-algorithms)
  - [User Interface Features](#-user-interface-features)
- [System Requirements](#system-requirements)
- [Installation](#installation)
  - [Prerequisites](#prerequisites)
  - [Build Instructions](#build-instructions)
- [Usage](#usage)
  - [Computing File Hash](#computing-file-hash)
  - [Computing Text Hash](#computing-text-hash)
  - [Verifying File Integrity](#verifying-file-integrity)
- [Technical Details](#technical-details)
  - [Technology Stack](#technology-stack)
  - [Architecture Overview](#architecture-overview)
  - [Algorithm Implementations](#algorithm-implementations)
- [Project Structure](#project-structure)
- [Development](#development)
- [Performance](#performance)
- [Security Considerations](#security-considerations)
- [License](#license)
- [Contributing](#contributing)
- [Changelog](#changelog)
- [Acknowledgments](#acknowledgments)

---

## Features

### 🎯 Core Functionality
- **File Hashing**: Compute hashes for any file quickly and efficiently with progress tracking
- **Text Hashing**: Calculate hashes for text input with instant results
- **Multi-Algorithm Support**: Compute multiple hashes simultaneously in a single operation
- **Drag & Drop**: Simply drag files onto the application window to start hashing
- **Hash Verification**: Compare computed hashes with known values to verify file integrity

### 🔐 Supported Hash Algorithms

HashCalc supports **50+ cryptographic algorithms** organized into 4 categories:

<details open>
<summary><b>📊 Tab 1: SHA & MD Family</b></summary>

| Algorithm | Output Size | Description |
|-----------|------------|-------------|
| **SHA-1** | 160-bit | Secure Hash Algorithm 1 |
| **SHA-2** | 224/256/384/512-bit | SHA-224, SHA-256, SHA-384, SHA-512 |
| **MD Family** | 128-bit | MD2, MD4, MD5 (legacy algorithms) |
| **MD6** | 128/160/192/224/256/384/512-bit | Native implementation with configurable parameters |

</details>

<details open>
<summary><b>🔷 Tab 2: SHA-3 & Modern Algorithms</b></summary>

| Algorithm | Output Size | Description |
|-----------|------------|-------------|
| **SHA-3** | 224/256/384/512-bit | SHA3-224, SHA3-256, SHA3-384, SHA3-512 |
| **Keccak** | 224/256/384/512-bit | Keccak-224, Keccak-256, Keccak-384, Keccak-512 |
| **SHAKE** | Variable | SHAKE-128, SHAKE-256 (Extendable-Output Functions) |
| **BLAKE2** | 256/512-bit | BLAKE2b, BLAKE2s (high-speed cryptographic hash) |
| **BLAKE3** | 128/160/192/224/256/384/512-bit | Native implementation, extremely fast |
| **LSH** | 256/512-bit | Korean standard hash functions |
| **SM3** | 256-bit | Chinese national standard |

</details>

<details open>
<summary><b>🔶 Tab 3: HAVAL & RIPEMD</b></summary>

| Algorithm | Output Size | Description |
|-----------|------------|-------------|
| **HAVAL** | 128/160/192/224/256-bit | Configurable rounds: 3, 4, or 5 passes |
| **RIPEMD** | 128/160/256/320-bit | RACE Integrity Primitives Evaluation Message Digest |

</details>

<details open>
<summary><b>✅ Tab 4: Checksum & Others</b></summary>

| Algorithm | Output Size | Description |
|-----------|------------|-------------|
| **CRC-8** | 8-bit | Cyclic Redundancy Check |
| **CRC-16** | 16-bit | Cyclic Redundancy Check |
| **CRC-32** | 32-bit | Standard CRC-32 |
| **CRC-32C** | 32-bit | CRC-32 Castagnoli |
| **CRC-64** | 64-bit | 64-bit Cyclic Redundancy Check |
| **Adler-32** | 32-bit | Fast checksum algorithm |
| **Tiger** | 192-bit | Designed for 64-bit platforms |
| **Whirlpool** | 512-bit | ISO/IEC standard hash function |

</details>

### 🎨 User Interface Features
- **Tabbed Interface**: Organized algorithm selection across 4 intuitive tabs
- **Stay on Top**: Keep the window visible while working with other applications
- **Progress Bar**: Visual progress indicator with Windows taskbar integration
- **Batch Operations**: Select multiple algorithms and compute all at once
- **Select All/Clear All**: Quick buttons to manage algorithm selection
- **Configuration Persistence**: Remembers your last selected algorithms and settings
- **Unicode Support**: Full Unicode support for international file paths and text

## System Requirements

| Component | Requirement |
|-----------|-------------|
| **Operating System** | Windows 7 or later (Windows 10/11 recommended) |
| **Architecture** | x64 or x86 |
| **Processor** | Any modern CPU with 1 GHz or higher |
| **Memory** | 50 MB RAM minimum |
| **Disk Space** | 10 MB for installation |
| **Build Tools** | Visual Studio 2019+, CMake 3.20+, vcpkg |

---

## Installation

### Prerequisites

Before building HashCalc, ensure you have the following installed:

- ✅ **Visual Studio 2019 or later** with C++ workload
- ✅ **CMake 3.20 or later**
- ✅ **vcpkg** (for dependency management)
- ✅ **Git** (for cloning the repository)

### Setting up vcpkg

If you don't have vcpkg installed:

```bash
# Clone vcpkg repository
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
.\bootstrap-vcpkg.bat
```

> **Tip**: Add vcpkg to your system PATH or use the integration script for easier usage.

### Build Instructions

Follow these steps to build HashCalc:

#### Step 1: Clone the repository

```bash
git clone https://github.com/mystique/hash-calc.git
cd hash-calc
```

#### Step 2: Install dependencies

```bash
vcpkg install cryptopp:x64-windows
```

> **Note**: Replace `x64-windows` with `x86-windows` if building for 32-bit.

#### Step 3: Configure and build with CMake

```bash
# Using CMake presets (recommended)
cmake --preset default
cmake --build build --config Release
```

#### Step 4: Run the application

```bash
./build/Release/HashCalc.exe
```

### Alternative Build (Manual Configuration)

If CMake presets don't work on your system:

```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

Replace `[vcpkg-root]` with your vcpkg installation path.

---

## Usage

### Computing File Hash

1. **Launch HashCalc**
2. **Select algorithms** - Choose desired hash algorithms from the checkboxes
   - Use **"Select All"** button to select all algorithms at once
   - Use **"Clear All"** button to deselect all
3. **Choose file** - Click **"Browse"** to select a file, or simply **drag-and-drop** a file onto the window
4. **Calculate** - Click the **"Calculate"** button to compute the hashes
5. **View results** - Hash values will be displayed in the output area

> **💡 Pro Tip**: Enable "Stay on Top" to keep HashCalc visible while copying hash values to other applications.

### Computing Text Hash

1. **Launch HashCalc**
2. **Switch to Text mode** - Select the **"Text"** radio button
3. **Enter text** - Type or paste your text into the input field
4. **Select algorithms** - Choose desired hash algorithms from the checkboxes
5. **Calculate** - Click the **"Calculate"** button to compute the hashes
6. **View results** - Hash values will be displayed in the output area

### Verifying File Integrity

To verify that a file hasn't been tampered with:

1. **Compute the hash** of your file using HashCalc
2. **Compare** the displayed hash with the expected hash value (from a trusted source)
3. **Verify** - If the hashes match exactly, the file integrity is confirmed ✅

### Tips for Efficient Use

| Feature | Description |
|---------|-------------|
| **🖱️ Drag & Drop** | Simply drag files onto the HashCalc window for instant hashing |
| **📌 Stay on Top** | Enable this option to keep HashCalc visible while working |
| **⚡ Multi-Algorithm** | Select multiple algorithms to compute all hashes simultaneously |
| **💾 Auto-Save** | Your last selected algorithms are automatically saved for next time |
| **📊 Progress Bar** | Visual progress indicator integrated with Windows taskbar |

---

## Technical Details

### Technology Stack

| Component | Technology | Purpose |
|-----------|-----------|---------|
| **Language** | C++17 | Core application logic |
| **GUI Framework** | Win32++ | Modern C++ wrapper for Windows API |
| **Cryptography** | Crypto++ | Industry-standard cryptographic library |
| **Build System** | CMake | Cross-platform build configuration |
| **Package Manager** | vcpkg | Dependency management |
| **Architecture** | Factory Pattern | Extensible algorithm registration |

### Architecture Overview

The application follows a clean, layered architecture with clear separation of concerns:

```
┌─────────────────────────────────────────────────────────────┐
│                    UI Layer                                 │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Win32++ Dialogs & Controls                         │   │
│  │  • HashCalcDialog (Main Window)                     │   │
│  │  • HoverButton (Custom Controls)                    │   │
│  │  • Resource Management (Icons, Dialogs)             │   │
│  └─────────────────────────────────────────────────────┘   │
└───────────────────────────┬─────────────────────────────────┘
                            │
┌───────────────────────────▼─────────────────────────────────┐
│                    Core Layer                               │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Hash Algorithm Factory                             │   │
│  │  • HashAlgorithmFactory (Factory Pattern)           │   │
│  │  • IHashAlgorithm (Abstract Interface)              │   │
│  │  • Algorithm Registration System                    │   │
│  └─────────────────────────────────────────────────────┘   │
└───────────────────────────┬─────────────────────────────────┘
                            │
┌───────────────────────────▼─────────────────────────────────┐
│              Implementation Layer                           │
│  ┌──────────────────────┐  ┌──────────────────────────┐    │
│  │  Crypto++ Algorithms │  │  Native C++ Algorithms   │    │
│  │  • SHA-1, SHA-2      │  │  • BLAKE3 (Pure C++)     │    │
│  │  • SHA-3, Keccak     │  │  • MD6 (Native)          │    │
│  │  • BLAKE2, SHAKE     │  │  • HAVAL (Native)        │    │
│  │  • MD2, MD4, MD5     │  │  • CRC Family (Native)   │    │
│  │  • RIPEMD, Tiger     │  │                          │    │
│  │  • Whirlpool, SM3    │  │                          │    │
│  └──────────────────────┘  └──────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

### Key Components

| Component | Responsibility |
|-----------|---------------|
| **HashAlgorithmFactory** | Factory pattern for creating hash algorithm instances with automatic registration |
| **IHashAlgorithm** | Abstract interface defining the contract for all hash algorithm implementations |
| **HashCalcDialog** | Main UI window handling user interactions, file operations, and threading |
| **ConfigManager** | Manages application configuration persistence via Windows Registry |
| **EditUtils** | Utility functions for text and edit control operations |

### Algorithm Implementations

The application includes both **Crypto++-based** and **native C++ implementations**:

#### 📦 Crypto++ Algorithms

Leveraging the industry-standard Crypto++ library:

- **SHA Family**: SHA-1, SHA-2 (224/256/384/512)
- **SHA-3 Family**: SHA-3 (224/256/384/512), Keccak (224/256/384/512)
- **SHAKE**: SHAKE-128, SHAKE-256 (Extendable-Output Functions)
- **BLAKE2**: BLAKE2b, BLAKE2s
- **MD Family**: MD2, MD4, MD5
- **RIPEMD**: RIPEMD-128, RIPEMD-160, RIPEMD-256, RIPEMD-320
- **Others**: Tiger, Whirlpool, SM3, LSH-256, LSH-512
- **Checksums**: CRC-32, Adler-32

#### ⚡ Native C++ Implementations

Custom implementations for performance and flexibility:

<details>
<summary><b>BLAKE3</b> - Ultra-fast cryptographic hash</summary>

**Location**: `src/core/impl/native/BLAKE3.{h,cpp}`

- Pure C++ implementation, independent of reference C code
- Supports multiple output sizes: 128, 160, 192, 224, 256, 384, 512-bit
- **Performance**: Faster than MD5, SHA-1, SHA-2, and SHA-3
- **Security**: Provides at least SHA-3 level security
- Fully integrated with Crypto++ `HashTransformation` interface
- Optimized for modern CPUs with SIMD support

</details>

<details>
<summary><b>MD6</b> - Flexible hash function</summary>

**Location**: `src/core/impl/native/MD6.{h,cpp}`

- Full C++ implementation of the MD6 hash function
- Supports multiple digest sizes: 128, 160, 192, 224, 256, 384, 512-bit
- Configurable rounds and parallel processing
- Compatible with Crypto++ interface
- Designed for high security and performance

</details>

<details>
<summary><b>HAVAL</b> - Variable-length hash</summary>

**Location**: `src/core/impl/native/Haval.{h,cpp}`

- Native implementation with configurable rounds (3, 4, or 5 passes)
- Supports multiple digest sizes: 128, 160, 192, 224, 256-bit
- Integrated with Crypto++ interface
- Flexible security/performance trade-off

</details>

<details>
<summary><b>CRC Family</b> - Fast checksums</summary>

**Location**: `src/core/impl/native/CRC.{h,cpp}`

- Various CRC variants: CRC-8, CRC-16, CRC-32C, CRC-64
- Native implementations optimized for performance
- Consistent interface with other hash algorithms
- Ideal for error detection and data integrity checks

</details>

> **Note**: All native implementations are fully integrated with Crypto++'s `HashTransformation` interface for consistency and ease of use.

### Threading Model

HashCalc uses a multi-threaded architecture for responsive UI:

| Thread | Purpose | Details |
|--------|---------|---------|
| **Main Thread** | UI Management | Handles all UI events and user interactions |
| **Worker Thread** | Hash Computation | Performs hash calculations in the background |
| **Synchronization** | Thread Safety | Uses atomic flags for cancellation and coordination |
| **Progress Updates** | UI Feedback | Windows messages for thread-safe UI updates |

**Benefits**:
- ✅ UI remains responsive during long hash calculations
- ✅ Users can cancel operations at any time
- ✅ Real-time progress updates with taskbar integration
- ✅ Safe concurrent access to shared resources

### Configuration Management

| Aspect | Implementation |
|--------|---------------|
| **Storage** | Windows Registry |
| **Persistence** | Saves selected algorithms and window state |
| **Auto-Load** | Restores previous settings on startup |
| **Registry Key** | `HKEY_CURRENT_USER\Software\HashCalc` |

---

## Project Structure

```
hash-calc/
├── 📄 CMakeLists.txt              # Main CMake build configuration
├── 📄 CMakePresets.json           # CMake presets for easy building
├── 📄 README.md                   # Project documentation
├── 📄 LICENSE                     # MIT License
│
├── 📁 lib/
│   └── win32xx/                   # Win32++ library files
│
├── 📁 res/                        # Application resources
│   ├── app.ico                    # Application icon
│   ├── app.manifest               # Windows manifest
│   ├── resource.h                 # Resource definitions
│   └── resource.rc                # Resource script
│
└── 📁 src/                        # Source code
    ├── 📁 app/
    │   └── main.cpp               # Application entry point
    │
    ├── 📁 core/                   # Core hash algorithm logic
    │   ├── HashAlgorithmFactory.{h,cpp}
    │   ├── IHashAlgorithm.{h,cpp}
    │   │
    │   └── 📁 impl/               # Algorithm implementations
    │       │
    │       ├── 📁 cryptopp/       # Crypto++ based implementations
    │       │   ├── CryptoppHashBase.h
    │       │   ├── SHAAlgorithms.{h,cpp}
    │       │   ├── ModernAlgorithms.{h,cpp}
    │       │   ├── OtherAlgorithms.{h,cpp}
    │       │   └── ExoticAlgorithms.{h,cpp}
    │       │
    │       └── 📁 native/         # Native C++ implementations
    │           ├── BLAKE3.{h,cpp}     # BLAKE3 hash
    │           ├── MD6.{h,cpp}        # MD6 hash
    │           ├── Haval.{h,cpp}      # HAVAL hash
    │           └── CRC.{h,cpp}        # CRC checksums
    │
    ├── 📁 ui/                     # User interface components
    │   ├── HashCalcDialog.{h,cpp}     # Main dialog window
    │   └── HoverButton.{h,cpp}        # Custom button control
    │
    └── 📁 utils/                  # Utility functions
        ├── ConfigManager.{h,cpp}      # Configuration management
        └── EditUtils.{h,cpp}          # Text utilities
```

---

## Development

### Adding a New Hash Algorithm

To extend HashCalc with a new hash algorithm:

#### Option 1: Using Crypto++ (Recommended for standard algorithms)

```cpp
// 1. Create a new class using CryptoppHashBase template
class MyNewHash : public CryptoppHashBase<CryptoPP::MyNewHashAlgorithm> {
public:
    std::string getName() const override { return "MyNewHash"; }
};

// 2. Register the algorithm
static HashAlgorithmRegistrar<MyNewHash> registrar("MyNewHash");
```

#### Option 2: Native Implementation (For custom algorithms)

```cpp
// 1. Inherit from IHashAlgorithm
class MyCustomHash : public IHashAlgorithm {
public:
    std::string getName() const override;
    size_t getDigestSize() const override;
    std::vector<uint8_t> computeHash(const std::vector<uint8_t>& data) override;
};

// 2. Implement the required methods
// 3. Register using HashAlgorithmRegistrar
static HashAlgorithmRegistrar<MyCustomHash> registrar("MyCustomHash");
```

#### UI Integration Steps

1. **Add checkbox control** in `res/resource.h`:
   ```cpp
   #define IDC_CHECK_MYNEWH ASH 1234
   ```

2. **Add to resource script** `res/resource.rc`:
   ```
   CONTROL "MyNewHash", IDC_CHECK_MYNEWHASH, "Button", BS_AUTOCHECKBOX | WS_TABSTOP, x, y, w, h
   ```

3. **Update UI logic** in `src/ui/HashCalcDialog.cpp`:
   - Add to algorithm ID lists
   - Handle checkbox state in `GetSelectedAlgorithms()`
   - Update configuration save/load logic

### Code Style Guidelines

| Aspect | Convention | Example |
|--------|-----------|---------|
| **Classes** | PascalCase | `HashCalcDialog`, `IHashAlgorithm` |
| **Functions** | PascalCase | `ComputeHash()`, `GetDigestSize()` |
| **Variables** | camelCase with prefix | `m_bIsCalculating`, `m_strFilePath` |
| **Constants** | UPPER_SNAKE_CASE | `WM_HASH_COMPLETE`, `MAX_BUFFER_SIZE` |
| **Member Variables** | `m_` prefix | `m_hThread`, `m_progressBar` |
| **Pointers** | `p` prefix (optional) | `pHashAlgorithm` |

### Documentation Standards

- Use **Doxygen-style comments** for public APIs
- Document **complex algorithms** with inline comments
- Keep comments **concise and meaningful**
- Update **README.md** when adding new features

### Testing Recommendations

- ✅ Test with various file sizes (small, medium, large)
- ✅ Verify hash outputs against known test vectors
- ✅ Test UI responsiveness during long operations
- ✅ Validate cancellation functionality
- ✅ Check memory usage with large files
- ✅ Test drag-and-drop functionality

---

## Performance

HashCalc is optimized for high performance and efficiency:

| Optimization | Description | Benefit |
|--------------|-------------|---------|
| **🧵 Multi-threading** | Hash calculations run in background thread | UI remains responsive |
| **📦 Buffered I/O** | Efficient buffered file reading | Handles large files smoothly |
| **⚡ Native Implementations** | BLAKE3, MD6, HAVAL, CRC use optimized code | Faster than library alternatives |
| **💾 Memory Efficient** | Processes files in chunks | Low memory footprint even for GB files |
| **🔄 Batch Processing** | Multiple algorithms computed in single pass | Reduced I/O overhead |

### Performance Benchmarks

Approximate performance on modern hardware (SSD, 3+ GHz CPU):

| File Size | Time (Single Algorithm) | Time (All Algorithms) |
|-----------|------------------------|----------------------|
| 1 MB | < 0.1 seconds | < 0.5 seconds |
| 100 MB | < 1 second | < 5 seconds |
| 1 GB | < 10 seconds | < 30 seconds |
| 10 GB | < 100 seconds | < 5 minutes |

> **Note**: Actual performance varies based on hardware, selected algorithms, and storage speed.

---

## Security Considerations

HashCalc is designed with security and privacy in mind:

| Aspect | Implementation |
|--------|---------------|
| **🔒 No Network Access** | Operates entirely offline, no internet connection required |
| **🚫 No Data Collection** | Zero telemetry, no data sent to external servers |
| **📁 File Access** | Only reads files explicitly selected by the user |
| **🔐 Cryptographic Strength** | Uses industry-standard Crypto++ library |
| **🛡️ Open Source** | Full source code available for security audits |
| **✅ No Dependencies** | Minimal external dependencies reduce attack surface |

### Security Best Practices

- ✅ Always verify hashes from **trusted sources**
- ✅ Use **SHA-256 or stronger** for security-critical applications
- ✅ Avoid **MD5 and SHA-1** for cryptographic purposes (legacy support only)
- ✅ Keep HashCalc **updated** to the latest version
- ✅ Verify the **integrity of HashCalc itself** before use

---

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

```
MIT License - Free to use, modify, and distribute
```

---

## Contributing

Contributions are welcome! We appreciate your help in making HashCalc better.

### How to Contribute

1. **🍴 Fork** the repository
2. **🌿 Create** a feature branch
   ```bash
   git checkout -b feature/amazing-feature
   ```
3. **💻 Commit** your changes
   ```bash
   git commit -m 'Add amazing feature'
   ```
4. **📤 Push** to the branch
   ```bash
   git push origin feature/amazing-feature
   ```
5. **🔀 Open** a Pull Request

### Contribution Guidelines

- 📝 **For major changes**, please open an issue first to discuss what you would like to change
- ✅ **Test your changes** thoroughly before submitting
- 📚 **Update documentation** (README, code comments) as needed
- 🎨 **Follow the existing code style** and conventions
- 🔍 **Ensure no security vulnerabilities** are introduced
- 📊 **Include test vectors** for new hash algorithms

### Areas for Contribution

- 🆕 Adding new hash algorithms
- 🐛 Bug fixes and improvements
- 📖 Documentation enhancements
- 🎨 UI/UX improvements
- ⚡ Performance optimizations
- 🌍 Internationalization (i18n)

---

## Support

If you encounter any issues or have questions:

### 🐛 Reporting Issues

1. **Check existing issues** on GitHub to avoid duplicates
2. **Create a new issue** with the following information:
   - 📝 Detailed description of the problem
   - 🔄 Steps to reproduce the issue
   - 💻 Your Windows version and architecture (x64/x86)
   - 📸 Screenshots if applicable
   - 📋 Error messages or logs

### 💬 Getting Help

- 📖 Check the [Usage](#usage) section for common tasks
- 🔍 Search [existing issues](https://github.com/mystique/hash-calc/issues) for solutions
- 💡 Open a [new issue](https://github.com/mystique/hash-calc/issues/new) for help

---

## Changelog

### 🎉 Version 1.1.0 - BLAKE3 Update

**Release Date**: 2026-02

#### ✨ New Features
- **BLAKE3 Algorithm**: Added BLAKE3 cryptographic hash algorithm
  - Pure C++ implementation independent of reference C code
  - Support for multiple output sizes (128/160/192/224/256/384/512-bit)
  - Extremely fast performance (faster than MD5, SHA-1, SHA-2, SHA-3)
  - Integrated with Crypto++ `HashTransformation` interface

#### 📝 Improvements
- Added comprehensive implementation documentation
- Enhanced native algorithm architecture
- Improved code organization and structure

---

### 🚀 Version 1.0.0 - Initial Release

**Release Date**: 2025-12

#### ✨ Features
- ✅ Core hashing functionality for files and text
- ✅ Support for 50+ cryptographic algorithms across 4 categories
- ✅ Multi-algorithm batch processing
- ✅ Drag and drop support
- ✅ Windows taskbar progress integration
- ✅ Configuration persistence via Windows Registry
- ✅ Stay on top window feature
- ✅ Native implementations: MD6, HAVAL, CRC family
- ✅ Crypto++ integration for standard algorithms

---

## Acknowledgments

HashCalc is built on the shoulders of giants. Special thanks to:

| Project | Purpose | License |
|---------|---------|---------|
| [**Win32++**](https://sourceforge.net/projects/win32xx/) | Modern C++ wrapper for Windows API | BSD-like |
| [**Crypto++**](https://www.cryptopp.com/) | Industry-standard cryptographic library | Boost Software License |
| [**vcpkg**](https://github.com/Microsoft/vcpkg) | C++ package manager | MIT |
| [**CMake**](https://cmake.org/) | Cross-platform build system | BSD 3-Clause |

### Additional Credits

- 🎨 Icon design inspired by modern Windows design language
- 📚 Algorithm implementations based on published standards and specifications
- 🌟 Community feedback and contributions

---

## Future Enhancements

Potential features for future releases:

### Planned Features

- [ ] **Hash Comparison Tool** - Compare two files side-by-side
- [ ] **Export Results** - Save results to CSV, JSON, or TXT files
- [ ] **Command-Line Interface** - CLI for batch processing and automation
- [ ] **Hash File Support** - Read/write .md5, .sha256, .sfv files
- [ ] **Context Menu Integration** - Right-click files in Windows Explorer
- [ ] **Dark Mode** - Modern dark theme support
- [ ] **Portable Mode** - Run without installation, store config in app folder

### Under Consideration

- [ ] **Additional Algorithms** - Argon2, scrypt, bcrypt (password hashing)
- [ ] **HMAC Support** - Keyed-hash message authentication codes
- [ ] **Parallel Processing** - Multi-core optimization for large files
- [ ] **Cloud Integration** - Optional hash verification against online databases
- [ ] **Localization** - Multi-language support (Chinese, Japanese, etc.)

> **Want to contribute?** Pick a feature from the list above and open a PR!

---

<div align="center">

## ⭐ Star This Project

If you find HashCalc useful, please consider giving it a star on GitHub!

**Made with ❤️ for developers and security enthusiasts**

[Report Bug](https://github.com/mystique/hash-calc/issues) · [Request Feature](https://github.com/mystique/hash-calc/issues) · [Contribute](https://github.com/mystique/hash-calc/pulls)

</div>