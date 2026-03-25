//
// Created by SyperOlao on 25.03.2026.
//

#include "Core/App/AppContext.h"

#include "Core/Graphics/Debug/DebugDrawQueue.h"
#include "Core/Graphics/Rendering/Deferred/DeferredFrameResources.h"
#include "Core/Graphics/Rendering/FrameRenderer.h"
#include "Core/Graphics/ModelRenderer.h"
#include "Core/Graphics/Rendering/PrimitiveRenderer3D.h"
#include "Core/Graphics/Rendering/RenderContext.h"
#include "Core/Graphics/Rendering/ShapeRenderer2D.h"

#include <stdexcept>

bool AppContext::IsValid() const noexcept {
    return Platform.MainWindow != nullptr
           && Input.System != nullptr
           && Graphics.Device != nullptr
           && Graphics.Render != nullptr
           && Graphics.Frame != nullptr
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

ModelRenderer &AppContext::GetModelRenderer() const {
    if (Graphics.Render == nullptr) {
        throw std::logic_error("AppContext::GetModelRenderer: Graphics.Render is null.");
    }

    return const_cast<RenderContext *>(Graphics.Render)->GetModelRenderer();
}

DebugDrawQueue &AppContext::GetDebugDraw() const {
    if (Graphics.Render == nullptr) {
        throw std::logic_error("AppContext::GetDebugDraw: Graphics.Render is null.");
    }

    return const_cast<RenderContext *>(Graphics.Render)->GetDebugDraw();
}

FrameRenderer &AppContext::GetFrameRenderer() const {
    if (Graphics.Frame == nullptr) {
        throw std::logic_error("AppContext::GetFrameRenderer: Graphics.Frame is null.");
    }

    return *Graphics.Frame;
}

DeferredFrameResources &AppContext::GetDeferredFrameResources() const {
    if (Graphics.Render == nullptr) {
        throw std::logic_error("AppContext::GetDeferredFrameResources: Graphics.Render is null.");
    }

    return const_cast<RenderContext *>(Graphics.Render)->GetDeferredFrameResources();
}

void AppContext::ResizeDeferredResources() const {
    if (Graphics.Render == nullptr) {
        throw std::logic_error("AppContext::ResizeDeferredResources: Graphics.Render is null.");
    }

    const_cast<RenderContext *>(Graphics.Render)->ResizeDeferredResources();
}
