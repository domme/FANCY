# PathTracer Scripts

## apply_clang_format.py

Automatically applies clang-format to all C++ source files in the PathTracer project.

### Usage

```bash
python apply_clang_format.py
```

Or directly:

```bash
./apply_clang_format.py
```

### Features

- **Recursive search**: Scans all subdirectories for `.h` and `.cpp` files
- **External library exclusion**: Automatically excludes:
  - `external/` directory
  - `_build_temp/` and `_cmake_build/` directories
  - `bin/`, `.git/`, `.vs/` directories
- **Centralized config**: Uses the `.clang-format` file at the project root
- **Progress reporting**: Shows progress and summary of formatting

### Configuration

The formatting style is defined in `.clang-format` at the project root. Key settings:
- **AlignConsecutiveDeclarations**: Aligns variable names in consecutive declarations with tabs
- **ColumnLimit**: 120 characters
- **IndentWidth**: 2 spaces
- **PointerAlignment**: Middle (e.g., `Type * name`)

### Requirements

- **clang-format**: Must be installed and available in PATH
  - Windows: Install via Visual Studio, LLVM, or package manager
  - macOS: `brew install clang-format`
  - Linux: `sudo apt-get install clang-format` (Ubuntu/Debian)

### Exit Codes

- `0`: All files formatted successfully
- `1`: One or more files failed to format
