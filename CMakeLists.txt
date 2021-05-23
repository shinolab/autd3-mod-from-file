cmake_minimum_required(VERSION 3.10)

project(autd3-mod-from-file)
set(VERSION_MAJOR   1   CACHE STRING "Project major version number.")
set(VERSION_MINOR   0   CACHE STRING "Project minor version number.")
set(VERSION_PATCH   0   CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

find_package(Git)
if(NOT Git_FOUND)
  message(WARNING "Git not found.")
endif()

if(Git_FOUND)
  execute_process(COMMAND git -C ${PROJECT_SOURCE_DIR} submodule update --init --recursive -- ${PROJECT_SOURCE_DIR}/autd3)
endif()

set(EIGEN_PATH ${PROJECT_SOURCE_DIR}/autd3/client/deps/Eigen3)

if(WIN32)
  set(OS win32)
  include(autd3/client/cmakes/win_vs.cmake)
elseif(APPLE)
  set(OS macosx)
  include(autd3/client/cmakes/mac.cmake)
else()
  set(OS linux)
  if(DEFINED BUILD_TARGET)
    if (${BUILD_TARGET} STREQUAL ARM32 OR ${BUILD_TARGET} STREQUAL ARM)
      include(autd3/client/cmakes/arm32.cmake)
    elseif(${BUILD_TARGET} STREQUAL ARM64 OR ${BUILD_TARGET} STREQUAL AARCH64)
      include(autd3/client/cmakes/arm64.cmake)
    elseif(${BUILD_TARGET} STREQUAL AMD64 OR ${BUILD_TARGET} STREQUAL x86_64 OR ${BUILD_TARGET} STREQUAL x64)
      include(autd3/client/cmakes/linux.cmake)
    endif()
  else()
    include(autd3/client/cmakes/linux.cmake)
  endif()
endif()

add_compile_definitions(_USE_MATH_DEFINES)
add_subdirectory(src)

# autd3-library
option(BUILD_HOLO_GAIN "BUILD_HOLO_GAIN" OFF)
option(BUILD_SOEM_LINK "BUILD_SOEM_LINK" ON)
option(IGNORE_EXAMPLE "IGNORE_EXAMPLE" ON)
option(BUILD_CAPI "BUILD_CAPI" OFF)
option(ENABLE_LINT "ENABLE_LINT" OFF)

add_subdirectory(autd3/client)

add_subdirectory(example)
