#include "Game/Katamari/UI/KatamariHud.h"

#include "Core/App/AppContext.h"
#include "Core/Graphics/Color.h"
#include "Core/UI/BitmapFont.h"
#include "Game/Katamari/Data/KatamariWorldContext.h"

#include <format>

void KatamariHud::Draw(
    AppContext &context,
    KatamariWorldContext const &world,
    int const displayFps,
    float const deltaTimeSeconds
)
{
    if (context.Ui.Font == nullptr)
    {
        return;
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
        "WASD move   R restart level",
        mutedColor,
        helpScale
    );
}
