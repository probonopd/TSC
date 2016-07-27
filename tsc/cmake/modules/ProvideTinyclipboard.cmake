if (USE_SYSTEM_TINYCLIPBOARD)
  find_file(Tinyclipboard_INCLUDE_DIRS tinyclipboard.h)
  find_library(Tinyclipboard_LIBRARIES tinyclipboard)
  message("-- Found tinyclipboard at ${Tinyclipboard_LIBRARIES}")
else()
  message("-- Building tinyclipboard statically")
  add_library(tinyclipboard STATIC
    "${TSC_SOURCE_DIR}/../tinyclipboard/src/tinyclipboard.c"
    "${TSC_SOURCE_DIR}/../tinyclipboard/include/tinyclipboard.h")

  set(Tinyclipboard_LIBRARIES tinyclipboard)
  set(Tinyclipboard_INCLUDE_DIRS "${TSC_SOURCE_DIR}/../tinyclipboard/include")
endif()