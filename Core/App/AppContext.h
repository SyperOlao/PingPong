//
// Created by SyperOlao on 18.03.2026.
//

#ifndef MYPROJECT_APPCONTEXT_H
#define MYPROJECT_APPCONTEXT_H


class Window;
class InputSystem;
class GraphicsDevice;
class ShapeRenderer2D;
class BitmapFont;
class AudioSystem;

struct AppContext {
    Window *MainWindow{nullptr};
    InputSystem *Input{nullptr};
    GraphicsDevice *Graphics{nullptr};
    ShapeRenderer2D *Shape2D{nullptr};
    BitmapFont *Font{nullptr};
    AudioSystem *Audio{nullptr};

    [[nodiscard]] bool IsValid() const noexcept {
        return MainWindow != nullptr
               && Input != nullptr
               && Graphics != nullptr
               && Shape2D != nullptr
               && Audio != nullptr;
    }
};


#endif //MYPROJECT_APPCONTEXT_H
