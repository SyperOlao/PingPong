#ifndef PINGPONG_KATAMARIHUD_H
#define PINGPONG_KATAMARIHUD_H

struct AppContext;
struct KatamariWorldContext;

class KatamariHud final
{
public:
    static void Draw(
        AppContext &context,
        KatamariWorldContext const &world,
        int displayFps,
        float deltaTimeSeconds
    );
};

#endif
