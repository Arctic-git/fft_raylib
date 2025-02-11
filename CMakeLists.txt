cmake_minimum_required(VERSION 3.30)


# Set the project name
project (app)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# sources
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "src/*.cpp") # Define PROJECT_SOURCES as a list of all source files

file(GLOB SOURCES ${PROJECT_SOURCES})
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC)

find_package(raylib REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC raylib)

# ImGui source files 
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
set(IMPLOT_DIR ${CMAKE_SOURCE_DIR}/external/implot)
set(RLIMGUI_DIR ${CMAKE_SOURCE_DIR}/external/rlImGui)
add_library(ImGui STATIC 
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMPLOT_DIR}/implot.cpp
    ${IMPLOT_DIR}/implot_items.cpp
    ${IMPLOT_DIR}/implot_demo.cpp
    ${RLIMGUI_DIR}/rlImGui.cpp
)
target_include_directories(ImGui PUBLIC ${IMGUI_DIR} ${IMPLOT_DIR} ${RLIMGUI_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ImGui)


add_custom_target(run
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)