#!/usr/bin/env python3
"""
Apply clang-format to all .h and .cpp files in the PathTracer project.
Excludes external library sources.
"""

import os
import subprocess
import sys
from pathlib import Path


def find_source_files(root_dir):
    """
    Find all .h and .cpp files, excluding external libraries.
    """
    root = Path(root_dir)
    source_files = []
    
    exclude_dirs = {'external', '.git', '_build_temp', '_cmake_build', 'bin', '.vs'}
    
    for path in root.rglob('*'):
        # Skip directories in exclude list
        if any(part in exclude_dirs for part in path.parts):
            continue
        
        # Only process .h and .cpp files
        if path.suffix in {'.h', '.cpp'} and path.is_file():
            source_files.append(path)
    
    return sorted(source_files)


def apply_clang_format(file_path, config_file):
    """
    Apply clang-format to a single file.
    Returns True if successful, False otherwise.
    """
    try:
        # Use clang-format with the specified config file
        result = subprocess.run(
            ['clang-format', '-i', f'--style=file:{config_file}', str(file_path)],
            capture_output=True,
            text=True,
            timeout=30
        )
        
        if result.returncode != 0:
            print(f"ERROR processing {file_path}: {result.stderr}", file=sys.stderr)
            return False
        
        return True
    except subprocess.TimeoutExpired:
        print(f"TIMEOUT processing {file_path}", file=sys.stderr)
        return False
    except FileNotFoundError:
        print("ERROR: clang-format not found. Please install clang-format.", file=sys.stderr)
        return False
    except Exception as e:
        print(f"ERROR processing {file_path}: {e}", file=sys.stderr)
        return False


def main():
    """
    Main entry point.
    """
    # Get the project root (parent of FANCY folder)
    script_dir = Path(__file__).parent
    project_root = script_dir.parent.parent
    
    # Path to clang-format config
    config_file = project_root / '.clang-format'
    
    if not config_file.exists():
        print(f"ERROR: .clang-format config not found at {config_file}", file=sys.stderr)
        sys.exit(1)
    
    # Find all source files
    source_files = find_source_files(project_root)
    
    if not source_files:
        print("No source files found to format.")
        return
    
    print(f"Found {len(source_files)} source files to format.")
    
    # Apply clang-format to each file
    success_count = 0
    fail_count = 0
    
    for i, file_path in enumerate(source_files, 1):
        print(f"[{i}/{len(source_files)}] Formatting {file_path.relative_to(project_root)}...")
        
        if apply_clang_format(file_path, config_file):
            success_count += 1
        else:
            fail_count += 1
    
    # Print summary
    print(f"\n{'='*60}")
    print(f"Formatting complete:")
    print(f"  Successful: {success_count}")
    print(f"  Failed:     {fail_count}")
    print(f"{'='*60}")
    
    if fail_count > 0:
        sys.exit(1)


if __name__ == '__main__':
    main()
