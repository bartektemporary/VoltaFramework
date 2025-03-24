#include "Game.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

void Game::init(VoltaFramework* framework) {
    std::cout << "Game Initialized from scripts/main.cpp\n";
    framework->setWindowTitle("C++ Game with Color");
    framework->setVsync(true);
    framework->loadFont("Minecraft.ttf", 24);

    framework->registerCppKeyPressCallback("space", []() {
        std::cout << "Space pressed from C++!\n";
    });
}

Color white(1.0f, 1.0f, 1.0f);
Color black(0.0f, 0.0f, 0.0f);

void Game::update(VoltaFramework* framework, float dt) {
    Color red = Color::fromRGB(255, 0, 0);
    framework->setColor(red);
    framework->drawRectangle(true, Vector2{400, 300}, Vector2{100, 100}, 0.0f);

    Color green = Color::fromHSV(120, 1.0f, 1.0f);
    framework->setColor(green);
    framework->drawCircle(false, Vector2{500, 400}, 50.0f);

    Color blue = Color::fromHex("#0000FF");
    framework->setColor(white);
    framework->drawLine(Vector2{300, 200}, Vector2{600, 500}, 2.0f);

    Color tweened = white.tween(black, dt, "in", "sine"); // Example tweening
    framework->setColor(tweened);
    framework->drawText("Hello from C++!", 350, 350, 1.0f);
}