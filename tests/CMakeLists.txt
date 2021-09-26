cmake_minimum_required(VERSION 3.16)

find_package(GTest CONFIG REQUIRED)
find_package(Threads REQUIRED)

function(add_dltest file)
  string(REGEX REPLACE "[/\\]" "-" name "${file}")
  add_executable(${name}
    main-test.cpp
    ./${file}.cpp
  )

  target_link_libraries(${name} PRIVATE dislua GTest::gmock GTest::gtest
    GTest::gmock_main GTest::gtest_main)
  if (UNIX)
    target_link_libraries(${name} PRIVATE pthread)
  endif()
  target_compile_features(${name} PRIVATE cxx_std_20)

  add_test(NAME ${name} COMMAND ./${name})
endfunction()

add_dltest(buffer-test)
add_dltest(types-test)
add_dltest(lj/parse-test)