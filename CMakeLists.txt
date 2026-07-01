cmake_minimum_required(VERSION 3.18)

project(
	WebsocketProtocolLib
	LANGUAGES CXX
	VERSION 1.0.0
	DESCRIPTION "Simple C++ library implementing Websocket Protocol"
)

########################################################################

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror -fPIC")

########################################################################

set(wsProtocolLib "WebsocketProtocol")

option(ENABLE_SHARED "Build shared (dynamic) library? If set to off, a static library will be built" OFF)

########################################################################

# Add a postfix for debug library builds to avoid name clashes with release builds

#if(WIN32)
#	set(CMAKE_DEBUG_POSTFIX d)
#endif()

set(LIBRARY_TYPE "STATIC")

if("${ENABLE_SHARED}")
	# Create a shared (dynamic) library for the WebsocketProtocol class
	set(LIBRARY_TYPE "SHARED")
endif()

######################################################################

add_library(
	"${wsProtocolLib}"
	"${LIBRARY_TYPE}"
	websocket_protocol.cpp
)

add_library(wsProtocol::Lib ALIAS "${wsProtocolLib}")

if (MSVC)
	# add correct compiler flags so that we can correctly import/export symbols
	if("${LIBRARY_TYPE}" STREQUAL "SHARED")
		target_compile_definitions("${wsProtocolLib}" PRIVATE WSPDLL)
	else()
		target_compile_definitions("${wsProtocolLib}" PRIVATE WSPLIB)
	endif()
endif()

set_target_properties(
	"${wsProtocolLib}"
	PROPERTIES
	OUTPUT_NAME wsProtocol
	EXPORT_NAME Lib
)

include(GNUInstallDirs)

target_include_directories(
	"${wsProtocolLib}"
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:"${CMAKE_INSTALL_INCLUDEDIR}">
)


target_include_directories(
	${wsProtocolLib}
	PRIVATE
   "${CMAKE_CURRENT_SOURCE_DIR}/utilities" # sha1.h
)


set_target_properties(
	${wsProtocolLib}
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)

########################################################################

# Outputs

install(
	TARGETS ${wsProtocolLib}
	EXPORT ${wsProtocolLib}Targets
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"	
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
	INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# Headers

install(
	FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/WebsocketProtocol/websocket_protocol.h"
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/WebsocketProtocol"
)

#[[
# OR all the headers in the include dir:
#install(
#  DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
#  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
#  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
#)
#]]

# Exporting the Target Information

install(
	EXPORT ${wsProtocolLib}Targets
	FILE ${wsProtocolLib}Targets.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${wsProtocolLib}
	NAMESPACE wsProtocol::
)


# Use a standard helper module for packaging
include(CMakePackageConfigHelpers) 

set(__PACKAGE_INIT__ "@PACKAGE_INIT@")
set(_TARGET_NAME "${wsProtocolLib}")

configure_file(
	"cmake/template.cmake.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/WebsocketProtocolLibConfig.cmake.in" 
	@ONLY
)

# Generate the main Config file from our template
configure_package_config_file(
	"cmake/WebsocketProtocolLibConfig.cmake.in"
	"${wsProtocolLib}Config.cmake"
	INSTALL_DESTINATION
	"${CMAKE_INSTALL_LIBDIR}/cmake/${wsProtocolLib}"
)


# Generate a version file for version checking
write_basic_package_version_file(
	"${wsProtocolLib}ConfigVersion.cmake" 
  VERSION ${PROJECT_VERSION} 
  COMPATIBILITY AnyNewerVersion 
)

# Install the generated config and version files
install(FILES 
	"${CMAKE_CURRENT_BINARY_DIR}/${wsProtocolLib}Config.cmake" 
	"${CMAKE_CURRENT_BINARY_DIR}/${wsProtocolLib}ConfigVersion.cmake" 
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${wsProtocolLib}" 
)
#c]]

########################################################################



