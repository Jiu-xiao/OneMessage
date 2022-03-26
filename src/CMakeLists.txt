cmake_minimum_required(VERSION 3.11)
project(OneMessage)

file(
  GLOB
  ${PROJECT_NAME}_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/om.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/app/om_msg.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/app/om_log.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/app/om_fmt.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/app/om_afl.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/core/om_core.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/comp/om_list.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/comp/om_color.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/comp/om_mutex.c")

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_SOURCES})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/app
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/core
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/comp
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../config)
