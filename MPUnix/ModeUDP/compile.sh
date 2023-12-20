#!/bin/bash

# Script to compile and link the client and server programs

# Check the number of arguments
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <client_source> <server_source>"
    exit 1
fi

# Assign the source file names to variables
CLIENT_SOURCE="$1"
SERVER_SOURCE="$2"

# Extract the base names without extensions
CLIENT_BASE=$(basename -s .c "$CLIENT_SOURCE")
SERVER_BASE=$(basename -s .c "$SERVER_SOURCE")

# Compile and link the client program
gcc -o "$CLIENT_BASE" "$CLIENT_SOURCE" -Wall
if [ $? -ne 0 ]; then
    echo "Compilation failed for $CLIENT_SOURCE"
    exit 1
fi

# Compile and link the server program
gcc -o "$SERVER_BASE" "$SERVER_SOURCE" -Wall
if [ $? -ne 0 ]; then
    echo "Compilation failed for $SERVER_SOURCE"
    exit 1
fi

echo "Compilation and linking successful for both client and server programs."
