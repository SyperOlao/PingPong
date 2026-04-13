#include "Core/Graphics/Rendering/Shadows/CascadedShadowMapMath.h"

#include "Core/Graphics/Rendering/Shadows/ShadowProjection.h"

#include <DirectXMath.h>

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

namespace
{
constexpr float kMinimumOrthographicExtent{1.0e-2f};
constexpr float kFrustumCornerMarginXY{1.25f};
constexpr float kFrustumCornerMarginZ{4.0f};

[[nodiscard]] Vector3 TransformWorldFromViewRow(
    const Vector3 &viewSpacePosition,
    const Matrix &inverseCameraViewWorld
)
{
    const Vector4 homogeneous = Vector4::Transform(
        Vector4(viewSpacePosition.x, viewSpacePosition.y, viewSpacePosition.z, 1.0f),
        inverseCameraViewWorld
    );
    return Vector3(homogeneous.x, homogeneous.y, homogeneous.z);
}

[[nodiscard]] Vector3 TransformLightSpaceRow(
    const Vector3 &worldSpacePosition,
    const Matrix &lightViewWorld
)
{
    const Vector4 homogeneous = Vector4::Transform(
        Vector4(worldSpacePosition.x, worldSpacePosition.y, worldSpacePosition.z, 1.0f),
        lightViewWorld
    );
    return Vector3(homogeneous.x, homogeneous.y, homogeneous.z);
}
}

CascadedShadowSplitCpu ComputePracticalCascadeSplits(
    const float cameraNearPlane,
    const float cameraFarPlane,
    const std::uint32_t cascadeCount,
    const float lambdaBlend
)
{
    CascadedShadowSplitCpu result{};
    result.NearPlaneDistance = cameraNearPlane;
    result.FarPlaneDistance = cameraFarPlane;
    result.ViewSpaceSplitBoundaries[0] = cameraNearPlane;

    if (cascadeCount == 0u || cameraFarPlane <= cameraNearPlane)
    {
        return result;
    }

    const float nearPlane = cameraNearPlane;
    const float farPlane = cameraFarPlane;
    const float ratio = farPlane / nearPlane;

    for (std::uint32_t index = 1u; index <= cascadeCount; ++index)
    {
        const float partition = static_cast<float>(index) / static_cast<float>(cascadeCount);
        const float logarithmicSplit = nearPlane * std::pow(ratio, partition);
        const float uniformSplit = nearPlane + (farPlane - nearPlane) * partition;
        const float blended =
            lambdaBlend * logarithmicSplit + (1.0f - lambdaBlend) * uniformSplit;
        result.ViewSpaceSplitBoundaries[index] = std::clamp(blended, nearPlane, farPlane);
    }

    return result;
}

Matrix BuildDirectionalCascadeLightViewProjection(
    const Vector3 &lightDirectionWorld,
    const Matrix &cameraViewMatrix,
    const float viewSpaceNearZ,
    const float viewSpaceFarZ,
    const float verticalFieldOfViewRadians,
    const float aspectRatio,
    const std::uint32_t cascadeResolutionPixels,
    const float lightSpaceDepthPadding
)
{
    const float safeAspect = (std::max)(aspectRatio, 1.0e-4f);
    const float tanHalfVertical = std::tan(verticalFieldOfViewRadians * 0.5f);
    const float tanHalfHorizontal = tanHalfVertical * safeAspect;

    const float nearZ = viewSpaceNearZ;
    const float farZ = viewSpaceFarZ;
    if (!(nearZ > 0.0f) || !(farZ > nearZ))
    {
        return Matrix::Identity;
    }

    const float leftNear = -nearZ * tanHalfHorizontal;
    const float rightNear = nearZ * tanHalfHorizontal;
    const float bottomNear = -nearZ * tanHalfVertical;
    const float topNear = nearZ * tanHalfVertical;

    const float leftFar = -farZ * tanHalfHorizontal;
    const float rightFar = farZ * tanHalfHorizontal;
    const float bottomFar = -farZ * tanHalfVertical;
    const float topFar = farZ * tanHalfVertical;

    const std::array<Vector3, 8> cornersViewSpace =
    {
        Vector3(leftNear, bottomNear, -nearZ),
        Vector3(rightNear, bottomNear, -nearZ),
        Vector3(leftNear, topNear, -nearZ),
        Vector3(rightNear, topNear, -nearZ),
        Vector3(leftFar, bottomFar, -farZ),
        Vector3(rightFar, bottomFar, -farZ),
        Vector3(leftFar, topFar, -farZ),
        Vector3(rightFar, topFar, -farZ)
    };

    const Matrix inverseCameraView = cameraViewMatrix.Invert();

    Vector3 centerWorldAccumulation{0.0f, 0.0f, 0.0f};
    std::array<Vector3, 8> cornersWorldSpace{};

    for (std::size_t cornerIndex = 0u; cornerIndex < cornersViewSpace.size(); ++cornerIndex)
    {
        const Vector3 worldCorner = TransformWorldFromViewRow(cornersViewSpace[cornerIndex], inverseCameraView);
        cornersWorldSpace[cornerIndex] = worldCorner;
        centerWorldAccumulation += worldCorner;
    }

    const Vector3 frustumCenterWorld = centerWorldAccumulation * (1.0f / 8.0f);

    Vector3 lightAxis = lightDirectionWorld;
    lightAxis.Normalize();

    constexpr float kLightEyeDistanceFromFocus{140.0f};
    const Matrix lightViewWorld = ShadowProjection::BuildDirectionalLightView(
        lightAxis,
        frustumCenterWorld,
        Vector3::UnitY,
        kLightEyeDistanceFromFocus
    );

    float minimumX = 1.0e30f;
    float maximumX = -1.0e30f;
    float minimumY = 1.0e30f;
    float maximumY = -1.0e30f;
    float minimumZ = 1.0e30f;
    float maximumZ = -1.0e30f;

    for (const Vector3 &worldCorner : cornersWorldSpace)
    {
        const Vector3 lightSpace = TransformLightSpaceRow(worldCorner, lightViewWorld);
        minimumX = (std::min)(minimumX, lightSpace.x);
        maximumX = (std::max)(maximumX, lightSpace.x);
        minimumY = (std::min)(minimumY, lightSpace.y);
        maximumY = (std::max)(maximumY, lightSpace.y);
        minimumZ = (std::min)(minimumZ, lightSpace.z);
        maximumZ = (std::max)(maximumZ, lightSpace.z);
    }

    minimumX -= kFrustumCornerMarginXY;
    maximumX += kFrustumCornerMarginXY;
    minimumY -= kFrustumCornerMarginXY;
    maximumY += kFrustumCornerMarginXY;
    minimumZ -= kFrustumCornerMarginZ + lightSpaceDepthPadding;
    maximumZ += kFrustumCornerMarginZ + lightSpaceDepthPadding;

    const float extentX = (std::max)(maximumX - minimumX, kMinimumOrthographicExtent);
    const float extentY = (std::max)(maximumY - minimumY, kMinimumOrthographicExtent);

    const float resolution = static_cast<float>((std::max)(cascadeResolutionPixels, 1u));
    const float unitsPerTexelX = extentX / resolution;
    const float unitsPerTexelY = extentY / resolution;

    minimumX = std::floor(minimumX / unitsPerTexelX) * unitsPerTexelX;
    maximumX = std::floor(maximumX / unitsPerTexelX) * unitsPerTexelX;
    minimumY = std::floor(minimumY / unitsPerTexelY) * unitsPerTexelY;
    maximumY = std::floor(maximumY / unitsPerTexelY) * unitsPerTexelY;

    const Matrix orthographicProjection = Matrix::CreateOrthographicOffCenter(
        minimumX,
        maximumX,
        minimumY,
        maximumY,
        minimumZ,
        maximumZ
    );

    return lightViewWorld * orthographicProjection;
}
