cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 11)
project ("MyPlugin" CXX C)

add_library(MyPlugin.plg SHARED  "src/Pluginmain.cpp")

set_target_properties(MyPlugin.plg PROPERTIES PREFIX "" SUFFIX "")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/plugin_output)


