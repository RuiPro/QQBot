cmake_minimum_required (VERSION 3.8)
set(CMAKE_BUILD_TYPE "Release")           # 设置模式
set(CMAKE_CXX_STANDARD 11)
project ("QQBot" CXX C)
# set(CMAKE_EXE_LINKER_FLAGS "-rdynamic ${CMAKE_EXE_LINKER_FLAGS}")

# 输出目录
# 可执行文件
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/release)
# 动态库文件
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/debug/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/release/lib)
# 静态库文件
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin/debug/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/release/lib)

add_executable (QQBot.app "src/main.cpp")

 # 自建库
 add_library(mainproc SHARED
 "src/ThreadPool/thread_pool.hpp"
 "src/ThreadPool/thread_pool.cpp"
 "src/main_process.cpp"
 "src/load_plugin.cpp")
 add_library(gadget SHARED "src/Gadget/all2str.cpp")
 add_library(loger SHARED "src/Loger/loger.hpp" "src/Loger/loger.cpp")
 add_library(QQ SHARED "src/QQ/QQ_message.cpp" "src/QQ/QQ.cpp")
 add_library(sqlitec SHARED "src/SQLiteClient/sqlite_client.cpp" "src/SQLiteClient/sql.hpp")

 # 外部库头文件
include_directories(${PROJECT_SOURCE_DIR}/include/json)
include_directories(${PROJECT_SOURCE_DIR}/include/libevent)
include_directories(${PROJECT_SOURCE_DIR}/include/libcurl)
include_directories(${PROJECT_SOURCE_DIR}/include/libsqlite3)

# 链接库
target_link_libraries(sqlitec "${PROJECT_SOURCE_DIR}/lib/linux_amd64/libsqlite3/libsqlite3.so.0.8.6")
target_link_libraries(QQBot.app mainproc)
target_link_libraries(QQBot.app gadget)
target_link_libraries(QQBot.app sqlitec)
target_link_libraries(QQBot.app loger)
target_link_libraries(QQBot.app QQ)
target_link_libraries(QQBot.app "${PROJECT_SOURCE_DIR}/lib/linux_amd64/libcurl/libcurl.so")
target_link_libraries(QQBot.app "${PROJECT_SOURCE_DIR}/lib/linux_amd64/libevent/libevent-2.1.so.7")
target_link_libraries(QQBot.app "${PROJECT_SOURCE_DIR}/lib/linux_amd64/libevent/libevent_core-2.1.so.7")
target_link_libraries(QQBot.app "${PROJECT_SOURCE_DIR}/lib/linux_amd64/libevent/libevent_extra-2.1.so.7")
target_link_libraries(QQBot.app "${PROJECT_SOURCE_DIR}/lib/linux_amd64/libevent/libevent_pthreads-2.1.so.7")
target_link_libraries(QQBot.app pthread)
target_link_libraries(QQBot.app dl)

# 设置可执行文件检索的库路径
 set_target_properties(QQBot.app PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "\$ORIGIN/lib"
)

# 复制库文件
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Copy library to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/lib")
    add_custom_command(TARGET QQBot.app POST_BUILD 
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${PROJECT_SOURCE_DIR}/lib/linux_amd64/libsqlite3/libsqlite3.so.0.8.6
                   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/lib
    )
    add_custom_command(TARGET QQBot.app POST_BUILD 
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${PROJECT_SOURCE_DIR}/lib/linux_amd64/libcurl/libcurl.so
                   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/lib
    )
    add_custom_command(TARGET QQBot.app POST_BUILD 
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${PROJECT_SOURCE_DIR}/lib/linux_amd64/libevent/libevent-2.1.so.7
                   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/lib
    ) 
    add_custom_command(TARGET QQBot.app POST_BUILD 
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${PROJECT_SOURCE_DIR}/lib/linux_amd64/libevent/libevent_core-2.1.so.7
                   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/lib
    ) 
    add_custom_command(TARGET QQBot.app POST_BUILD 
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${PROJECT_SOURCE_DIR}/lib/linux_amd64/libevent/libevent_extra-2.1.so.7
                   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/lib
    ) 
    add_custom_command(TARGET QQBot.app POST_BUILD 
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${PROJECT_SOURCE_DIR}/lib/linux_amd64/libevent/libevent_pthreads-2.1.so.7
                   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}/lib
    ) 
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Copy library to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/lib")
    add_custom_command(TARGET QQBot.app POST_BUILD 
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${PROJECT_SOURCE_DIR}/lib/linux_amd64/libsqlite3/libsqlite3.so.0.8.6
                   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/lib
    )
    add_custom_command(TARGET QQBot.app POST_BUILD 
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${PROJECT_SOURCE_DIR}/lib/linux_amd64/libcurl/libcurl.so
                   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/lib
    )
    add_custom_command(TARGET QQBot.app POST_BUILD 
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${PROJECT_SOURCE_DIR}/lib/linux_amd64/libevent/libevent-2.1.so.7
                   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/lib
    ) 
    add_custom_command(TARGET QQBot.app POST_BUILD 
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${PROJECT_SOURCE_DIR}/lib/linux_amd64/libevent/libevent_core-2.1.so.7
                   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/lib
    ) 
    add_custom_command(TARGET QQBot.app POST_BUILD 
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${PROJECT_SOURCE_DIR}/lib/linux_amd64/libevent/libevent_extra-2.1.so.7
                   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/lib
    ) 
    add_custom_command(TARGET QQBot.app POST_BUILD 
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${PROJECT_SOURCE_DIR}/lib/linux_amd64/libevent/libevent_pthreads-2.1.so.7
                   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}/lib
    ) 
endif ()