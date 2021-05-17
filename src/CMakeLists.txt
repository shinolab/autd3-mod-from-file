set (CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/autd3/client/include)

add_library(from_file_modulation STATIC
  ${PROJECT_SOURCE_DIR}/src/from_file_modulation.cpp
  ${PROJECT_SOURCE_DIR}/include/from_file_modulation.hpp
)

set_target_properties(from_file_modulation
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

if(NOT WIN32)
  set_target_properties(from_file_modulation PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
