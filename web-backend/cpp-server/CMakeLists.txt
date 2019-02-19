cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 11)

set(TARGET_LIST
)
project (server)
# set executable
add_executable(server main.cpp)
set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

if(MINGW)
  target_link_libraries(server ws2_32)
  target_link_libraries(server wsock32)
endif()

find_package(Boost 1.53.0 COMPONENTS system filesystem thread)
target_link_libraries(server ${Boost_LIBRARIES})
target_include_directories(server PRIVATE ${Boost_INCLUDE_DIR})