cmake_minimum_required(VERSION 3.16)

project(dislua)

include(GNUInstallDirs)

option(DISLUA_INSTALL "Install the disluapp library" ON)
option(DISLUA_TESTS "Build tests." ON)
option(DISLUA_DOCS "Build the Doxygen documentation." OFF)

if (DISLUA_DOCS)
  find_package(Doxygen)
  if (DOXYGEN_FOUND)
    add_custom_target(docs
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  else()
    message(WARNING "Doxygen not found, unable to generate documentation")
  endif()
endif()

add_library(dislua INTERFACE)
target_include_directories(dislua
  INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
if (MSVC)
  target_compile_options(dislua INTERFACE /EHsc)
else()
  # target_compile_options(dislua INTERFACE -fcxx-exceptions)
endif()
target_compile_features(dislua INTERFACE cxx_std_20)

if (DISLUA_TESTS)
  enable_testing()

  add_subdirectory(tests)
endif()

if (DISLUA_INSTALL)
  install(TARGETS dislua EXPORT dislua-targets)

  install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/dislua"
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  install(FILES "${CMAKE_CURRENT_LIST_DIR}/LICENSE"
                "${CMAKE_CURRENT_LIST_DIR}/README.md"
          DESTINATION ${CMAKE_INSTALL_DOCDIR})

  install(FILES "${PROJECT_SOURCE_DIR}/cmake/dislua-config.cmake"
          DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dislua")
  install(EXPORT dislua-targets
          DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dislua")

  if (DISLUA_DOCS)
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs/html"
            DESTINATION ${CMAKE_INSTALL_DOCDIR})
  endif()
endif()