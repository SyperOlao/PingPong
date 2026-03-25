//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_SPACEBACKDROPRENDERER_H
#define PINGPONG_SPACEBACKDROPRENDERER_H
#include <random>
#include <SimpleMath.h>
#include <vector>

class Camera;
class PrimitiveRenderer3D;

class SpaceBackdropRenderer final
{
public:
    void Initialize();

    void Render(
        PrimitiveRenderer3D& renderer,
        const Camera& camera,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection
    ) const;

private:
    struct StarInstance final
    {
        DirectX::SimpleMath::Vector3 Direction{0.0f, 1.0f, 0.0f};
        float Distance{300.0f};
        float Radius{0.12f};
        DirectX::SimpleMath::Color Color{1.0f, 1.0f, 1.0f, 1.0f};
    };

    [[nodiscard]] static DirectX::SimpleMath::Vector3 RandomUnitVector(std::mt19937& rng);
    void GenerateDeepField(std::mt19937& rng, int count);
    void GenerateGalacticBand(std::mt19937& rng, int count);
    void GenerateAccentStars(std::mt19937& rng, int count);

private:
    std::vector<StarInstance> m_stars;
    bool m_initialized{false};
};

#endif //PINGPONG_SPACEBACKDROPRENDERER_H