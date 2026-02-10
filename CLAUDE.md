# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

HashCalc is a Windows GUI application for computing file and text hashes using 53+ cryptographic algorithms. It's built with C++17, Win32++, and Crypto++, featuring both GUI and console modes.

## Build System

### Prerequisites
- **Visual Studio 2019+** with C++ workload
- **CMake 3.20+**
- **vcpkg** for dependency management
- **Git**

### Building

```bash
# Install dependencies via vcpkg
vcpkg install cryptopp:x64-windows

# Configure and build (using CMake presets)
cmake --preset default
cmake --build build --config Release

# Alternative: Manual configuration
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release

# Run the application
./build/Release/HashCalc.exe
```

### Build Presets
- **default**: Uses vcpkg toolchain, requires `VCPKG_ROOT` environment variable
- **release**: Release configuration build
- **debug**: Debug configuration build

### Key Build Settings
- Target: WIN32 executable (no console window by default)
- Unicode enabled (`UNICODE`, `_UNICODE`)
- Auto-discovers all `.cpp`/`.h` files in `src/`
- Links: `comctl32`, `uxtheme`, `version`, `cryptopp`
- Resource compilation includes `res/resource.rc` and `res/app.manifest`

## Architecture

### Core Design Pattern: Factory with Auto-Registration

The application uses a **Factory Pattern** with automatic registration for extensibility:

```
┌─────────────────────────────────────────────────┐
│         HashAlgorithmFactory                    │
│  • create(name) → unique_ptr<IHashAlgorithm>    │
│  • getAvailableAlgorithms() → vector<string>    │
│  • Registry: map<string, Creator>               │
└─────────────────────────────────────────────────┘
                     ▲
                     │ implements
┌─────────────────────────────────────────────────┐
│           IHashAlgorithm (interface)            │
│  • getName() → string                           │
│  • getDigestSize() → size_t                     │
│  • update(data, length)                         │
│  • finalize() → vector<uint8_t>                 │
│  • reset()                                      │
│  • computeFile(path) → vector<uint8_t>          │
│  • computeString(str) → vector<uint8_t>         │
└─────────────────────────────────────────────────┘
                     ▲
        ┌────────────┴────────────┐
        │                         │
┌───────────────────┐   ┌─────────────────────┐
│ CryptoppHashBase  │   │ Native Impls        │
│ (template)        │   │ • BLAKE3            │
│ • SHA-1, SHA-2    │   │ • MD6               │
│ • SHA-3, Keccak   │   │ • HAVAL             │
│ • BLAKE2, SHAKE   │   │ • CRC family        │
│ • MD2/4/5, RIPEMD │   │ • GOST family       │
│ • Tiger, etc.     │   │                     │
└───────────────────┘   └─────────────────────┘
```

**Key Files:**
- [`src/core/HashAlgorithmFactory.{h,cpp}`](src/core/HashAlgorithmFactory.h) - Factory implementation
- [`src/core/IHashAlgorithm.{h,cpp}`](src/core/IHashAlgorithm.h) - Abstract interface
- [`src/core/AlgorithmIds.h`](src/core/AlgorithmIds.h) - Centralized algorithm ID constants

### UI Architecture: Tab-Based with Win32++

```
┌──────────────────────────────────────────────────┐
│         HashCalcDialog (Main Window)             │
│  • CTab control managing 4 tab views             │
│  • System tray integration                       │
│  • File/text input handling                      │
│  • Worker thread for hash computation            │
│  • Progress bar & taskbar integration            │
└──────────────────────────────────────────────────┘
              │
              ├─── TabViewSHA (SHA & MD Family)
              ├─── TabViewSHA3 (SHA-3 & Modern)
              ├─── TabViewHAVAL (HAVAL & RIPEMD)
              └─── TabViewChecksum (Checksum & Others)
                   (All inherit from TabViewBase)
```

**Key Files:**
- [`src/ui/HashCalcDialog.{h,cpp}`](src/ui/HashCalcDialog.h) - Main dialog window
- [`src/ui/TabViewBase.{h,cpp}`](src/ui/TabViewBase.h) - Base class for tab views
- [`src/ui/TabView*.{h,cpp}`](src/ui/) - Individual tab implementations
- [`src/ui/AboutDialog.{h,cpp}`](src/ui/AboutDialog.h) - About dialog

### Directory Structure

```
src/
├── app/
│   └── main.cpp              # Entry point, GUI mode initialization
├── core/
│   ├── HashAlgorithmFactory  # Factory pattern implementation
│   ├── IHashAlgorithm        # Abstract interface
│   ├── AlgorithmIds.h        # Centralized algorithm constants
│   └── impl/
│       ├── cryptopp/         # Crypto++ based implementations
│       │   ├── CryptoppHashBase.h     # Template base class
│       │   ├── SHAAlgorithms.cpp      # SHA-1, SHA-2
│       │   ├── ModernAlgorithms.cpp   # SHA-3, Keccak, SHAKE, BLAKE2
│       │   ├── OtherAlgorithms.cpp    # MD2/4/5, RIPEMD, CRC32, Adler32
│       │   └── ExoticAlgorithms.cpp   # Tiger, Whirlpool, SM3, LSH
│       └── native/           # Pure C++ implementations
│           ├── BLAKE3.cpp    # Ultra-fast hash (multiple output sizes)
│           ├── MD6.cpp       # Flexible hash function
│           ├── Haval.cpp     # Variable-length hash (3/4/5 rounds)
│           ├── CRC.cpp       # CRC-8/16/32C/64 checksums
│           └── GOST.cpp      # Russian standards (GOST-94, Streebog-256/512)
├── ui/
│   ├── HashCalcDialog        # Main window & worker thread management
│   ├── AboutDialog           # About dialog
│   ├── TabViewBase           # Base class for tab views
│   ├── TabView*              # Individual tab implementations
│   └── HoverButton           # Custom button control
└── utils/
    ├── CommandLineParser     # Command-line argument parsing & console mode
    ├── ConfigManager         # Registry-based config persistence
    └── EditUtils             # Text utility functions

lib/win32xx/                  # Win32++ library (bundled)
res/                          # Resources (icons, manifest, .rc files)
```

## Adding New Hash Algorithms

### Option 1: Using Crypto++ (Recommended for standard algorithms)

**Step 1:** Add implementation in `src/core/impl/cryptopp/` (choose appropriate file):

```cpp
// In appropriate *Algorithms.cpp file (e.g., ModernAlgorithms.cpp)
#include "CryptoppHashBase.h"
#include <cryptopp/mynewhashalgo.h>

// Define the class using template base
using MyNewHash = CryptoppHashBase<CryptoPP::MyNewHashAlgorithm>;

// Auto-register using static initializer
static HashAlgorithmRegistrar<MyNewHash> registrar_mynewh("MyNewHash");
```

**Step 2:** Add resource ID in `res/resource.h`:
```cpp
#define IDC_CHECK_MYNEWHASH    1XXX  // Pick unused ID
```

**Step 3:** Add checkbox to appropriate dialog template in `res/resource.rc`:
```
CONTROL "MyNewHash", IDC_CHECK_MYNEWHASH, "Button", BS_AUTOCHECKBOX | WS_TABSTOP, x, y, w, h
```

**Step 4:** Add ID to `src/core/AlgorithmIds.h`:
```cpp
inline constexpr int ALL_ALGORITHM_IDS[] = {
  // ... existing IDs ...
  IDC_CHECK_MYNEWHASH,  // Add here
};
```

**Step 5:** Update appropriate tab view class to handle the checkbox.

### Option 2: Native Implementation (For custom algorithms)

**Step 1:** Create implementation in `src/core/impl/native/MyAlgorithm.{h,cpp}`:

```cpp
// MyAlgorithm.h
#include "core/IHashAlgorithm.h"
#include <cryptopp/cryptlib.h>  // For HashTransformation base

class MyAlgorithm : public CryptoPP::HashTransformation {
public:
  std::string AlgorithmName() const override { return "MyAlgorithm"; }
  unsigned int DigestSize() const override { return 32; }  // bytes
  void Update(const byte* input, size_t length) override { /* ... */ }
  void TruncatedFinal(byte* digest, size_t digestSize) override { /* ... */ }
  // Implement other required methods...
};

// MyAlgorithm.cpp
#include "MyAlgorithm.h"
#include "core/HashAlgorithmFactory.h"
#include "core/impl/cryptopp/CryptoppHashBase.h"

using MyAlgorithmHash = CryptoppHashBase<MyAlgorithm>;
static HashAlgorithmRegistrar<MyAlgorithmHash> registrar_myalgo("MyAlgorithm");
```

**Step 2-5:** Same as Option 1 (add resource ID, dialog control, update AlgorithmIds.h, tab view)

### Important Notes on Algorithm Registration

- **Auto-registration happens at static initialization time** via `HashAlgorithmRegistrar<T>`
- The string name passed to registrar **must match** the name returned by `getName()`
- Names are **case-sensitive** - use consistent naming (e.g., "SHA-256" not "sha-256")
- Algorithm becomes available to both GUI and console modes automatically

## Code Conventions

### Naming Conventions
| Type | Convention | Example |
|------|-----------|---------|
| Classes | PascalCase | `HashCalcDialog`, `IHashAlgorithm` |
| Functions/Methods | PascalCase | `ComputeHash()`, `GetDigestSize()` |
| Member Variables | `m_` + camelCase | `m_bIsCalculating`, `m_strFilePath` |
| Local Variables | camelCase | `fileName`, `digestSize` |
| Constants | UPPER_SNAKE_CASE | `WM_HASH_COMPLETE`, `MAX_BUFFER_SIZE` |
| Namespaces | lowercase | `core`, `ui`, `utils` |

### File Organization
- Header guards: `#ifndef FILENAME_H` / `#define FILENAME_H`
- Namespaces: Use `core`, `ui`, `utils` as appropriate
- Include order: System headers → Win32++ → Crypto++ → Project headers

### Threading Model
- **Main Thread**: UI event handling, window messages
- **Worker Thread**: Hash computation (`std::thread` in HashCalcDialog)
- **Synchronization**: Uses `std::atomic<bool>` for cancellation flags
- **Progress Updates**: PostMessage to main thread for UI updates

## Command-Line Interface

The application supports both GUI and console modes with intelligent mode switching:

### Mode Selection Logic
- **Console Mode**: When using `-t`/`--text` OR `-f`/`--file` flags (requires `-a`/`--algorithm` or `-A`/`--all`)
- **GUI Mode**: All other cases (no flags, input only, or `-a`/`-A` without `-t`/`-f`)

### Console Mode Examples
```bash
# Help and information
HashCalc.exe --help
HashCalc.exe --list

# File hashing
HashCalc.exe -f "file.txt" -a SHA256 -a MD5
HashCalc.exe --file "document.pdf" --algorithm BLAKE3

# Batch processing with all algorithms
HashCalc.exe -f "file.txt" -A
HashCalc.exe -f "file.txt" --all

# Text hashing
HashCalc.exe -t "Hello World" -a SHA256
HashCalc.exe --text "sample" --algorithm MD5

# Lowercase hex output
HashCalc.exe -f "file.txt" -a SHA256 -c
HashCalc.exe -t "text" -A --lowercase
```

### GUI Mode Examples
```bash
# Launch GUI
HashCalc.exe

# GUI with file loaded
HashCalc.exe "file.txt"

# GUI with text loaded
HashCalc.exe "Hello World"

# GUI with pre-selected algorithms
HashCalc.exe "file.txt" -a MD5 -a SHA256

# GUI with all algorithms pre-selected
HashCalc.exe "file.txt" -A
```

### Implementation Details
- **Command-line parsing**: [`src/utils/CommandLineParser.{h,cpp}`](src/utils/CommandLineParser.h)
  - Handles mode detection (GUI vs Console)
  - Parses algorithm parameters (`-a`, `-A`, `-c`)
  - Maps algorithm names between display and factory formats
  - Supports HAVAL pass number extraction (e.g., "HAVAL-3-256")
- **Console management**:
  - Attaches to parent console (PowerShell/CMD) or allocates new one
  - UTF-8 console output support for Unicode characters
  - Smart wait logic: Pauses when console is allocated vs. attached
- **Algorithm validation**: Validates against factory registry
- **Output formatting**: Uppercase (default) or lowercase (`-c`) hex output
- **Performance timing**: Displays calculation time in console mode

## Configuration Management

- **Storage**: Windows Registry (`HKEY_CURRENT_USER\Software\HashCalc`)
- **Managed by**: [`src/utils/ConfigManager`](src/utils/ConfigManager.h)
- **Persists**: Selected algorithms, window state, preferences
- **Auto-saves**: On application exit
- **Auto-loads**: On application startup

## Testing & Validation

When modifying hash algorithms:
1. **Verify against test vectors**: Check algorithm output against published test vectors
2. **Test file sizes**: Small (< 1MB), medium (10-100MB), large (> 1GB)
3. **Test cancellation**: Ensure worker thread responds to cancellation
4. **Verify UI responsiveness**: Hash computation shouldn't freeze UI
5. **Check memory usage**: Monitor for leaks during large file processing

## GitHub CI/CD

The repository includes a GitHub Actions workflow ([`.github/workflows/release.yml`](.github/workflows/release.yml)):

- **Triggers**: On version tags (`v*.*.*`) or manual dispatch
- **Platform**: `windows-latest` runner
- **Steps**:
  1. Checkout with submodules
  2. Setup MSBuild
  3. Setup vcpkg (pinned to stable commit)
  4. Install cryptopp via vcpkg
  5. Configure CMake with vcpkg toolchain
  6. Build Release configuration
  7. Package artifacts
  8. Create GitHub Release with HashCalc.exe

**Manual dispatch**: Can trigger build with custom version number via GitHub Actions UI

## Common Development Tasks

### Rebuilding after code changes
```bash
cmake --build build --config Release
```

### Cleaning build directory
```bash
rm -rf build
cmake --preset default
cmake --build build --config Release
```

### Debugging
- Use Debug configuration: `cmake --build build --config Debug`
- Debug executable: `build/Debug/HashCalc.exe`
- Attach Visual Studio debugger to running process

### Updating dependencies
```bash
vcpkg update
vcpkg upgrade cryptopp:x64-windows
```

## Key Implementation Notes

### Win32++ Integration
- Win32++ is a header-only C++ wrapper around Windows API
- Located in `lib/win32xx/`
- Main classes used: `CDialog`, `CTab`, `CButton`, `CEdit`, `CStatic`
- All UI classes inherit from Win32++ base classes

### Resource Management
- **resource.h**: Defines all control IDs (IDC_*), dialog IDs (IDD_*)
- **resource.rc**: Defines dialog layouts, strings, icons
- **app.manifest**: Windows manifest for visual styles, DPI awareness
- Resource compilation handled by CMake

### CryptoPP Integration
- Template-based design: `CryptoppHashBase<THashAlgorithm>`
- Wraps CryptoPP's `HashTransformation` interface
- Provides uniform interface via `IHashAlgorithm`
- All Crypto++ algorithms automatically get progress/cancel support

### Native Algorithm Integration
- Must inherit from CryptoPP's `HashTransformation` for consistency
- Implement required methods: `Update()`, `TruncatedFinal()`, `DigestSize()`, `AlgorithmName()`
- Wrap with `CryptoppHashBase<T>` for automatic integration
- Examples: BLAKE3, MD6, HAVAL, CRC family, GOST family

## Troubleshooting

### vcpkg not found
- Ensure `VCPKG_ROOT` environment variable is set
- Or specify manually: `-DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg.cmake`

### Crypto++ link errors
- Verify cryptopp is installed: `vcpkg list cryptopp`
- Reinstall if needed: `vcpkg remove cryptopp:x64-windows && vcpkg install cryptopp:x64-windows`

### Resource compilation errors
- Check `res/resource.h` for duplicate IDs
- Ensure all dialog templates have unique IDD_* identifiers
- Verify control IDs (IDC_*) are unique within their context

### Algorithm not appearing in UI
- Verify registration in .cpp file: `static HashAlgorithmRegistrar<T> registrar_name("AlgorithmName");`
- Check AlgorithmIds.h includes the control ID
- Verify checkbox added to appropriate dialog template
- Confirm tab view handles the checkbox state
