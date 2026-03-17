cmake_minimum_required(VERSION 4.0)
project(PingPong)

set(CMAKE_CXX_STANDARD 20)

add_executable(PingPong main.cpp
        Game/Game.cpp
        Game/Game.cpp
        Game/Game.h
        Core/App/Application.cpp
        Core/App/Application.h
        Core/App/Timer.cpp
        Core/App/Timer.h
        Core/Display/Window.cpp
        Core/Display/Window.h
        Core/Display/Viewport.cpp
        Core/Display/Viewport.h
        Core/Render/Renderer.cpp
        Core/Render/Renderer.h
        Core/Render/ShaderManager.cpp
        Core/Render/ShaderManager.h
        Core/Render/Vertex.h
        Core/Render/Mesh.cpp
        Core/Render/Mesh.h
        Core/Render/SpriteBatch2D.cpp
        Core/Render/SpriteBatch2D.h
        Core/Render/Color.h
        Core/Input/InputSystem.cpp
        Core/Input/InputSystem.h
        Core/Input/Keyboard.cpp
        Core/Input/Keyboard.h
        Core/Input/RawInputHandler.cpp
        Core/Input/RawInputHandler.h
        Core/Scene/Entity.cpp
        Core/Scene/Entity.h
        Core/Scene/Entity.h
        Core/Scene/Scene.cpp
        Core/Scene/Scene.h
        Core/Scene/Scene.h
        Core/Scene/Transform2D.cpp
        Core/Scene/Transform2D.h
        Core/Scene/Transform2D.h
        Core/Scene/Components/Collider2D.h
        Core/Scene/Components/RenderComponent2D.h
        Core/Scene/Components/MovementComponent.h
        Core/Physics/CollisionSystem.cpp
        Core/Physics/CollisionSystem.h
        Core/Physics/AABB.h
        Core/Math/MathHelpers.h
        Core/Common/Types.h
        Core/Common/Constants.h
        Game/AI/PaddleAI.cpp
        Game/AI/PaddleAI.h
        Game/PongScene.cpp
        Game/PongScene.h
        Game/GameRules.cpp
        Game/GameRules.h
        Game/Objects/Paddle.cpp
        Game/Objects/Paddle.h
        Game/Objects/Ball.cpp
        Game/Objects/Ball.h
        Game/Objects/Wall.cpp
        Game/Objects/Wall.h
        Game/Objects/CenterLine.cpp
        Game/Objects/CenterLine.h
        Game/Objects/ScoreBoard.cpp
        Game/Objects/ScoreBoard.h
        Game/Objects/PlayField.cpp
        Game/Objects/PlayField.h)

target_include_directories(PingPong PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(PingPong PRIVATE
        d3d11
        dxgi
        d3dcompiler
)