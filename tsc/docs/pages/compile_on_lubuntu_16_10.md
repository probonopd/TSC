# Compilation instructions for Lubuntu 16.10

Time-stamp: <2016-08-03 09:53:19 xet7>

[ TODO: Maybe this should be extracted to a Wiki page? ]

This document is intended as an augmentation to INSTALL.md with short
and concise instructions on how to get TSC directly from Git (devel
branch) up and running on Lubuntu 16.10.

1) Install dependencies:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sudo apt install ruby-full rake gperf pkg-config bison libglew-dev \
  freeglut3-dev gettext libpng-dev libpcre3-dev libxml++2.6-dev \
  libfreetype6-dev libdevil-dev libboost1.58-all-dev libsfml-dev \
  libcegui-mk2-dev cmake build-essential git git-core
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

2) Install rubygems for documentation generation etc:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sudo gem install bundler nanoc adsf kramdown coderay
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

3) Clone TSC and build it:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
git clone https://github.com/Secretchronicles/TSC.git
cd TSC
git submodule init && git submodule update
cd tsc && mkdir build && cd build
rm -rf ~/tsc && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=~/tsc .. && make && make install
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

4) Run TSC:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
cd ~/tsc
./bin/tsc
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

5) Keep your own levels at local directory, so cleanup below does not delete them:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ls ~/.local/share/tsc
ls ~/.local/share/tsc/levels
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

6) Update, cleanup and run again:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
cd ~/TSC/tsc/build
rm -rf *
git pull
rm -rf ~/tsc && cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=~/tsc .. && make && make install
rm -rf ~/.cache/tsc ~/.config/tsc
cd ~/tsc
./bin/tsc
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
