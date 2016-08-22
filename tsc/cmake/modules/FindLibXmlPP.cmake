#############################################################################
# FindLibXmlPP.cmake - CMake module for finding libxml++
#
# Copyright © 2016 The TSC Contributors
#############################################################################
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

pkg_check_modules(PKG_LibXmlPP QUIET libxml++-2.6)

# Note we use a subdirectory find (libxml++/document.h) since that is what
# we include. Many other libs may have a document.h file, so one definitely
# does not want that subdirectory into the compiler include search path
# (prevents conflicts).
find_path(LibXmlPP_INCLUDE_DIR libxml++/document.h
  HINTS ${PKG_LibXmlPP_INCLUDE_DIRS} /usr/include/libxml++-2.6)

find_library(LibXmlPP_LIBRARY
  NAMES xml++ xml++-2.6
  HINTS ${PKG_LibXmlPP_LIBRARY_DIRS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibXmlPP DEFAULT_MSG LibXmlPP_INCLUDE_DIR LibXmlPP_LIBRARY)

set(LibXmlPP_LIBRARIES ${LibXmlPP_LIBRARY})
set(LibXmlPP_INCLUDE_DIRS ${LibXmlPP_INCLUDE_DIR})

if (PKG_LibXmlPP_CFLAGS)
  set(LibXmlPP_DEFINITIONS ${PKG_LibXmlPP_CFLAGS})
endif()

mark_as_advanced(LibXmlPP_LIBRARIES LibXmlPP_INCLUDE_DIRS LibXmlPP_DEFINITIONS)
