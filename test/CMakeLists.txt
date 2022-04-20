cmake_minimum_required(VERSION 3.11)
project(Test)

find_package(Check REQUIRED)
include_directories(${CHECK_INCLUDE_DIRS})
link_directories(${CHECK_LIBRARY_DIRS})

file(GLOB ${PROJECT_NAME}_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_SOURCES})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC $<TARGET_PROPERTY:OneMessage,INTERFACE_INCLUDE_DIRECTORIES>)

set(TEST_SOURCES om_test_main.c)

add_executable(om_test ${TEST_SOURCES})

target_link_libraries(
  om_test
  PRIVATE ${PROJECT_NAME}
  PRIVATE OneMessage
  PRIVATE ${CHECK_LIBRARIES}
  PRIVATE subunit
  PRIVATE pthread
  PRIVATE rt
  PRIVATE m)
