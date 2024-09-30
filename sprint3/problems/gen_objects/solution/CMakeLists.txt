cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# -------- создаем статическую библиотеку ----------
add_library(ModelLib STATIC 
	src/model.h
	src/model.cpp
	src/Dog.h
	src/Dog.cpp
	src/loot_generator.h
	src/loot_generator.cpp
	src/tagged.h
)
target_include_directories(ModelLib PUBLIC ${ZLIB_INCLUDES} ${BOOST_INCLUDES})
target_link_libraries(ModelLib PUBLIC ${BOOST_LIB} ${ZLIB_LIB})


add_executable(game_server
	src/logger.h
	src/logger.cpp
	src/main.cpp
	src/http_server.cpp
	src/http_server.h
	src/sdk.h
	src/boost_json.cpp
	src/json_loader.h
	src/json_loader.cpp
	src/request_handler.cpp
	src/request_handler.h
	src/ApiHandler.h 
	src/ApiHandler.cpp 
	src/handlers_tools 
	src/handlers_tools.cpp
	src/Player.h 
	src/Player.cpp 
	src/Players.h 
	src/Players.cpp
	src/PlayerTokens.h 
	src/PlayerTokens.cpp
	src/GameApp.h 
	src/GameApp.cpp 
	src/Ticker.h
	src/extra_data.h
)

target_include_directories(game_server PRIVATE  CONAN_PKG::boost)
target_link_libraries(game_server PRIVATE Threads::Threads CONAN_PKG::boost ModelLib)

add_executable(game_server_tests
	src/model.h 
	src/model.cpp 
	src/loot_generator.h 
	src/loot_generator.cpp 
	src/tagged.h 
    tests/model-tests.cpp
    tests/loot_generator_tests.cpp
)
target_include_directories(game_server_tests PRIVATE  CONAN_PKG::boost)
target_link_libraries(game_server_tests PRIVATE CONAN_PKG::catch2 CONAN_PKG::boost Threads::Threads ModelLib) 