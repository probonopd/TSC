#!/usr/bin/env bash

set -e

mkdir install
export INSTALL_PREFIX=$PWD/install

hg clone -b v0-8 https://bitbucket.org/cegui/cegui
cd cegui
mkdir build
cd build
cmake -G Ninja .. -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX
ninja install
cd ../..

curl -Lo sfml.tgz https://www.sfml-dev.org/files/SFML-2.4.2-linux-gcc-64-bit.tar.gz
tar xvf sfml.tgz
cp -r SFML-2.4.2/* $INSTALL_PREFIX

mkdir build
cd build
cmake -G Ninja ../tsc -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX

sed -i 's:"$INSTALL_PREFIX":/usr:' config.hpp
ninja install -j3

if [ "$TRAVIS_SUDO" == "true" ]; then
    echo "Building AppImage..."

    mv $INSTALL_PREFIX usr
    curl -Lo functions.sh https://raw.githubusercontent.com/probonopd/AppImages/master/functions.sh
    . ./functions.sh

    patch_usr
    copy_deps

    export APP=TSC
    export VERSION=2.0.0
    mkdir TSC.AppDir
    mv usr TSC.AppDir
    cp ../tsc/extras/tsc.desktop TSC.AppDir/TSC.desktop

    cd TSC.AppDir
    get_apprun
    cd ..

    generate_appimage
fi
