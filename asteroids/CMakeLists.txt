cmake_minimum_required(VERSION 3.8)
project(Asteroids)

#add_subdirectory(${CMAKE_SOURCE_DIR}/DirectXTK12 ${CMAKE_BINARY_DIR}/bin/CMake/DirectXTK12)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/*.cpp")
add_executable(${PROJECT_NAME} ${sources})

set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
set_target_properties(${PROJECT_NAME} PROPERTIES
                      VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

target_include_directories(${PROJECT_NAME} PRIVATE "include")
target_compile_definitions(${PROJECT_NAME} PUBLIC UNICODE)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11.lib dxgi.lib d2d1.lib dwrite.lib dxguid.lib uuid.lib kernel32.lib
    user32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib
    comdlg32.lib runtimeobject.lib
)
