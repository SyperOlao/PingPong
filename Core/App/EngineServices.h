//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_ENGINESERVICES_H
#define PINGPONG_ENGINESERVICES_H

class Window;
class InputSystem;
class GraphicsDevice;
class RenderContext;
class FrameRenderer;
class BitmapFont;
class AudioSystem;
class AssetCache;

struct PlatformServices final {
    Window *MainWindow{nullptr};
};

struct InputServices final {
    InputSystem *System{nullptr};
};

struct GraphicsServices final {
    GraphicsDevice *Device{nullptr};
    RenderContext *Render{nullptr};
    FrameRenderer *Frame{nullptr};
};

struct UiServices final {
    BitmapFont *Font{nullptr};
};

struct AudioServices final {
    AudioSystem *System{nullptr};
};

struct AssetServices final {
    AssetCache *Cache{nullptr};
};

#endif //PINGPONG_ENGINESERVICES_H
