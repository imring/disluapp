cmake_minimum_required(VERSION 3.16)

enable_testing()

find_package(GTest CONFIG REQUIRED)
find_package(Threads REQUIRED)

if (MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

function(add_dltest name)
  add_executable(${name}
    main-test.cpp
    ${name}.cpp
  )

  target_include_directories(${name} PRIVATE ../include)
  target_link_libraries(${name} PRIVATE GTest::gtest GTest::gmock)
  if (UNIX)
    target_link_libraries(${name} PRIVATE pthread)
  endif()
  target_compile_features(${name} PRIVATE cxx_std_17)

  add_test(NAME ${name} COMMAND ./${name})
endfunction()

add_dltest(buffer-test)
add_dltest(types-test)