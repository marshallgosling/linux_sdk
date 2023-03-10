cmake_minimum_required(VERSION 2.4)

project(AgoraSDKDemo)

include(${CMAKE_SOURCE_DIR}/scripts/env.cmake)
include(${CMAKE_SOURCE_DIR}/scripts/os.cmake)
include(${CMAKE_SOURCE_DIR}/scripts/check.cmake)

if("${MEM_CHECK}" STREQUAL "true")
  set(sanitizer -fsanitize=address)
endif()

set(AGORA_MEDIA_SDK_DIR ${CMAKE_SOURCE_DIR}/../agora_sdk)
set(AGORA_MEDIA_SDK_HEADER_DIR ${CMAKE_SOURCE_DIR}/../agora_sdk)

# Set common flags
set(CMAKE_DEBUG_POSTFIX "")
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++11 -fno-omit-frame-pointer -Wno-unused-command-line-argument ${sanitizer}")

if("${LICENSE_CHECK}" STREQUAL "y")
  message("license check is enabled")
  add_definitions(-DLICENSE_CHECK)
endif()

# include header path
include_directories(
  "${THIRD_PARTY}/include"
  "${THIRD_PARTY}/sw/redis++"
  # "${THIRD_PARTY}/json_parser/include"
  # "${THIRD_PARTY}/opusfile_parser/include"c
  "${CMAKE_SOURCE_DIR}"
  "${AGORA_MEDIA_SDK_HEADER_DIR}/include")

# Link common libs
link_directories(${AGORA_MEDIA_SDK_DIR})
link_libraries(${LIB_SDK} ${LIB_THREAD} ${HIREDIS_LIB_DIR} ${REDIS_PLUS_PLUS_LIB_DIR})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/out/)

# Build apps
subdirlist(SUBDIRS ${CMAKE_SOURCE_DIR})
foreach(subdir ${SUBDIRS})
  if(EXISTS ${CMAKE_SOURCE_DIR}/${subdir}/CMakeLists.txt)
    add_subdirectory(${subdir})
  endif()
endforeach()
