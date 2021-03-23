include("${CMAKE_CURRENT_LIST_DIR}/dislua-targets.cmake")

if (TARGET dislua)
  get_target_property(DISLUA_INCLUDE_DIR dislua INTERFACE_INCLUDE_DIRECTORIES)
endif()