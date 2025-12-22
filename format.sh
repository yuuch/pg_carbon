#!/bin/bash
# Find all .c, .h, .cpp, .hpp files and format them
find src  -name "*.h" -o -name "*.c" -o -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i -style=file
echo "Formatting complete."
