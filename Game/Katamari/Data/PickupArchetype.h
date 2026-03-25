//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_PICKUPARCHETYPE_H
#define PINGPONG_PICKUPARCHETYPE_H

#include <string>

class ModelAsset;

struct PickupArchetype final {
    std::string Id;
    std::wstring ModelPath;
    float VisualScale{1.0f};
    float BoundingRadius{1.0f};
    float AbsorbValue{1.0f};
    float MinRequiredBallRadius{1.0f};
};

#endif //PINGPONG_PICKUPARCHETYPE_H
