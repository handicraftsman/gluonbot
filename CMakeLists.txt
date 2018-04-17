cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(GluonBot VERSION 0.1.0 LANGUAGES C)

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX /usr/ CACHE PATH "Default install prefix" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CMAKE_C_STANDARD 11)


find_package(PkgConfig)
pkg_search_module(Tiny REQUIRED tiny>=0.6.0)
pkg_search_module(TinyLog REQUIRED tiny-log>=0.1.0)
pkg_search_module(XML REQUIRED libxml-2.0)
set(_LIBRARIES ${Tiny_LIBRARIES} ${TinyLog_LIBRARIES} ${XML_LIBRARIES})
set(_INCLUDE_DIRS ${Tiny_INCLUDE_DIRS} ${TinyLog_INCLUDE_DIRS} ${XML_INCLUDE_DIRS})
include_directories(${_INCLUDE_DIRS} ${CMAKE_BINARY_DIR})

file(GLOB lgb_SRC gluonbot/*.c gluonbot/events/*.c)
add_library(libgluonbot SHARED ${lgb_SRC})
set_target_properties(libgluonbot PROPERTIES OUTPUT_NAME gluonbot)
target_link_libraries(libgluonbot ${_LIBRARIES})

file(GLOB gb_SRC bin/*.c)
add_executable(gluonbot ${gb_SRC})
target_link_libraries(gluonbot libgluonbot)
target_include_directories(gluonbot PRIVATE ${Tiny_INCLUDE_DIRS})

configure_file(${CMAKE_SOURCE_DIR}/gluonbot.pc.in ${CMAKE_BINARY_DIR}/gluonbot.pc)
install(FILES ${CMAKE_BINARY_DIR}/gluonbot.pc DESTINATION lib/pkgconfig)

install(
  TARGETS gluonbot libgluonbot
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

install(FILES gluonbot.h DESTINATION include)
file(GLOB gb_HDR gluonbot/*.h)
install(FILES ${gb_HDR} DESTINATION include/gluonbot)
