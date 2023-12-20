#!/bin/bash

# Check if the correct number of arguments is provided
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <source_file1.c> [source_file2.c ...]"
    exit 1
fi

# Compilation step
for source_file in "$@"; do
    echo "Compiling: $source_file"
    gcc -c "$source_file" -o "${source_file%.c}.o"
done

# Linking step for server
server_executable="serveurTCP"
echo "Linking: $server_executable"
gcc "serveurTCP.o" -o "$server_executable"

# Linking step for client
client_executable="clientTCP"
echo "Linking: $client_executable"
gcc "clientTCP.o" -o "$client_executable"

echo "Build completed successfully!"
