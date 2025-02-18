cmake_minimum_required(VERSION 3.30)

project (fft_raylib)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

# project sources
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.c") 
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE)
target_compile_definitions(${PROJECT_NAME} PRIVATE PATH_RESOURCES="${CMAKE_CURRENT_SOURCE_DIR}/resources/")

if(APPLE)
set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE True
    MACOSX_BUNDLE_GUI_IDENTIFIER my.domain.style.identifier.main
    MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
    MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/template.plist.in
)
# set(EXE_PREFIX ./${PROJECT_NAME}.app/contents/macos/)
# set(EXE_PREFIX open -a)
# set(EXE_PREFIX ./)
endif()

# raylib
# find_package(raylib REQUIRED)
add_subdirectory(external/raylib)
set_property(TARGET raylib PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE) #-flto
target_compile_definitions(raylib PRIVATE RL_DEFAULT_BATCH_BUFFER_ELEMENTS=65536)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

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
target_link_libraries(ImGui PRIVATE raylib)
target_link_libraries(${PROJECT_NAME} PRIVATE ImGui)

# portaudio
option(PA_BUILD_SHARED_LIBS OFF)
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
    COMMAND ${EXE_PREFIX}${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)