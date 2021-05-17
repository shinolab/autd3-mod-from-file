cmake_minimum_required(VERSION 3.1)

set (CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/autd3/client/include ${EIGEN_PATH})

if(WIN32)
  link_libraries(ws2_32.lib)
  link_libraries(winmm.lib)
elseif(APPLE)
  link_libraries(pcap)
else()
  link_libraries(rt)
endif()
  
add_executable(example main.cpp)
target_link_libraries(example autd3 soem_link from_file_modulation)
