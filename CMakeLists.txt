cmake_minimum_required(VERSION 4.0)
project(celeste)

set(CMAKE_CXX_STANDARD 20)

# 静态链接 MinGW 运行时库
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
add_definitions(-D_WIN32_WINNT=0x0601)

add_executable(celeste src/main.cpp
        src/Core/game.cpp
        src/Level/level.cpp
        src/Render/ViewRender.cpp
        src/Render/ViewRender.h
        src/Player/Player.cpp
        src/Player/Player.h
        src/Render/Line.cpp
        src/Render/Line.h
        src/Common/Renderable.h
        src/Input/Input.cpp
        src/Input/Input.h
        src/Common/Constant.h
        src/Common/json.hpp
        src/Player/StateMachine.cpp
        src/Player/StateMachine.h
        src/Render/TrailEffect.cpp
        src/Render/TrailEffect.h
        src/Render/Particle.cpp
        src/Render/Particle.h
)

# 设置SFML为静态链接
set(SFML_STATIC_LIBRARIES TRUE)

set(libcopp_DIR "C:/Users/ClapEcho233/.vcpkg-clion/vcpkg/installed/x64-mingw-static/share/libcopp")

#添加下面SFML配置
set(SFML_DIR "C:/Users/ClapEcho233/Documents/programofgame/SFML-3.0.2/lib/cmake/SFML")
find_package(SFML COMPONENTS System Window Graphics Audio Network REQUIRED)
include_directories(C:/Users/ClapEcho233/Documents/programofgame/SFML-3.0.2/include)
target_link_libraries(celeste SFML::System SFML::Window SFML::Graphics SFML::Audio SFML::Network)

set(CMAKE_TOOLCHAIN_FILE "C:/Users/ClapEcho233/.vcpkg-clion/vcpkg/scriptsbuildsystems/vcpkg.cmake")

# 静态链接 Windows 系统库
target_link_libraries(celeste opengl32 freetype winmm gdi32 advapi32 user32 ws2_32)

# 复制assets文件夹及其所有内容
add_custom_command(TARGET celeste POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:celeste>/assets
        COMMENT "Copying assets to output directory"
)

find_package(libcopp REQUIRED)
target_link_libraries(celeste libcopp::copp libcopp::cotask)

# 复制 SFML/bin 下的所有 DLL 文件到输出目录
#file(COPY "C:/Users/ClapEcho233/Documents/program of game/SFML-3.0.2/bin/"
#        DESTINATION ${CMAKE_BINARY_DIR}
#        FILES_MATCHING PATTERN "*.dll")