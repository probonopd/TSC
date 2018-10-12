#!/bin/bash

# Debian 9

git submodule init

git submodule update

sudo echo "deb http://ftp.debian.org/debian stretch-backports main" > /etc/apt/sources.list.d/backports.list

sudo apt-get update

sudo apt-get install -t stretch-backports flatpak flatpak-builder

flatpak install flathub org.gnome.Builder

flatpak install flathub org.freedesktop.Sdk//18.08
