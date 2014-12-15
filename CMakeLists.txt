cmake_minimum_required(VERSION 2.8.12)

# remove unused configuration types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "possible build types" FORCE)

# IMPORTANT: this command must be placed after CMAKE_CONFIGURATION_TYPES is changed
project(video_marker)

# CMAKE_BUILD_TYPE must not be used with non make-based generators (we only consider MSVC here)
# FIXME: we need a generic way of checking whether a generator is make-based or not
if(MSVC_IDE)
  if(CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "CMAKE_BUILD_TYPE is not supported by MSVC generator")
  endif()
else()
  if(CMAKE_BUILD_TYPE)
    if(NOT(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Release"))
      message(FATAL_ERROR "Invalid CMAKE_BUILD_TYPE. Possible values: Release [default], Debug")
    endif()
  else()
    message(STATUS "Using default build type: Release")
    set(CMAKE_BUILD_TYPE "Release")
  endif()
endif()

# --- architecture ---
if(WIN32)
  set(VIDEO_MARKER_OPERATING_SYSTEM "win")
elseif(UNIX)
  set(VIDEO_MARKER_OPERATING_SYSTEM "linux")
else()
  message(FATAL_ERROR "Unknown operating system")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL "4")
  set(VIDEO_MARKER_CPU_BITNESS "32")
elseif(CMAKE_SIZEOF_VOID_P EQUAL "8")
  set(VIDEO_MARKER_CPU_BITNESS "64")
else()
  message(FATAL_ERROR "Unsupported CPU bitness")
endif()

set(VIDEO_MARKER_ARCH ${VIDEO_MARKER_OPERATING_SYSTEM}${VIDEO_MARKER_CPU_BITNESS})
message(STATUS "Target architecture: ${VIDEO_MARKER_ARCH}")

# directories
set(VIDEO_MARKER_LIB_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/lib/${VIDEO_MARKER_ARCH})
if(WIN32 AND NOT MSVC_IDE)
  set(VIDEO_MARKER_LIB_ROOT ${VIDEO_MARKER_LIB_ROOT} ${VIDEO_MARKER_LIB_ROOT}/${CMAKE_BUILD_TYPE})
endif()

# compiler settings
if(MSVC)
  # suppress some Visual Studio warnings
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -DNOMINMAX)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fno-omit-frame-pointer -Wno-attributes -pthread -std=gnu99")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fno-omit-frame-pointer -Wno-attributes -pthread")

  add_definitions(-DCOMFORT_GNU_COMPILER)

  # Strange, but this fixes some troubles with undefined references.
  # TODO: investigate issues
  add_definitions(-Wl,--no-undefined)
endif()

# global include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
if(MSVC)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/compat)
endif()

# global link directories
link_directories(${VIDEO_MARKER_LIB_ROOT})

# output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin/${VIDEO_MARKER_ARCH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin/${VIDEO_MARKER_ARCH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib/${VIDEO_MARKER_ARCH}/debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/${VIDEO_MARKER_ARCH}/release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib/${VIDEO_MARKER_ARCH}/debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/${VIDEO_MARKER_ARCH}/release)

add_subdirectory(pugixml)
add_subdirectory(src)
