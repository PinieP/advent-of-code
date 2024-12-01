add_subdirectory(utils)

add_executable(day1 day1/main.cpp)
target_link_libraries(day1 utils)
