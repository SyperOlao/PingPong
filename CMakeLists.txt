cmake_minimum_required(VERSION 3.21)

project(PingPong LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(PingPong
        main.cpp

        Core/App/Application.cpp
        Core/App/Application.h
        Core/App/ApplicationDefaults.h
        Core/App/Timer.cpp
        Core/App/Timer.h
        Core/App/IGame.h
        Core/App/EngineServices.h
        Core/App/AppContext.h
        Core/App/AppContext.cpp

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
        Core/Graphics/D3d11Helpers.h
        Core/Graphics/ShaderCompiler.cpp
        Core/Graphics/ShaderCompiler.h
        Core/Graphics/Color.h
        Core/Graphics/OrbitCamera.cpp
        Core/Graphics/OrbitCamera.h
        Core/Graphics/FpsCamera.cpp
        Core/Graphics/FpsCamera.h
        Core/Graphics/MeshData.h
        Core/Graphics/MeshGenerator.h
        Core/Graphics/MeshGenerator.cpp
        Core/Graphics/ProjectionMode.h
        Core/Graphics/Camera.h
        Core/Graphics/Vertex3D.h
        Core/Graphics/ObjectConstants.h
        Core/Graphics/ModelAsset.h
        Core/Graphics/ModelAsset.cpp
        Core/Graphics/ModelRenderer.h
        Core/Graphics/ModelRenderer.cpp
        Core/Graphics/Texture2DAsset.h
        Core/Graphics/Texture2DAsset.cpp

        Core/Graphics/Rendering/ShapeRenderer2D.cpp
        Core/Graphics/Rendering/ShapeRenderer2D.h
        Core/Graphics/Rendering/PrimitiveRenderer3D.cpp
        Core/Graphics/Rendering/PrimitiveRenderer3D.h
        Core/Graphics/Rendering/RenderContext.cpp
        Core/Graphics/Rendering/RenderContext.h
        Core/Graphics/Rendering/Renderables/RenderMaterialParameters.h
        Core/Graphics/Rendering/Renderables/RenderTransform3D.h
        Core/Graphics/Rendering/Renderables/RenderGeometry3D.h
        Core/Graphics/Rendering/Renderables/Renderable3D.h
        Core/Graphics/Rendering/Renderables/SceneRenderData3D.h
        Core/Graphics/Rendering/Renderables/SceneRenderer3D.h
        Core/Graphics/Rendering/Renderables/SceneRenderer3D.cpp

        Core/Assets/AssetCache.h
        Core/Assets/AssetCache.cpp
        Core/Assets/AssetPathResolver.h
        Core/Assets/AssetPathResolver.cpp

        Core/Physics/AABB.h
        Core/Physics/BoundingSphere.h
        Core/Physics/Collision3D.h
        Core/Physics/CollisionFoundation.h
        Core/Physics/Collision2D/AABB2D.h
        Core/Physics/Collision2D/Circle2D.h
        Core/Physics/Collision2D/CollisionContact2D.h
        Core/Physics/Collision2D/CollisionQueries2D.cpp
        Core/Physics/Collision2D/CollisionQueries2D.h
        Core/Physics/Collision2D/HorizontalBoundsExit.h
        Core/Physics/Collision3D/AxisAlignedBox3D.h
        Core/Physics/Collision3D/BoundingSphere3D.h
        Core/Physics/Collision3D/CollisionContact3D.h
        Core/Physics/Collision3D/CollisionQueries3D.cpp
        Core/Physics/Collision3D/CollisionQueries3D.h
        Core/Physics/HorizontalBoundsExit.h
        Core/Physics/PlaneConstraint.h
        Core/Physics/PlaneConstraint.cpp

        Core/Math/SpatialMath.h
        Core/Math/Transform3D.h
        Core/Math/Transform3D.cpp
        Core/Math/Transform2D.h
        Core/Math/Transform2D.cpp

        Core/UI/BitmapFont.cpp
        Core/UI/BitmapFont.h
        Core/UI/Button.cpp
        Core/UI/Button.h
        Core/UI/Switcher.cpp
        Core/UI/Switcher.h
        Core/UI/Widgets/SliderWidget.cpp
        Core/UI/Widgets/SliderWidget.h
        Core/UI/Widgets/CollapsibleSection.cpp
        Core/UI/Widgets/CollapsibleSection.h
        Core/UI/Common/UITheme.h
        Core/UI/Common/RectF.h
        Core/UI/Common/MouseState.h

        Core/Audio/AudioSystem.cpp
        Core/Audio/AudioSystem.h

        Game/Pong/PongGame.cpp
        Game/Pong/PongGame.h
        Game/Pong/PongTypes.h
        Game/Pong/Common/Types.h
        Game/Pong/Entities/Paddle.cpp
        Game/Pong/Entities/Ball.cpp
        Game/Pong/Systems/PongRules.cpp
        Game/Pong/Systems/PaddleAI.cpp
        Game/Pong/Systems/PongCollisionSystem.cpp
        Game/Pong/Systems/PongCollisionSystem.h
        Game/Pong/PongScene.cpp
        Game/Pong/Entities/ScoreBoard.cpp
        Game/Pong/Entities/Wall.cpp
        Game/Pong/Entities/PlayField.cpp
        Game/Pong/Entities/CenterLine.cpp
        Game/Pong/UI/PongUI.cpp
        Game/Pong/UI/PongUI.h

        Game/SolarSystem/SolarSystemGame.cpp
        Game/SolarSystem/SolarSystemGame.h
        Game/SolarSystem/SolarSystemScene.cpp
        Game/SolarSystem/SolarSystemScene.h
        Game/SolarSystem/Entities/PlanetBody.cpp
        Game/SolarSystem/Entities/PlanetBody.h
        Game/SolarSystem/Systems/OrbitMath.h
        Game/SolarSystem/Systems/OrbitMath.cpp
        Game/SolarSystem/Entities/OrbitalBody.cpp
        Game/SolarSystem/Entities/OrbitalBody.h
        Game/SolarSystem/UI/SolarSystemSettingsPanel.cpp
        Game/SolarSystem/UI/SolarSystemSettingsPanel.h
        Game/SolarSystem/Systems/SolarSystemTuning.h
        Game/SolarSystem/Rendering/SpaceBackdropRenderer.cpp
        Game/SolarSystem/Rendering/SpaceBackdropRenderer.h
        Game/SolarSystem/Systems/SolarSystemEngineAudioController.cpp
        Game/SolarSystem/Systems/SolarSystemEngineAudioController.h

        Game/Katamari/KatamariGame.cpp
        Game/Katamari/KatamariGame.h
        Game/Katamari/KatamariScene.cpp
        Game/Katamari/KatamariScene.h
        Game/Katamari/KatamariTypes.h
        Game/Katamari/Components/SphereColliderComponent.h
        Game/Katamari/Components/VelocityComponent.h
        Game/Katamari/Components/PickupVisualComponent.h
        Game/Katamari/Entities/KatamariBall.cpp
        Game/Katamari/Entities/KatamariBall.h
        Game/Katamari/Entities/PickupItem.cpp
        Game/Katamari/Entities/PickupItem.h
        Game/Katamari/Entities/LevelBounds.cpp
        Game/Katamari/Entities/LevelBounds.h
        Game/Katamari/Systems/KatamariSpawner.cpp
        Game/Katamari/Systems/KatamariSpawner.h
        Game/Katamari/Systems/KatamariAbsorptionSystem.cpp
        Game/Katamari/Systems/KatamariAbsorptionSystem.h
        Game/Katamari/Systems/KatamariMovementSystem.cpp
        Game/Katamari/Systems/KatamariMovementSystem.h
        Game/Katamari/Systems/KatamariCameraController.cpp
        Game/Katamari/Systems/KatamariCameraController.h
        Game/Katamari/Systems/KatamariProgressionSystem.cpp
        Game/Katamari/Systems/KatamariProgressionSystem.h
        Game/Katamari/Data/PickupArchetype.h
        Game/Katamari/Data/KatamariGameConfig.h
        Game/Katamari/UI/KatamariHud.cpp
        Game/Katamari/UI/KatamariHud.h
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
file(COPY ${CMAKE_SOURCE_DIR}/Game/SolarSystem/Assets DESTINATION ${CMAKE_BINARY_DIR}/Game/SolarSystem)
if(EXISTS "${CMAKE_SOURCE_DIR}/Game/Katamari/Assets")
    file(COPY ${CMAKE_SOURCE_DIR}/Game/Katamari/Assets DESTINATION ${CMAKE_BINARY_DIR}/Game/Katamari)
endif()
file(COPY
        ${CMAKE_SOURCE_DIR}/Core/Shaders
        DESTINATION ${CMAKE_BINARY_DIR}/Core
)
