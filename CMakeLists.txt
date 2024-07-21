cmake_minimum_required(VERSION 3.29)

project(Vulkan)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(LIB_FOLDER ${CMAKE_SOURCE_DIR}/libs)

###########
# ImGui
# set(IMGUI_FOLDER ${LIB_FOLDER}/imgui)
#
# include_directories(${IMGUI_FOLDER})
# set(IMGUI_SOURCES
#     ${IMGUI_FOLDER}/imgui.cpp
#     ${IMGUI_FOLDER}/imgui_demo.cpp
#     ${IMGUI_FOLDER}/imgui_draw.cpp
#     ${IMGUI_FOLDER}/imgui_widgets.cpp
#     ${IMGUI_FOLDER}/imgui_tables.cpp
#     ${IMGUI_FOLDER}/misc/cpp/imgui_stdlib.cpp
# )
#

###########
# GLM
include_directories(${LIB_FOLDER}/glm)

###########
# GLFW
include_directories(${LIB_FOLDER}/glfw/include)
link_directories(${LIB_FOLDER}/glfw/bin)

set(GLFW_STATIC_LIBRARY ${LIB_FOLDER}/glfw/bin/libglfw3.a)

###########
# Vulkan
find_package(Vulkan REQUIRED)

include_directories($ENV{VULKAN_SDK}/Include)

###########
# Project
include(source_files.cmake)

add_executable(
    ${PROJECT_NAME}
    ${IMGUI_SOURCES}
    ${RLIMGUI_SOURCES}
    ${PROJECT_SOURCE_FILES}
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE ${GLFW_STATIC_LIBRARY} ${Vulkan_LIBRARIES} -fuse-ld=lld -Wl,/debug,/pdb:${PROJECT_NAME}.pdb
)

target_precompile_headers(
    ${PROJECT_NAME}
    PRIVATE src/pch/glm.h src/pch/stdlib.h
)

target_compile_options(
    ${PROJECT_NAME}
    PUBLIC -Wall -Wextra -Wpedantic -g -gcodeview -O0 -fno-exceptions -std=c++17
)
