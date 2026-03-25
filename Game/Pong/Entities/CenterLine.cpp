//
// Created by SyperOlao on 17.03.2026.
//

#include "../Entities/CenterLine.h"
#include "Game/Pong/Common/Constants.h"
#include "Core/Graphics/Color.h"
#include "../../../Core/Graphics/Rendering/ShapeRenderer2D.h"

void CenterLine::Configure(const float x, const float topY, const float playableHeight) noexcept
{
    m_x = x;
    m_topY = topY;
    m_playableHeight = playableHeight;
}

void CenterLine::Render(const ShapeRenderer2D& renderer) const
{
    const int segmentCount = static_cast<int>(
        m_playableHeight / (Constants::CenterLineSegmentHeight + Constants::CenterLineGap)
    );

    for (int index = 0; index < segmentCount; ++index)
    {
        const float segmentY =
            m_topY + 16.0f + static_cast<float>(index) *
            (Constants::CenterLineSegmentHeight + Constants::CenterLineGap);

        renderer.DrawFilledRect(
            m_x,
            segmentY,
            Constants::CenterLineWidth,
            Constants::CenterLineSegmentHeight,
            Color(0.55f, 0.55f, 0.62f, 1.0f)
        );
    }
}