cmake_minimum_required(VERSION 4.0)
project(celeste)

set(CMAKE_CXX_STANDARD 20)

add_executable(celeste src/main.cpp
        src/Core/game.cpp
        src/Level/level.cpp
        src/Render/ViewRender.cpp
        src/Render/ViewRender.h
        src/Player/Player.cpp
        src/Player/Player.h
        src/Render/Line.cpp
        src/Render/Line.h
        src/Render/Renderable.h
        src/Input/Input.cpp
        src/Input/Input.h
        src/Common/Constant.h)

#添加下面SFML配置
#set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR "C:/Users/ClapEcho233/Documents/program of game/SFML-3.0.2/lib/cmake/SFML")
find_package(SFML COMPONENTS System Window Graphics Audio Network REQUIRED)
include_directories(C:/Users/ClapEcho233/Documents/program of game/SFML-3.0.2/include)
target_link_libraries(celeste SFML::System SFML::Window SFML::Graphics SFML::Audio SFML::Network)

#复制资源到生成的exe文件夹
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

# 复制 SFML/bin 下的所有 DLL 文件到输出目录
file(COPY "C:/Users/ClapEcho233/Documents/program of game/SFML-3.0.2/bin/"
        DESTINATION ${CMAKE_BINARY_DIR}
        FILES_MATCHING PATTERN "*.dll")