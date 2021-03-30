cmake_minimum_required(VERSION 3.16)

project(dislua)

include(GNUInstallDirs)

option(BUILD_TEST "Build tests." ON)
option(BUILD_DOC "Build the Doxygen documentation." OFF)

if (BUILD_TEST)
  add_subdirectory(tests)
endif()

if (BUILD_DOC)
  find_package(Doxygen)
  if (DOXYGEN_FOUND)
    message(${CMAKE_CURRENT_LIST_DIR}/Doxyfile)
    add_custom_target(doc
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs/html"
            DESTINATION ${CMAKE_INSTALL_DOCDIR})
  else()
    message(WARNING "Doxygen not found, unable to generate documentation")
  endif()
endif()

add_library(dislua INTERFACE)
target_include_directories(dislua INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}> $<INSTALL_INTERFACE:include>)

install(TARGETS dislua EXPORT dislua)

install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include/dislua"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES "${CMAKE_CURRENT_LIST_DIR}/LICENSE"
              "${CMAKE_CURRENT_LIST_DIR}/README.md"
        DESTINATION ${CMAKE_INSTALL_DOCDIR})

install(FILES cmake/dislua-config.cmake
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dislua")
install(EXPORT dislua FILE dislua-targets.cmake
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/dislua")