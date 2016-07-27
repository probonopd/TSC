#############################################################################
# CMakeLists.txt  -  Build file
#
# Copyright © 2013-2016 The TSC Contributors
#############################################################################
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

########################################
# Configuring CMake

cmake_minimum_required(VERSION 3.0)
project(TSC C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
include(ExternalProject)
include(GNUInstallDirs)

########################################
# TSC version number

# If you get a merge conflict below, always keep "dev" on the
# "devel" branch. The postfix is only to be altered on release
# branches.
set(TSC_VERSION_MAJOR 2)
set(TSC_VERSION_MINOR 1)
set(TSC_VERSION_PATCH 0)
set(TSC_VERSION_POSTFIX "dev")

# Set TSC_VERSION_GIT if possible
execute_process(COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${TSC_SOURCE_DIR}
  OUTPUT_VARIABLE TSC_VERSION_GIT
  OUTPUT_STRIP_TRAILING_WHITESPACE)

###############################################
# Flags & Options

option(ENABLE_MRUBY "Enable the MRuby scripting engine" ON)
option(ENABLE_NLS "Enable translations and localisations" ON)
option(ENABLE_EDITOR "Enable the in-game editor" ON)
option(USE_SYSTEM_TINYCLIPBOARD "Use the system's tinyclipboard library" OFF)

########################################
# Compiler config

# We use some C++11. For portability it's important to not rely on
# compiler-specific extensions, so we'll disable them.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include CEGUI's weird location into the runtime library search path.
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# -Wall for better code quality, and -Wno-deprecated-delcarations to prevent
# useless warnings from not-yet-c++11-ready dependencies (glibmm!).
# The _DEBUG macro is used in TSC's own codebase for debugging code.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-deprecated-declarations")
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG=1")
endif()

# Find out the host triplet if we are crosscompiling.
# This is needed because the mruby build system uses it
# as the output directory name.
if (CMAKE_CROSSCOMPILING)
  get_filename_component(HOST_TRIPLET ${CMAKE_C_COMPILER} NAME)
  string(REGEX REPLACE "(.*)-.*$" "\\1" HOST_TRIPLET "${HOST_TRIPLET}")
endif()

########################################
# Dependencies

# External libraries we depend on
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Gettext)
find_package(PNG REQUIRED)
find_package(Ruby REQUIRED)

if (WIN32)
  set(Boost_USE_STATIC_LIBS ON)
  find_package(Boost 1.50.0 COMPONENTS filesystem chrono thread_win32 system REQUIRED)
else()
  find_package(Boost 1.50.0 COMPONENTS filesystem chrono thread system REQUIRED)
  find_package(X11 REQUIRED)
endif()

# Use pkg-config for those which have no Find module available
pkg_search_module(SFML sfml sfml-all)
pkg_check_modules(LibXmlPP REQUIRED libxml++-2.6)
pkg_check_modules(PCRE REQUIRED libpcre)

# Libraries we can build ourselves under certain cirumstances if missing
include("ProvideTinyclipboard")
include("ProvideCEGUI")
if (ENABLE_MRUBY)
  include("ProvideMRuby")
endif()

###############################################
# Configuration header

configure_file(${TSC_SOURCE_DIR}/config.hpp.in
  ${TSC_BINARY_DIR}/config.hpp)
include_directories(${TSC_BINARY_DIR})

########################################
# Credits file

set_source_files_properties(${TSC_BINARY_DIR}/credits.cpp
  PROPERTIES GENERATED true)
add_custom_command(OUTPUT ${TSC_BINARY_DIR}/credits.cpp
  COMMAND ${RUBY_EXECUTABLE} ${TSC_SOURCE_DIR}/generate_credits.rb ${TSC_BINARY_DIR}/credits.cpp
  DEPENDS ${TSC_SOURCE_DIR}/docs/authors.txt ${TSC_SOURCE_DIR}/docs/specialthanks.txt)

########################################
# Compiler flags by dependencies

include_directories(
  ${SFML_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${LibXmlPP_INCLUDE_DIRS}
  ${Tinyclipboard_INCLUDE_DIRS}
  ${CEGUI_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
  ${PCRE_INCLUDE_DIRS}
  ${MRuby_INCLUDE_DIR}
  ${X11_INCLUDE_DIR}
  ${PNG_INCLUDE_DIRS})

add_definitions(
  ${SFML_CFLAGS})

########################################
# Source files

file(GLOB_RECURSE tsc_sources
  "src/*.cpp"
  "src/*.hpp")

# Windows icon resource
# See http://stackoverflow.com/a/708382
if (WIN32)
  enable_language(RC)
  list(APPEND tsc_sources icon.rc)
endif()


########################################
# Gettext internationalization

# Process all PO files into MO files usable at runtime.
file(GLOB po_files "data/translations/*.po")
gettext_process_po_files(languages ALL PO_FILES ${po_files})

########################################
# Main target

add_executable(tsc ${tsc_sources} ${TSC_BINARY_DIR}/credits.cpp)

target_link_libraries(tsc
  ${CEGUI_LIBRARIES}
  ${SFML_LIBRARIES}
  ${Boost_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${PNG_LIBRARY}
  ${MRuby_LIBRARIES}
  ${Tinyclipboard_LIBRARIES})

if (WIN32)
  target_link_libraries(tsc
    ${LibXmlPP_STATIC_LIBRARIES}
    ${PCRE_STATIC_LIBRARIES})

  add_dependencies(tsc CEGUI)
else()
  target_link_libraries(tsc
    ${LibXmlPP_LIBRARIES}
    ${PCRE_LIBRARIES}
    ${X11_LIBRARIES}
    ${CMAKE_DL_LIBS})
endif()

if (ENABLE_MRUBY)
  add_dependencies(tsc mruby)
endif()

########################################
# Installation instructions

install(TARGETS tsc
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT base)

install(DIRECTORY "${TSC_SOURCE_DIR}/data/campaigns/" # Note trailing slash for content copy
  DESTINATION ${CMAKE_INSTALL_DATADIR}/tsc/campaigns
  COMPONENT campaigns)
install(DIRECTORY "${TSC_SOURCE_DIR}/data/editor/"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/tsc/editor
  COMPONENT base)
install(DIRECTORY "${TSC_SOURCE_DIR}/data/gui/"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/tsc/gui
  COMPONENT base)
install(DIRECTORY "${TSC_SOURCE_DIR}/data/icon/"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/tsc/icon
  COMPONENT base)
install(DIRECTORY "${TSC_SOURCE_DIR}/data/levels/"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/tsc/levels
  COMPONENT levels)
install(DIRECTORY "${TSC_SOURCE_DIR}/data/music/"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/tsc/music
  COMPONENT music)
install(DIRECTORY "${TSC_SOURCE_DIR}/data/pixmaps/"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/tsc/pixmaps
  COMPONENT base)
install(DIRECTORY "${TSC_SOURCE_DIR}/data/schema/"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/tsc/schema
  COMPONENT base)
install(DIRECTORY "${TSC_SOURCE_DIR}/data/scripting/"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/tsc/scripting
  COMPONENT base)
install(DIRECTORY "${TSC_SOURCE_DIR}/data/sounds/"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/tsc/sounds
  COMPONENT sounds)
install(DIRECTORY "${TSC_SOURCE_DIR}/data/worlds/"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/tsc/worlds
  COMPONENT worlds)

install(FILES "${TSC_SOURCE_DIR}/docs/man/tsc.6"
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man6
  COMPONENT base)
install(FILES "${TSC_SOURCE_DIR}/extras/icons/tsc_icon_16.png"
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/16x16/apps
  RENAME tsc.png
  COMPONENT base)
install(FILES "${TSC_SOURCE_DIR}/extras/icons/tsc_icon_32.png"
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/32x32/apps
  RENAME tsc.png
  COMPONENT base)
install(FILES "${TSC_SOURCE_DIR}/extras/icons/tsc_icon_48.png"
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps
  RENAME tsc.png
  COMPONENT base)
install(FILES "${TSC_SOURCE_DIR}/extras/icons/tsc_icon_64.png"
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps
  RENAME tsc.png
  COMPONENT base)
install(FILES "${TSC_SOURCE_DIR}/extras/icons/tsc_icon_128.png"
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps
  RENAME tsc.png
  COMPONENT base)
install(FILES "${TSC_SOURCE_DIR}/extras/tsc.desktop"
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
  COMPONENT base)

foreach(pofile ${po_files})
  get_filename_component(lang ${pofile} NAME_WE)

  install(FILES "${TSC_BINARY_DIR}/${lang}.gmo" # .gmo is generated by gettext_process_po_files() further above
    DESTINATION ${CMAKE_INSTALL_DATADIR}/tsc/translations/${lang}/LC_MESSAGES
    RENAME "Secretchronicles.mo" # This must match TSC_GETTEXT_DOMAIN from global_game.hpp so Gettext finds the files
    COMPONENT base)
endforeach()

########################################
# CPack

# Main info
set(CPACK_PACKAGE_NAME "TSC")
set(CPACK_PACKAGE_VENDOR "TSC Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TSC - 2-dimensional platform game")
set(CPACK_PACKAGE_VERSION_MAJOR ${TSC_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${TSC_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${TSC_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "TSC")
set(CPACK_PACKAGE_EXECUTABLES tsc "TSC")

# Nicer names
set(CPACK_COMPONENT_BASE_DISPLAY_NAME "Basic application components (required)")
set(CPACK_COMPONENT_CAMPAIGNS_DISPLAY_NAME "Core campaigns")
set(CPACK_COMPONENT_LEVELS_DISPLAY_NAME "Core levels")
set(CPACK_COMPONENT_MUSIC_DISPLAY_NAME "Music files")
set(CPACK_COMPONENT_SOUNDS_DISPLAY_NAME "Sound files")
set(CPACK_COMPONENT_WORLDS_DISPLAY_NAME "Core worlds")

# Descriptions
set(CPACK_COMPONENT_BASE_DESCRIPTION "The tsc program itself and its core requirements.")
set(CPACK_COMPONENT_CAMPAIGNS_DESCRIPTION "Standard campaigns included with TSC.")
set(CPACK_COMPONENT_LEVELS_DESCRIPTION "Standard levels included with TSC.")
set(CPACK_COMPONENT_MUSIC_DESCRIPTION "Background music files. No background music without this.")
set(CPACK_COMPONENT_SOUNDS_DESCRIPTION "Sound files. No sounds without this.")
set(CPACK_COMPONENT_WORLDS_DESCRIPTION "Standard worlds included with TSC.")

# Interdependencies
set(CPACK_COMPONENT_CAMPAIGNS_DEPENDS worlds)
set(CPACK_COMPONENT_WORLDS_DEPENDS levels)

# Required marks
set(CPACK_COMPONENT_BASE_REQUIRED ON)

# Misc
set(CPACK_RESOURCE_FILE_LICENSE "${TSC_SOURCE_DIR}/docs/license.txt")
set(CPACK_CREATE_DESKTOP_LINKS tsc)
set(CPACK_NSIS_MODIFY_PATH "ON") # Required for desktop shortcuts
set(CPACK_NSIS_MUI_ICON "${TSC_SOURCE_DIR}/extras/icons/windows_icon.ico") # Installer icon
set(CPACK_NSIS_MUI_UNIICON "${TSC_SOURCE_DIR}/extras/icons/windows_icon.ico") # Uninstaller icon
set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\tsc.exe") # Icon for system control panel
set(CPACK_PACKAGE_ICON "${TSC_SOURCE_DIR}/extras/icons/windows_icon.ico") # Installer window bar icon

include(CPack)

########################################
# Print summary

message(STATUS "--------------- Build info summary ----------------")
message(STATUS "TSC version:    ${TSC_VERSION_MAJOR}.${TSC_VERSION_MINOR}.${TSC_VERSION_PATCH}${TSC_VERSION_POSTFIX}")
message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")
if (CMAKE_CROSSCOMPILING)
  message(STATUS "Crosscompiling: ${HOST_TRIPLET}")
else()
  message(STATUS "Crosscompiling: No")
endif()

message(STATUS "--------------- Configuration summary -------------")
message(STATUS "Enable the in-game editor:         ${ENABLE_EDITOR}")
message(STATUS "Enable the mruby scripting engine: ${ENABLE_MRUBY}")
message(STATUS "Enable native language support:    ${ENABLE_NLS}")
message(STATUS "Use system-provided tinyclipboard: ${USE_SYSTEM_TINYCLIPBOARD}")

message(STATUS "--------------- Path configuration -----------------")
message(STATUS "Install prefix:        ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Binary directory:      ${CMAKE_INSTALL_BINDIR}")
message(STATUS "Shared data directory: ${CMAKE_INSTALL_DATADIR}")
message(STATUS "Manpage directory:     ${CMAKE_INSTALL_MANDIR}")
message(STATUS "(relative pathes are relative to the install prefix;")
message(STATUS "the directory /tsc will be appended to the shared")
message(STATUS "data directory indicated above.)")
message(STATUS "----------------------------------------------------")
