#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Core/Graphics/Texture2DAsset.h"

#include "Core/Graphics/GraphicsDevice.h"

#include <directxtk/WICTextureLoader.h>

bool Texture2DAsset::LoadFromWicFile(GraphicsDevice &graphics, const std::filesystem::path &resolvedFilePath)
{
    m_shaderResourceView.Reset();
    m_resolvedSourcePath.clear();

    ID3D11Device *const device = graphics.GetDevice();
    if (device == nullptr)
    {
        return false;
    }

    Microsoft::WRL::ComPtr<ID3D11Resource> textureResource;
    const HRESULT result = DirectX::CreateWICTextureFromFile(
        device,
        resolvedFilePath.c_str(),
        textureResource.GetAddressOf(),
        m_shaderResourceView.GetAddressOf()
    );

    if (FAILED(result))
    {
        return false;
    }

    m_resolvedSourcePath = resolvedFilePath;
    return true;
}

ID3D11ShaderResourceView *Texture2DAsset::GetShaderResourceView() const noexcept
{
    return m_shaderResourceView.Get();
}

const std::filesystem::path &Texture2DAsset::GetSourcePath() const noexcept
{
    return m_resolvedSourcePath;
}
