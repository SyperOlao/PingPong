cmake_minimum_required(VERSION 3.21)

project(PingPong LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(PingPong
        main.cpp

        Game/Pong/PongGame.cpp
        Game/Pong/PongGame.h

        Core/App/Application.cpp
        Core/App/Application.h
        Core/App/Timer.cpp
        Core/App/Timer.h
        Core/App/IGame.h
        Core/App/AppContext.h

        Core/Platform/Window.cpp
        Core/Platform/Window.h

        Core/Input/InputSystem.cpp
        Core/Input/InputSystem.h
        Core/Input/Keyboard.cpp
        Core/Input/Keyboard.h
        Core/Input/RawInputHandler.cpp
        Core/Input/RawInputHandler.h

        Core/Graphics/GraphicsDevice.cpp
        Core/Graphics/GraphicsDevice.h
        Core/Graphics/ShaderCompiler.cpp
        Core/Graphics/ShaderCompiler.h
        Core/Graphics/Color.h

        Core/Graphics2D/ShapeRenderer2D.cpp
        Core/Graphics2D/ShapeRenderer2D.h
        Core/Physics/AABB.h
        Core/UI/BitmapFont.cpp
        Core/UI/BitmapFont.h
        Core/UI/Button.cpp
        Core/UI/Button.h
        Core/Physics/CollisionSystem.cpp
)
find_package(directxtk CONFIG REQUIRED)
target_include_directories(PingPong PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(PingPong PRIVATE
        Microsoft::DirectXTK
        d3d11
        dxgi
        d3dcompiler
        dxguid
        user32
        gdi32
)


file(COPY
        ${CMAKE_SOURCE_DIR}/Core/Shaders
        DESTINATION ${CMAKE_BINARY_DIR}/Core
)