cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 11)
project ("QQBot" CXX C)

add_executable (QQBot.out 
 "src/main.cpp"
 "src/ThreadPool/ThreadPool.hpp"
 "src/ThreadPool/ThreadPool.cpp"
 "src/json/json.hpp"
 "src/BasicPlugin.h"
 "src/QQ/QQ.h"
 "src/QQ/QQMessage.h"
 "src/MainProcess.h"
 "src/MainProcess.cpp"
 "src/Functions.h"
 "src/Functions.cpp"
 "src/LoadPlugin.h")

include_directories(${PROJECT_SOURCE_DIR}/src/include/libevent)
include_directories(${PROJECT_SOURCE_DIR}/src/include/libcurl)

add_library(QQ SHARED  "src/QQ/QQMessage.cpp"  "src/QQ/QQ.cpp")
target_link_libraries(QQBot.out QQ)

target_link_libraries(QQBot.out "${PROJECT_SOURCE_DIR}/lib/libcurl/libcurl.a")
target_link_libraries(QQBot.out "${PROJECT_SOURCE_DIR}/lib/libevent/libevent.a")
target_link_libraries(QQBot.out "${PROJECT_SOURCE_DIR}/lib/libevent/libevent_core.a")
target_link_libraries(QQBot.out "${PROJECT_SOURCE_DIR}/lib/libevent/libevent_extra.a")
target_link_libraries(QQBot.out "${PROJECT_SOURCE_DIR}/lib/libevent/libevent_pthreads.a")
target_link_libraries(QQBot.out pthread)
target_link_libraries(QQBot.out dl)

set(LIBRARY_OUTPUT_PATH /root/app/${PROJECT_NAME})
set(EXECUTABLE_OUTPUT_PATH /root/app/${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)