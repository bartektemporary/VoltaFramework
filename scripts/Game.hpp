#ifndef GAME_HPP
#define GAME_HPP

#include "VoltaFramework.hpp"

class Game : public GameBase {
public:
    void init(VoltaFramework* framework) override;
    void update(VoltaFramework* framework, float dt) override;
};

#endif