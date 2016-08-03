message("-- Scripting engine enabled: building mruby statically")

# mruby requires Bison and ruby to compile. Ruby is checked
# for on the toplevel anyway.
find_package(BISON REQUIRED)

# The CROSSCOMPILE_TARGET environment variable is used by our custom
# mruby_tsc_build_config.rb build script for mruby. It is not part
# of mruby's official build documentation.
ExternalProject_Add(
  mruby
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E copy_directory "${TSC_SOURCE_DIR}/../mruby/mruby" "${TSC_BINARY_DIR}/mruby"
  SOURCE_DIR "${TSC_BINARY_DIR}/mruby"
  CONFIGURE_COMMAND ""
  BUILD_IN_SOURCE 1
  BUILD_COMMAND ./minirake MRUBY_CONFIG=${TSC_SOURCE_DIR}/mruby_tsc_build_config.rb CROSSCOMPILE_TARGET=${HOST_TRIPLET}
  INSTALL_COMMAND "")

set(MRuby_INCLUDE_DIR ${TSC_SOURCE_DIR}/../mruby/mruby/include)

if(CMAKE_CROSSCOMPILING)
  # Appearently no libmruby_core.a library when crosscompiling?
  set(MRuby_LIBRARIES "${TSC_BINARY_DIR}/mruby/build/${HOST_TRIPLET}/lib/libmruby.a")
else()
  set(MRuby_LIBRARIES "${TSC_BINARY_DIR}/mruby/build/host/lib/libmruby.a" "${TSC_BINARY_DIR}/mruby/build/host/lib/libmruby_core.a")
endif()
