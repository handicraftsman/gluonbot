cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(GluonBot VERSION 0.1.0 LANGUAGES C)


if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /usr/ CACHE PATH "Default install prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
endif()

set(CMAKE_C_STANDARD 11)


find_package(PkgConfig)
pkg_search_module(Tiny REQUIRED tiny>=0.6.0)
pkg_search_module(TinyLog REQUIRED tiny-log>=0.1.0)
pkg_search_module(XML REQUIRED libxml-2.0)
pkg_search_module(SQLite REQUIRED sqlite3)
pkg_search_module(PCRE REQUIRED libpcre2-8)
set(_LIBRARIES ${Tiny_LIBRARIES} ${TinyLog_LIBRARIES} ${XML_LIBRARIES} ${SQLite_LIBRARIES} ${PCRE_LIBRARIES} dl pthread)
set(_INCLUDE_DIRS ${Tiny_INCLUDE_DIRS} ${TinyLog_INCLUDE_DIRS} ${XML_INCLUDE_DIRS} ${SQLite_INCLUDE_DIRS} ${PCRE_INLCUDE_DIRS})
include_directories(. ${_INCLUDE_DIRS} ${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR} ${_INCLUDE_DIRS} ${CMAKE_BINARY_DIR})


find_package(Doxygen REQUIRED)
configure_file(${CMAKE_SOURCE_DIR}/Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile)
add_custom_target(
  doc ALL
  COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  VERBATIM
)


file(GLOB lgb_SRC gluonbot/*.c gluonbot/Event/*.c gluonbot/utils/utils.c)
add_library(libgluonbot SHARED ${lgb_SRC})
set_target_properties(libgluonbot PROPERTIES OUTPUT_NAME gluonbot)
target_link_libraries(libgluonbot ${_LIBRARIES})

file(GLOB gb_SRC bin/*.c)
add_executable(gluonbot ${gb_SRC})
target_link_libraries(gluonbot libgluonbot)

file(GLOB gb_core_SRC core/*.c)
add_library(gb-core SHARED ${gb_core_SRC})
target_link_libraries(gb-core libgluonbot)

file(GLOB gb_chanop_SRC chanop/*.c)
add_library(gb-chanop SHARED ${gb_chanop_SRC})
target_link_libraries(gb-chanop libgluonbot)


configure_file(${CMAKE_SOURCE_DIR}/gluonbot.pc.in ${CMAKE_BINARY_DIR}/gluonbot.pc)
install(FILES ${CMAKE_BINARY_DIR}/gluonbot.pc DESTINATION lib/pkgconfig)

install(
  TARGETS gluonbot libgluonbot gb-core gb-chanop 
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)

install(FILES gluonbot.h gluonbot-plugin.h DESTINATION include)
file(GLOB gb_HDR gluonbot/*.h)
install(FILES ${gb_HDR} DESTINATION include/gluonbot)
