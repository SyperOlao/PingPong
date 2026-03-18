//
// Created by SyperOlao on 17.03.2026.
//

#include "PongGame.h"
#include "../../Core/Graphics/Color.h"
#include "../../Core/App/AppContext.h"
#include "../../Core/Graphics2D/ShapeRenderer2D.h"
void PongGame::Initialize(AppContext &context) {
    (void) context;
}

void PongGame::Update(AppContext &context, float deltaTime) {
    (void) context;
    (void) deltaTime;
}

void PongGame::Render(AppContext &context) {
    context.Shape2D->DrawFilledRect(40.0f, 250.0f, 20.0f, 120.0f, Color(1.0f, 1.0f, 1.0f, 1.0f));
    context.Shape2D->DrawFilledRect(760.0f, 250.0f, 20.0f, 120.0f, Color(1.0f, 1.0f, 1.0f, 1.0f));
    context.Shape2D->DrawFilledRect(395.0f, 295.0f, 10.0f, 10.0f, Color(1.0f, 1.0f, 1.0f, 1.0f));
}
