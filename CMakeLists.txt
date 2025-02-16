cmake_minimum_required(VERSION 3.30)

project (app)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")


# sources
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.c") 
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC)
target_compile_definitions(${PROJECT_NAME} PUBLIC PATH_RESOURCES="${CMAKE_CURRENT_SOURCE_DIR}/resources/")

find_package(raylib REQUIRED)

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
target_link_libraries(ImGui PUBLIC raylib)
target_link_libraries(${PROJECT_NAME} PUBLIC ImGui)

# portaudio
add_subdirectory(external/portaudio)
target_link_libraries(${PROJECT_NAME} PRIVATE portaudio)

# kissfft
option(KISSFFT_TEST OFF)
option(KISSFFT_TOOLS OFF)
option(KISSFFT_STATIC ON)
option(KISSFFT_PKGCONFIG OFF)
add_subdirectory(external/kissfft)
target_link_libraries(${PROJECT_NAME} PRIVATE kissfft)

add_custom_target(run
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)