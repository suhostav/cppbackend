cmake_minimum_required(VERSION 3.11)

project(postgres CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(book_manager main.cpp)
target_include_directories(book_manager PRIVATE  CONAN_PKG::boost)
target_link_libraries(book_manager PRIVATE Threads::Threads CONAN_PKG::boost CONAN_PKG::libpqxx)