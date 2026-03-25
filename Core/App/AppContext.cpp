//
// Created by SyperOlao on 25.03.2026.
//

#include "Core/App/AppContext.h"

#include "Core/Graphics/Rendering/PrimitiveRenderer3D.h"
#include "Core/Graphics/Rendering/RenderContext.h"
#include "Core/Graphics/Rendering/ShapeRenderer2D.h"

#include <stdexcept>

bool AppContext::IsValid() const noexcept {
    return Platform.MainWindow != nullptr
           && Input.System != nullptr
           && Graphics.Device != nullptr
           && Graphics.Render != nullptr
           && Ui.Font != nullptr
           && Audio.System != nullptr
           && Assets.Cache != nullptr;
}

ShapeRenderer2D &AppContext::GetShapeRenderer2D() const {
    if (Graphics.Render == nullptr) {
        throw std::logic_error("AppContext::GetShapeRenderer2D: Graphics.Render is null.");
    }

    return const_cast<RenderContext *>(Graphics.Render)->GetShapeRenderer2D();
}

PrimitiveRenderer3D &AppContext::GetPrimitiveRenderer3D() const {
    if (Graphics.Render == nullptr) {
        throw std::logic_error("AppContext::GetPrimitiveRenderer3D: Graphics.Render is null.");
    }

    return const_cast<RenderContext *>(Graphics.Render)->GetPrimitiveRenderer3D();
}
