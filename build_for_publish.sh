#!/bin/sh

echo "Input release version: "
read -r version
if ! echo "$version" | grep -qE "^[0-9]+\.[0-9]+$"; then
    echo "ERROR: Invalid version"
    exit 1
fi
build_dir="build"
binary_dir="$build_dir/bin"

# Run CMake and compile
mkdir -p "$build_dir"
cmake -S . -B "$build_dir" -DCMAKE_BUILD_TYPE=Release
cmake --build "$build_dir" -- -j "$(nproc)"

# Pack
echo "$version" > "$binary_dir/version.txt"
zip --junk-paths "Charon_linux.zip" "$binary_dir/Charon" "$binary_dir/version.txt"
