//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_PLANECONSTRAINT_H
#define PINGPONG_PLANECONSTRAINT_H

#include <SimpleMath.h>

class PlaneConstraint final {
public:
    static void ClampToGroundPlane(DirectX::SimpleMath::Vector3 &position, float y = 0.0f) noexcept;
};

#endif //PINGPONG_PLANECONSTRAINT_H
