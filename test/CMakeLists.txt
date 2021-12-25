include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

find_package(Check REQUIRED)
include_directories(${CHECK_INCLUDE_DIRS})
link_directories(${CHECK_LIBRARY_DIRS})

set(TEST_SOURCES
  om_test_main.c
  om_test_case.c
)

add_executable(om_test ${TEST_SOURCES})
target_link_libraries(om_test m OneMessage ${CHECK_LIBRARIES})
