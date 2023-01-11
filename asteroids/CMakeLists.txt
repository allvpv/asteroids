cmake_minimum_required(VERSION 3.8)
project(Asteroids)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/*.cpp")
add_executable(${PROJECT_NAME} ${sources})

set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
set_target_properties(${PROJECT_NAME} PROPERTIES
                      VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# You can optionally turn on beta support for HighDPI
# But this game looks better in LowDPI, just like Hollywood film looks better in 27 FPS
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DPI_AWARE "OFF")

target_include_directories(${PROJECT_NAME} PRIVATE "include")
target_compile_definitions(${PROJECT_NAME} PUBLIC UNICODE)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11.lib dxgi.lib d2d1.lib dwrite.lib dxguid.lib uuid.lib kernel32.lib
    user32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib
    comdlg32.lib runtimeobject.lib
)

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_SOURCE_DIR}/assets
      ${CMAKE_CURRENT_BINARY_DIR}/assets
)

add_dependencies(${PROJECT_NAME} copy_assets)
