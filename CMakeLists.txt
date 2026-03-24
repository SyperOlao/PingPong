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
        Game/Pong/PongTypes.h
        Game/Pong/Entities/Paddle.cpp
        Game/Pong/Entities/Ball.cpp
        Game/Pong/Systems/PongRules.cpp
        Game/Pong/Systems/PaddleAI.cpp
        Game/Pong/Systems/PongCollisionSystem.cpp
        Game/Pong/PongScene.cpp
        Game/Pong/Entities/ScoreBoard.cpp
        Game/Pong/Entities/Wall.cpp
        Game/Pong/Entities/PlayField.cpp
        Game/Pong/Entities/CenterLine.cpp
        Core/UI/Switcher.cpp
        Core/UI/Switcher.h
        Core/Audio/AudioSystem.cpp
        Core/Audio/AudioSystem.h
        Core/Audio/AudioSystem.h
        Game/Pong/UI/PongUI.cpp
        Game/Pong/UI/PongUI.h
        Game/Pong/UI/PongUI.h
        Game/SolarSystem/SolarSystemGame.cpp
        Game/SolarSystem/SolarSystemGame.h
        Game/SolarSystem/SolarSystemScene.cpp
        Game/SolarSystem/SolarSystemScene.h
        Game/SolarSystem/Entities/PlanetBody.cpp
        Game/SolarSystem/Entities/PlanetBody.h
        Core/Graphics/OrbitCamera.cpp
        Core/Graphics/OrbitCamera.h
        Core/Math/Transform3D.h
        Game/SolarSystem/Systems/OrbitMath.h
        Game/SolarSystem/Systems/OrbitMath.cpp
        Game/SolarSystem/Entities/OrbitalBody.cpp
        Game/SolarSystem/Entities/OrbitalBody.h
        Core/Graphics/MeshData.h
        Core/Graphics/MeshGenerator.h
        Core/Graphics/ProjectionMode.h
        Core/Graphics/Camera.h
        Core/Graphics/Vertex3D.h
        Core/Graphics/MeshGenerator.cpp
        Core/Graphics/MeshGenerator.cpp
        Core/Graphics/FpsCamera.cpp
        Core/Graphics/FpsCamera.h
        Core/Graphics/FpsCamera.h
        Core/Graphics/FpsCamera.h
        Core/Graphics/ObjectConstants.h
        Core/Graphics/PrimitiveRenderer3D.h
        Core/Graphics/FpsCamera.cpp
        Core/Graphics/PrimitiveRenderer3D.cpp
        Core/Graphics/PrimitiveRenderer3D.cpp
        Game/SolarSystem/UI/SolarSystemSettingsPanel.cpp
        Game/SolarSystem/UI/SolarSystemSettingsPanel.h
        Game/SolarSystem/UI/SolarSystemSettingsPanel.h
        Game/SolarSystem/Systems/SolarSystemTuning.h
        Core/UI/Widgets/SliderWidget.cpp
        Core/UI/Widgets/SliderWidget.cpp
        Core/UI/Widgets/SliderWidget.h
        Core/UI/Widgets/CollapsibleSection.cpp
        Core/UI/Widgets/CollapsibleSection.h
        Core/UI/Common/UITheme.h
        Core/UI/Common/RectF.h
        Core/UI/Common/MouseState.h
)
find_package(directxtk CONFIG REQUIRED)
target_include_directories(PingPong PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(SolarSystemGame STATIC
        Game/SolarSystem/SolarSystemGame.cpp
        Game/SolarSystem/SolarSystemScene.cpp
)

target_include_directories(SolarSystemGame PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SolarSystemGame PRIVATE
        Microsoft::DirectXTK
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
file(COPY ${CMAKE_SOURCE_DIR}/Game/Pong/Assets DESTINATION ${CMAKE_BINARY_DIR}/Game/Pong)

file(COPY
        ${CMAKE_SOURCE_DIR}/Core/Shaders
        DESTINATION ${CMAKE_BINARY_DIR}/Core
)