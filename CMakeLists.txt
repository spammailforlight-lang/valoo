cmake_minimum_required(VERSION 3.24)
project(ValoBoggiee LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "x64 only")
endif()

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE third_party)


add_library(imgui STATIC
  third_party/imgui/imgui.cpp
  third_party/imgui/imgui_draw.cpp
  third_party/imgui/imgui_tables.cpp
  third_party/imgui/imgui_widgets.cpp
  third_party/imgui/backends/imgui_impl_win32.cpp
  third_party/imgui/backends/imgui_impl_dx11.cpp
)
target_include_directories(imgui PUBLIC third_party/imgui third_party/imgui/backends)
target_link_libraries(imgui PUBLIC d3d11 dxgi)

add_library(Core STATIC
  src/Core/common.cpp
  src/Core/log.cpp
  src/Core/config.cpp
  src/Core/hotkeys.cpp
  src/Core/memory.cpp
  src/Core/scanner.cpp
  src/Core/offsets.cpp
  src/Core/game.cpp
  src/Features/radar.cpp
  src/Features/prediction.cpp
  src/Features/recoil.cpp
  src/Features/movement.cpp
  src/Features/utility.cpp
  src/Features/skins.cpp
)
target_include_directories(Core PUBLIC src)
target_link_libraries(Core PUBLIC imgui d3d11 dxgi d3dcompiler psapi nlohmann_json)
target_compile_options(Core PRIVATE /W4 /permissive- /WX)

add_executable(Cheat WIN32
  src/App/main.cpp
  src/Overlay/overlay.cpp
)
target_link_libraries(Cheat PRIVATE Core imgui d3d11 dxgi d3dcompiler dwmapi)
target_sources(Cheat PRIVATE src/App/app.manifest)
target_link_options(Cheat PRIVATE "/MANIFESTUAC:level='requireAdministrator' uiAccess='false'")
set_target_properties(Cheat PROPERTIES OUTPUT_NAME "ValoBoggiee")
target_compile_options(Cheat PRIVATE /W4 /permissive- /WX)
add_custom_command(TARGET Cheat POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/data
    $<TARGET_FILE_DIR:Cheat>/data
)
