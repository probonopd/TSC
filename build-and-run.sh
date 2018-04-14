#!/bin/bash

# Get latest git submodules
git submodule init
git submodule update

# Change to tsc directory
cd tsc

# Delete old build files
rm -rf build
mkdir build
cd build

# Delete old game directory at %HOME/tsc
rm -rf ~/tsc

# Build TSC
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=~/tsc ..
make

# Install TSC to $HOME/tsc
make install

# Changing to game directory
cd ~/tsc

# Start TSC
./bin/tsc
