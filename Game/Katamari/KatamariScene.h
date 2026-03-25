//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_KATAMARISCENE_H
#define PINGPONG_KATAMARISCENE_H
#include <vector>

#include "Core/App/AppContext.h"
#include "Data/KatamariGameConfig.h"
#include "Entities/KatamariBall.h"
#include "Entities/PickupItem.h"


class KatamariScene final
{
public:
    void Initialize(AppContext& context);
    void Reset(AppContext& context);

    [[nodiscard]] KatamariBall& GetPlayerBall() noexcept;
    [[nodiscard]] std::vector<PickupItem>& GetPickups() noexcept;
    [[nodiscard]] const std::vector<PickupItem>& GetPickups() const noexcept;

    [[nodiscard]] const KatamariGameConfig& GetConfig() const noexcept;
    [[nodiscard]] KatamariGameConfig& GetConfig() noexcept;

private:
    KatamariGameConfig m_config{};
    KatamariBall m_playerBall{};
    std::vector<PickupItem> m_pickups;
};

#endif //PINGPONG_KATAMARISCENE_H