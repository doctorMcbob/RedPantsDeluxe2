#!/bin/bash

# Set the path to your executable and destination lib directory
executable="./build/game"
lib_dir="./lib/"

# Use ldd to get a list of libraries
libraries=$(ldd "$executable" | awk '/=>/ {print $3}' | while read -r line; do echo $line; done)

# Create the lib directory if it doesn't exist
if [ ! -d "$lib_dir" ]; then
    mkdir -p "$lib_dir"
fi

# Loop through the libraries and copy them to the lib directory
for lib in $libraries; do
    cp "$lib" "$lib_dir"
done

