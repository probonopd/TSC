message("-- Scripting engine enabled: building mruby statically")

# mruby requires Bison and ruby to compile. Ruby is checked
# for on the toplevel anyway.
find_package(BISON REQUIRED)

ExternalProject_Add(
  mruby
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E copy_directory "${TSC_SOURCE_DIR}/../mruby/mruby" "${TSC_BINARY_DIR}/mruby"
  SOURCE_DIR "${TSC_BINARY_DIR}/mruby"
  CONFIGURE_COMMAND ""
  BUILD_IN_SOURCE 1
  BUILD_COMMAND ./minirake MRUBY_CONFIG=${TSC_SOURCE_DIR}/mruby_tsc_build_config.rb CROSSCOMPILE_TARGET=${CMAKE_C_COMPILER_TARGET}
  INSTALL_COMMAND "")

set(MRuby_INCLUDE_DIR ${TSC_SOURCE_DIR}/../mruby/mruby/include)

if(CMAKE_CROSSCOMPILING)
  set(MRuby_LIBRARIES "${TSC_BINARY_DIR}/mruby/build/${CMAKE_C_COMPILER_TARGET}/lib/libmruby.a" "${TSC_BINARY_DIR}/mruby/build/${CMAKE_C_COMPILER_TARGET}/lib/libmruby_core.a")
else()
  set(MRuby_LIBRARIES "${TSC_BINARY_DIR}/mruby/build/host/lib/libmruby.a" "${TSC_BINARY_DIR}/mruby/build/host/lib/libmruby_core.a")
endif()
