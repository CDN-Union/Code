#!/bin/bash
# Change this if your clang-format executable is somewhere else
CLANG_FORMAT="$HOME/Library/Application Support/Alcatraz/Plug-ins/ClangFormat/bin/clang-format"

find . \( -name '*.h' -or -name '*.c' -or -name '*.mm' \) -print0 | xargs -0 "$CLANG_FORMAT" -i
