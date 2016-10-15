Installation instructions for TSC
=================================

Time-stamp: <2016-08-03 10:13:02 quintus>

TSC uses [CMake][1] as the build system, so the first thing you have to
ensure is that you have CMake installed.

TSC supports the Linux and Windows platforms officially.
On Windows, testing is done on Windows 7.
**Windows XP is unsupported**.

TSC can be installed either from Git, meaning that you clone the
repository, or from a release tarball, where for the purpose of this
document a beta release is considered a release. Finally, you have the
possibility to cross-compile to Windows from Linux either from Git or
from a release tarball. Each of these possibilities will be covered
after we have had a look on the dependencies. Note that if you want
to crosscompile, you should probably read this entire file and not
just the section on crosscompilation to get a better understanding.

Installation instructions tailored specifically towards compiling TSC
from Git on Lubuntu 16.10 can be found in the separate file
tsc/docs/pages/compile_on_lubuntu_16_10.md.

Contents
--------

I. Dependencies
    1. Common dependencies
    2. Linux dependencies
    3. Windows dependencies
II. Configuration options
III. Installing from a released tarball
IV. Installing from Git
V. Upgrade notices
VI. Crosscompiling from Linux to Windows
    1. Crosscompiling from a released tarball
    2. Crosscompiling from Git














I. Dependencies
----------------

In any case, you will have to install a number of dependencies before
you can try installing TSC itself. The following sections list the
dependencies for each supported system.




### 1. Common dependencies ###

The following dependencies are required regardless of the system you
install to.

* A Ruby 1.9 or 2.0 installation with `rake` in your PATH.
* The `gperf` program.
* The `pkg-config` program.
* The `bison` program.
* OpenGL.
* GLEW OpenGL wrangler extension library.
* GNU Gettext.
* The LibPNG library.
* The libPCRE regular expression library.
* The libxml++ library < 3.0.0. Versions >= 3.0.0 will not be
  supported until the libxml++ developers provide a porting guide
  from version 2.8 to version 3.0.
* The Freetype library.
* CEGUI >= 0.8.0
  * If you have glm 0.9.6 or newer, you need CEGUI >= 0.8.5
    due to CEGUI bug #1063 (https://bitbucket.org/cegui/cegui/issues/1063).
* Boost >= 1.50.0 (to be exact: boost_system, boost_filesystem, boost_thread)
* SFML >= 2.3.0
* Optionally for generating the docs:
  * `kramdown` RubyGem.
  * The `coderay` RubyGem.
  * The `dot` program.
  * The `doxygen` program.
  * Ruby’s `rdoc` program.




### 2. Linux dependencies ###

* The DevIL library.

#### Example for Ubuntu ####

(specific instructions for Lubuntu 16.10 can be found in
tsc/docs/pages/compile_on_lubuntu_16_10.md).

Install core dependencies:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sudo apt install ruby-full rake gperf pkg-config bison libglew-dev \
  freeglut3-dev gettext libpng-dev libpcre3-dev libxml++2.6-dev \
  libfreetype6-dev libdevil-dev libboost1.58-all-dev libsfml-dev \
  libcegui-mk2-dev cmake build-essential git git-core
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Install rubygems for documentation generation etc (optional; you need
this only if you want the docs):

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sudo gem install kramdown coderay
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~




### 3. Windows dependencies ###

* The FreeImage library.
* For generating a setup installer:
  * The NSIS tools.













II. Configuration options
-------------------------

This section describes possible configuration you may apply before
building. If you just want to build the game, you can skip it. If you
want custom configuration or want to package it for e.g. a Linux
distribution, read on. Each of the flags described in this section
needs to be passed when invoking cmake by use of a `-D` option,
e.g. `-DENABLE_EDITOR=OFF`. Default values are indicated in brackets.

The following options are available:

ENABLE_EDITOR [ON]
: Enables or disables the in-game editor. Switching this off is not
  yet supported.

ENABLE_MRUBY [ON]
: Enables or disables the scripting engine. Switching this off is not
  yet supported.

ENABLE_NLS [ON]
: Enables or disables use of translations. If disabled, TSC will use
  English only.

USE_SYSTEM_TINYCLIPBOARD [OFF]
: For clipboard access, TSC uses the `tinyclipboard` library written
  by Marvin Gülker (Quintus). The library is not part of many Linux
  distributions yet, so it is build as part of building TSC itself
  as a static library. If you *are* on a Linux distribution where
  this library is packaged, set this value to ON and the build
  system will dynamically link to the tinyclipboard library of
  the system and not build its own variant.

The following path options are available:

CMAKE_INSTALL_PREFIX [/usr/local]
: Prefix value for all other CMAKE_INSTALL variables.

CMAKE_INSTALL_BINDIR [(prefix)/bin]
: Binary directory, i.e. where the `tsc` executable will be installed.
  Do not change this option when compiling for Windows (see below
  for further information).

CMAKE_INSTALL_DATADIR [(prefix)/share]
: Directory where the main data (levels, graphics, etc.) will be
  installed. Do not change this option when compiling for
  Windows (see below for further information).

CMAKE_INSTALL_DATAROOTDIR [(prefix)/share]
: Installation target of non-program-specific data, namely the
  `.desktop` starter file, icons and the manpage by default.
  Only change this if you have good reasons.

CMAKE_INSTALL_MANDIR [(datarootdir)/man]
: Where TSC will install its manpage under. Note that the
  installer will create a subdirectory `man6`, i.e. the
  manpage is installed into (mandir)/man6/tsc.6.

**Do not change CMAKE_INSTALL_BINDIR or CMAKE_INSTALL_DATADIR when
compiling for Windows**! The `tsc` executable when run on Windows
searches for the data directory relative to its own path on the
filesystem, and it assumes the default layout. If you change this
option, the `tsc` executable will be unable to locate the data
directory and crash. On all other systems, the `tsc` executable will
take value of `CMAKE_INSTALL_DATADIR` as the data directory, hence you
can change it to your likening (handy for Linux distribution
packagers).

Especially if you are packaging, you will most likely also find it
useful to execute the install step like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
make DESTDIR=/some/dir install
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will shift the file system root for installation to `/some/dir`
so that all pathes you gave to `cmake` will be below that path.













III. Installing from a released tarball
----------------------------------------

Extract the tarball, create a directory for the build and switch into
it:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ tar -xvJf TSC-*.tar.xz
$ cd TSC-*/tsc
$ mkdir build
$ cd build
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Execute `cmake` to configure and `make` to build and install TSC. Be
sure to replace `/opt/tsc` with the directory you want TSC to install
into.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ cmake -DCMAKE_INSTALL_PREFIX=/opt/tsc ..
$ make
# make install
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you want or are asked to, add `-DCMAKE_BUILD_TYPE=Debug` as a
parameter to `cmake` in order to build a version with debugging
symbols. These are needed by the developers to track down bugs more
easily.

After the last command finishes, you will find a `bin/tsc` executable
file below your chosen install directory. Execute it in order to start
TSC.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ /opt/tsc/bin/tsc
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~













IV. Installing from Git
-----------------------

Installing from Git basically works the same way as the normal release
install, but with a few preparations needed. You have to clone the
repository, and initialize the Git submodules before you can continue
with the real build process. These preprations can be done as follows:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ git clone git://github.com/Secretchronicles/TSC.git
$ cd TSC
$ git submodule init
$ git submodule update
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

From there on, you can continue with the normal instructions as per
the above section.













V. Upgrade notices
------------------

Before upgrading TSC to a newer released version or new development
version from Git, you may want to make a backup of your locally
created levels and worlds. You can do this by copying the directory
`~/.locals/share/tsc` to a safe place.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ cp -r ~/.local/share/tsc ~/backup-tsc
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To upgrade your Git copy of TSC:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ git pull
$ git submodule update
$ cmake ..
$ make
$ make install
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you switch branches (maybe because you want to test a specific new
feature not merged into `devel` yet), it is recommended to clean the
build directory as well.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ cd ..
$ git checkout feature-branch
$ rm -rf build
$ mkdir build
$ cd build
$ cmake [OPTIONS] ..
$ make
$ make install
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~













VI. Crosscompiling from Linux to Windows
-----------------------------------------

TSC can be crosscompiled from Linux to Windows, such that you don’t
have to even touch a Windows system in order to generate the
executable that will run on Windows, and indeed this is how we produce
the Windows releases. Regardless whether you compile
from Git or from a release tarball, you will need a crosscompilation
toolchain for that. We recommend you to use [MXE][2] for that, which
includes all dependencies necessary for building TSC.

MXE is an ever-evolving distribution, so it’s better to use a version
that is known to work. For this, we maintain [a fork
of MXE](https://github.com/Secretchronicles/mxe) that contains a
branch named `tsc-crosscompile`. This branch is known to work for a
successful crosscompilation.

The following commands download our MXE and check out the
`tsc-crosscompile` branch.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ mkdir ~/tsc-cross
$ cd ~/tsc-cross
$ git clone git://github.com/Secretchronicles/mxe.git
$ cd mxe
$ git checkout tsc-crosscompile
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

After that you can build MXE with all dependencies required for
building TSC:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ make boost libxml++ glew cegui libpng freeimage sfml nsis
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will take a long time (about an hour on my machine).




### 1. Crosscompiling from a released tarball ###

Crosscompiling from Linux to Windows works similar to native
compilation, except you have invoke CMake a little differently. Start
as usual, but use another directory for the crosscompilation build
than the native one:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ tar -xvJf TSC-*.tar.xz
$ cd TSC-*/tsc
$ mkdir crossbuild
$ cd crossbuild
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now add your crosscompilation toolchain to the PATH environment
variable so CMake can find it.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ export PATH=$HOME/tsc-cross/mxe/usr/bin:$PATH
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It now is time to invoke CMake. If you use MXE for crosscompilation as
it is recommended, you have a special CMake wrapper
`i686-w64-mingw32.static-cmake` available which you can use now:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ i686-w64-mingw32.static-cmake \
  -DCMAKE_BUILD_TYPE=Debug # If you want a debug build
  -DCMAKE_INSTALL_PREFIX=$PWD/testinstall ..
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you do *not* use MXE, you need to invoke it as usual (i.e. bare
"cmake"), but you have to specify a "toolchain file" manually by
passing `-DCMAKE_TOOLCHAIN_FILE` to CMake. Refer to the CMake
documentation or your crosscompilation toolchain's documentation in
that case.

Either way, you can now continue with the actual build:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ make
$ make install
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will give you a Windows TSC installation in the
crossbuild/testinstall directory. Copy it to Windows or run it with
Wine:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
% wine testinstall/bin/tsc.exe
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#### Generating a windows setup installer ####

The above method will yield a directory `testinstall/` that is
standalone e.g. for distribution in form of a ZIP file. Creating a
setup installer that registers TSC with the registry requires a
slightly different approach. If you built TSC already with the above
method, clear your `crossbuild` directory to prevent artifacts.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ rm -rf *
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Follow the above guide up until and including adding the MXE tools to
your PATH variable (the `export PATH=...` line). Then, execute the
build commands like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ i686-w64-mingw32.static-cmake ..
$ make
$ i686-w64-mingw32.static-cpack -G NSIS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will create a `TSC-x.y.z-win32.exe` file. This file is the
ready-to-distribute setup installer.

Note that you shouldn’t install multiple versions of TSC at once using
the setup installer. Uninstall any previous version of TSC before
installing with another setup installer; the standalone approach does
not suffer from this problem.




### 2. Crosscompiling from Git ###

Clone the Git repository and execute the preparation steps. They are
the same as for a normal non-cross build.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ git clone git://github.com/Secretchronicles/TSC.git
$ cd TSC
$ git submodule init
$ git submodule update
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Then continue with “Crosscompiling from a released tarball” above.

[1]: http://cmake.org
[2]: http://mxe.cc
