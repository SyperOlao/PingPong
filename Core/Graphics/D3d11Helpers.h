#ifndef PINGPONG_D3D11HELPERS_H
#define PINGPONG_D3D11HELPERS_H

#include <stdexcept>

#include <d3d11.h>

namespace D3d11Helpers
{
    inline void ThrowIfFailed(const HRESULT result, const char *const message)
    {
        if (FAILED(result))
        {
            throw std::runtime_error(message);
        }
    }
}

#endif
