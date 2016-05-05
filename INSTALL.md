Installation instructions for TSC
=================================

TSC uses [CMake][1] as the build system, so the first thing you have to
ensure is that you have CMake installed.

TSC currently supports the Linux and Windows platforms officially. To
be more exact, it is tested against Arch Linux and Windows 7. The
current Ubuntu LTS should also work. **Windows XP is unsupported**.

TSC can be installed either from Git, meaning that you clone the
repository, or from a release tarball, where for the purpose of this
document a beta release is considered a release. Finally, you have the
possibility to cross-compile to Windows from Linux either from Git or
from a release tarball. Each of these possibilities will be covered
after we have had a look on the dependencies. Note that if you want
to crosscompile, you should probably read this entire file and not
just the section on crosscompilation to get a better understanding.

Dependencies
------------

In any case, you will have to install a number of dependencies before
you can try installing TSC itself. The following sections list the
dependencies for each supported system.

### Common dependencies ###

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
* The libxml++ library.
* The Freetype library.
* Boost >= 1.50.0 (to be exact: boost_system, boost_filesystem, boost_thread)
* Simple and fast cross-platform multimedia library `libsfml-dev` 
* For generating the docs:
  * `kramdown` RubyGem.
  * The `coderay` RubyGem.
  * The `dot` program.
  * The `doxygen` program.
  * Ruby’s `rdoc` program.

### Linux dependencies ###

* The DevIL library.

Additionally, TSC needs CEGUI version 0.7.x. However, as this old
version is not provided by most modern Linux distributions anymore, the
build system has been set up to download and compile it on its own and
then link it in statically. **For Windows** (see below) this does not
hold true, you have to provide CEGUI 0.7.x libraries yourself (or just
use MXE as described below). We are working on the issue and hope to
get it resolved with the next feature release of TSC.

The following commandline installs all dependencies required to built
TSC on Ubuntu Linux 14.04:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# apt-get install ruby-full rake gperf pkg-config bison libglew-dev \
  freeglut3-dev gettext libpng12-dev libpcre3-dev libxml++2.6-dev \
  libfreetype6-dev libdevil-dev libboost1.55-all-dev libsfml-dev
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Windows dependencies ###

* The FreeImage library.
* CEGUI version 0.7.x. CEGUI 0.8.x is not yet supported.
* For generating a setup installer:
  * The NSIS tools.

Configuration options
---------------------

This section describes possible configuration you may apply before
building. If you just want to build the game, you can skip it. If you
want custom configuration or want to package it for e.g. a Linux
distribution, read on.

You may pass the following optional variables to the `cmake` command:

* `FIXED_DATA_DIR=/some/dir`: Overrides the default installation path for the
  game data with this fixed directory.
* `BINARY_DIR=/some/dir`: Overrides the default installation path for
  the game binary.

Especially if you are packaging, you will most likely also find it
useful to execute the install step like this:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
make DESTDIR=/some/dir install
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will shift the file system root for installation to `/some/dir`
so that all pathes you gave to `cmake` will be below that path.

Installing from a released tarball
----------------------------------

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

Installing from Git
-------------------

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

Crosscompiling from Linux to Windows
------------------------------------

TSC can be crosscompiled from Linux to Windows, such that you don’t
have to even touch a Windows system in order to generate the
executable that will run on Windows, and indeed this is how we produce
the Windows releases. Regardless whether you compile
from Git or from a release tarball, you will need a crosscompilation
toolchain for that. We recommend you to use [MXE][2] for that, which
includes all dependencies necessary for building TSC.

MXE is an ever-evolving distribution, so it’s better to use a version
that is known to work. For this, we maintain [a slightly changed fork
of MXE](https://github.com/Secretchronicles/mxe) that contains a
branch named `tsc-building`. This branch is known to work for a
successful crosscompilation, and contains only very little adjustments
to upstream MXE.

The following commands download our MXE and checks out the
`tsc-building` branch.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ mkdir ~/tsc-cross
$ cd ~/tsc-cross
$ git clone git://github.com/Secretchronicles/mxe.git
$ cd mxe
$ git checkout tsc-building
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

After that you can build MXE with all dependencies required for
buildint TSC:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ make boost libxml++ glew cegui libpng freeimage sdl sdl_image sdl_mixer sdl_ttf nsis
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will take a long time.

There’s a little annoyence with CMake not finding `pkg-config` and
`makensis` in MXE’s default directory setup. We have added a small
new task to MXE’s makefile to correct that, which you now need to run:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ make fixcmakelinks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Crosscompiling from a released tarball ###

Crosscompiling from Linux to Windows works similar as native
compilation, except you have to tell CMake where your crosscompilation
toolchain resides. First, extract the tarball and prepare a build
directory as usual:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ tar -xvJf TSC-*.tar.xz
$ cd TSC-*/tsc
$ mkdir crossbuild
$ cd crossbuild
$ cp ../cmake/toolchains/linux2mingw32.cmake .
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The last step copied the toolchain file `tsc/cmake/toolchains/linux2mingw32.cmake`
to your crossbuild directory.  Edit that file to point to your MXE installation,
which should be `~/tsc-building/mxe` if you followed the above steps. For this,
ensure the `CMAKE_FIND_ROOT_PATH` line is correct:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
set(CMAKE_FIND_ROOT_PATH "$ENV{HOME}/tsc-building/mxe")
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Then build TSC. Be sure to include the new parameter
`-DCMAKE_TOOLCHAIN_FILE` as shown below to make CMake aware you want a
crosscompilation with the toolchain file you just edited. Again, you
may or may not include `-DCMAKE_BUILD_TYPE=Debug` depending on whether
you want debugging symbols or not.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
$ export PATH=$HOME/tsc-building/mxe/usr/bin:$PATH
$ cmake -DCMAKE_TOOLCHAIN_FILE=./linux2mingw32.cmake \
  -DCMAKE_INSTALL_PREFIX=$PWD/testinstall ..
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
$ cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/linux2mingw32.cmake ..
$ make
$ cpack -G NSIS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will create a `TSC-x.y.z-win32.exe` file. This file is the
ready-to-distribute setup installer.

Note that you shouldn’t install multiple versions of TSC at once using
the setup installer. Uninstall any previous version of TSC before
installing with another setup installer; the standalone approach does
not suffer from this problem.

### Crosscompiling from Git ###

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
