#include "Game/Katamari/UI/KatamariHud.h"

#include "Core/App/AppContext.h"
#include "Core/Graphics/Color.h"
#include "Core/Graphics/Rendering/ShapeRenderer2D.h"
#include "Core/UI/BitmapFont.h"
#include "Core/Platform/Window.h"
#include "Game/Katamari/Data/KatamariWorldContext.h"

#include <array>
#include <format>

static void DrawGBufferDebugLabels(AppContext &Context)
{
    if (Context.Ui.Font == nullptr || Context.Platform.MainWindow == nullptr)
    {
        return;
    }

    const int WindowWidth = Context.Platform.MainWindow->GetWidth();
    const int WindowHeight = Context.Platform.MainWindow->GetHeight();
    if (WindowWidth <= 0 || WindowHeight <= 0)
    {
        return;
    }

    const std::array<const char *, 6> LabelTexts =
    {
        "ALBEDO / AMBIENT",
        "NORMAL / RECEIVE LIGHTING",
        "SPECULAR / POWER",
        "EMISSIVE",
        "SCENE DEPTH",
        "UNUSED"
    };

    constexpr float LabelScale = 0.42f;
    constexpr float LabelPaddingX = 8.0f;
    constexpr float LabelPaddingY = 5.0f;
    const float TileWidth = static_cast<float>(WindowWidth) / 3.0f;
    const float TileHeight = static_cast<float>(WindowHeight) / 2.0f;
    const Color LabelTextColor(0.96f, 0.98f, 1.0f, 1.0f);
    const Color LabelBackgroundColor(0.02f, 0.025f, 0.035f, 0.82f);

    for (std::size_t LabelIndex = 0u; LabelIndex < LabelTexts.size(); ++LabelIndex)
    {
        const std::size_t Column = LabelIndex % 3u;
        const std::size_t Row = LabelIndex / 3u;
        const float LabelX = static_cast<float>(Column) * TileWidth + 14.0f;
        const float LabelY = Row == 0u ? TileHeight - 32.0f : TileHeight + 14.0f;
        const float LabelWidth = BitmapFont::MeasureTextWidth(LabelTexts[LabelIndex], LabelScale) + LabelPaddingX * 2.0f;
        const float LabelHeight = BitmapFont::MeasureTextHeight(LabelScale) + LabelPaddingY * 2.0f;

        Context.GetShapeRenderer2D().DrawFilledRect(
            LabelX - LabelPaddingX,
            LabelY - LabelPaddingY,
            LabelWidth,
            LabelHeight,
            LabelBackgroundColor
        );
        BitmapFont::DrawString(
            Context.GetShapeRenderer2D(),
            LabelX,
            LabelY,
            LabelTexts[LabelIndex],
            LabelTextColor,
            LabelScale
        );
    }
}

void KatamariHud::Draw(
    AppContext &context,
    KatamariWorldContext const &world,
    int const displayFps,
    float const deltaTimeSeconds,
    const bool gBufferDebugVisualizationEnabled,
    const bool shadowCascadeDebugVisualizationEnabled
)
{
    if (context.Ui.Font == nullptr)
    {
        return;
    }

    if (gBufferDebugVisualizationEnabled)
    {
        DrawGBufferDebugLabels(context);
    }

    constexpr float hudScale = 0.52f;
    constexpr float helpScale = 0.38f;
    const Color textColor(1.0f, 1.0f, 1.0f, 1.0f);
    const Color mutedColor(0.72f, 0.8f, 0.92f, 1.0f);

    float rowY = 16.0f;
    const float columnX = 16.0f;

    BitmapFont::DrawString(
        context.GetShapeRenderer2D(),
        columnX,
        rowY,
        std::format("FPS: {}", displayFps),
        textColor,
        hudScale
    );
    rowY += 18.0f;

    BitmapFont::DrawString(
        context.GetShapeRenderer2D(),
        columnX,
        rowY,
        std::format("Ball radius: {:.2f}", static_cast<double>(world.BallRadius)),
        textColor,
        hudScale
    );
    rowY += 18.0f;

    BitmapFont::DrawString(
        context.GetShapeRenderer2D(),
        columnX,
        rowY,
        std::format("Collected: {} / {}", world.CollectedCount, world.TotalPickups),
        textColor,
        hudScale
    );
    rowY += 18.0f;

    BitmapFont::DrawString(
        context.GetShapeRenderer2D(),
        columnX,
        rowY,
        std::format("Frame dt: {:.3f} ms", static_cast<double>(deltaTimeSeconds) * 1000.0),
        mutedColor,
        hudScale
    );
    rowY += 22.0f;

    BitmapFont::DrawString(
        context.GetShapeRenderer2D(),
        columnX,
        rowY,
        world.DebugDrawCollision ? "Collision debug: ON (F3)" : "Collision debug: OFF (F3)",
        world.DebugDrawCollision ? Color(0.45f, 1.0f, 0.55f, 1.0f) : mutedColor,
        helpScale
    );
    rowY += 16.0f;

    BitmapFont::DrawString(
        context.GetShapeRenderer2D(),
        columnX,
        rowY,
        gBufferDebugVisualizationEnabled ? "GBuffer textures: ON (F5)" : "GBuffer textures: OFF (F5)",
        gBufferDebugVisualizationEnabled ? Color(0.45f, 1.0f, 0.55f, 1.0f) : mutedColor,
        helpScale
    );
    rowY += 16.0f;

    BitmapFont::DrawString(
        context.GetShapeRenderer2D(),
        columnX,
        rowY,
        shadowCascadeDebugVisualizationEnabled
            ? "Cascade shadow debug: ON (F4)"
            : "Cascade shadow debug: OFF (F4)",
        shadowCascadeDebugVisualizationEnabled ? Color(0.45f, 1.0f, 0.55f, 1.0f) : mutedColor,
        helpScale
    );
    rowY += 16.0f;

    BitmapFont::DrawString(
        context.GetShapeRenderer2D(),
        columnX,
        rowY,
        "WASD move   R restart level",
        mutedColor,
        helpScale
    );
}
