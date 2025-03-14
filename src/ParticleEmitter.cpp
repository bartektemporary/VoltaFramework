#include "VoltaFramework.hpp"
#include "Vector2.hpp"
#include <random>

ParticleEmitter::ParticleEmitter(Vector2 position, float particleLife, float speed, float spread, Vector2 direction, EmitterShape shape, float width, float height)
    : position(position), particleLife(particleLife), speed(speed), spread(spread), direction(direction), shape(shape), width(width), height(height) {
    // Normalize the direction vector if using Cone shape
    if (shape == EmitterShape::Cone) {
        float mag = sqrtf(direction.x * direction.x + direction.y * direction.y);
        if (mag > 0) {
            this->direction.x /= mag;
            this->direction.y /= mag;
        } else {
            this->direction = {1.0f, 0.0f}; // Default to right if zero vector
        }
    }
}

void ParticleEmitter::emit(int count) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> lifeDist(particleLife * 0.8f, particleLife * 1.2f);

    if (shape == EmitterShape::Circle) {
        std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
        for (int i = 0; i < count; i++) {
            float angle = angleDist(gen) * (M_PI / 180.0f);
            float life = lifeDist(gen);
            Particle p;
            p.position = position;
            p.velocity = {cosf(angle) * speed, sinf(angle) * speed};
            p.life = life;
            p.maxLife = life;
            p.size = 20.0f;
            particles.push_back(p);
        }
    } else if (shape == EmitterShape::Cone) {
        std::uniform_real_distribution<float> angleDist(-spread / 2.0f, spread / 2.0f);
        float baseAngle = atan2f(direction.y, direction.x);
        for (int i = 0; i < count; i++) {
            float angleOffset = angleDist(gen) * (M_PI / 180.0f);
            float angle = baseAngle + angleOffset;
            float life = lifeDist(gen);
            Particle p;
            p.position = position;
            p.velocity = {cosf(angle) * speed, sinf(angle) * speed};
            p.life = life;
            p.maxLife = life;
            p.size = 20.0f;
            particles.push_back(p);
        }
    } else if (shape == EmitterShape::Rectangle) {
        std::uniform_real_distribution<float> xDist(-width / 2.0f, width / 2.0f);
        std::uniform_real_distribution<float> yDist(-height / 2.0f, height / 2.0f);
        for (int i = 0; i < count; i++) {
            float offsetX = xDist(gen);
            float offsetY = yDist(gen);
            float life = lifeDist(gen);
            Particle p;
            p.position = {position.x + offsetX, position.y + offsetY};
            // Velocity is directed outward from the center, scaled by distance
            float dist = sqrtf(offsetX * offsetX + offsetY * offsetY);
            if (dist > 0) {
                p.velocity = {offsetX / dist * speed, offsetY / dist * speed};
            } else {
                p.velocity = {speed, 0.0f}; // Default right if at center
            }
            p.life = life;
            p.maxLife = life;
            p.size = 20.0f;
            particles.push_back(p);
        }
    }
}

void ParticleEmitter::update(float dt) {
    for (auto it = particles.begin(); it != particles.end();) {
        it->life -= dt;
        if (it->life <= 0) {
            it = particles.erase(it);
        } else {
            it->position.x += it->velocity.x * dt;
            it->position.y += it->velocity.y * dt;
            ++it;
        }
    }
}

void ParticleEmitter::render(VoltaFramework* framework) {
    if (particles.empty()) return;

    std::vector<float> vertices;
    for (const auto& p : particles) {
        float alpha = p.life / p.maxLife;
        float glX, glY;
        framework->windowToGLCoords(p.position.x - p.size / 2, p.position.y - p.size / 2, &glX, &glY);
        float right = glX + (p.size / framework->getWidth()) * 2.0f;
        float bottom = glY - (p.size / framework->getHeight()) * 2.0f;

        vertices.insert(vertices.end(), {
            glX, glY,
            right, glY,
            right, bottom,
            glX, bottom
        });
    }

    glUseProgram(framework->shaderProgram);
    float color[4] = {framework->currentColor[0], framework->currentColor[1], framework->currentColor[2], 1.0f};
    glUniform3fv(framework->colorUniform, 1, color);
    glUniform1i(framework->useTextureUniform, 0);

    glBindVertexArray(framework->shapeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, framework->shapeVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    glDrawArrays(GL_QUADS, 0, vertices.size() / 2);

    glBindVertexArray(0);
}

void VoltaFramework::renderParticles(float dt) {
    for (auto& emitter : particleEmitters) {
        emitter.update(dt);
    }
}

int l_particleEmitter_new(lua_State* L) {
    Vector2* pos = checkVector2(L, 1);
    float particleLife = static_cast<float>(luaL_checknumber(L, 2));
    float speed = static_cast<float>(luaL_checknumber(L, 3));
    float spread = static_cast<float>(luaL_checknumber(L, 4));
    Vector2 direction = {1.0f, 0.0f}; // Default direction (right)
    EmitterShape shape = EmitterShape::Cone; // Default to cone
    float width = 100.0f;  // Default rectangle width
    float height = 100.0f; // Default rectangle height

    // Optional direction parameter
    if (lua_gettop(L) >= 5 && !lua_isnil(L, 5)) {
        Vector2* dir = checkVector2(L, 5);
        direction = *dir;
    }
    // Optional shape parameter
    if (lua_gettop(L) >= 6 && !lua_isnil(L, 6)) {
        const char* shapeStr = luaL_checkstring(L, 6);
        if (strcmp(shapeStr, "circle") == 0) {
            shape = EmitterShape::Circle;
        } else if (strcmp(shapeStr, "cone") == 0) {
            shape = EmitterShape::Cone;
        } else if (strcmp(shapeStr, "rectangle") == 0) {
            shape = EmitterShape::Rectangle;
        } else {
            luaL_error(L, "Invalid emitter shape: %s (expected 'circle', 'cone', or 'rectangle')", shapeStr);
            return 0;
        }
    }
    // Optional width and height parameters for rectangle
    if (lua_gettop(L) >= 7 && !lua_isnil(L, 7)) {
        width = static_cast<float>(luaL_checknumber(L, 7));
    }
    if (lua_gettop(L) >= 8 && !lua_isnil(L, 8)) {
        height = static_cast<float>(luaL_checknumber(L, 8));
    }

    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework not found");
        return 0;
    }

    ParticleEmitter emitter(*pos, particleLife, speed, spread, direction, shape, width, height);
    framework->particleEmitters.push_back(emitter);

    ParticleEmitter* emitterPtr = &framework->particleEmitters.back();
    lua_pushlightuserdata(L, emitterPtr);
    luaL_getmetatable(L, "ParticleEmitter");
    lua_setmetatable(L, -2);
    return 1;
}

int l_particleEmitter_setSize(lua_State* L) {
    ParticleEmitter* emitter = static_cast<ParticleEmitter*>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitter) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    float width = static_cast<float>(luaL_checknumber(L, 2));
    float height = static_cast<float>(luaL_checknumber(L, 3));
    if (width < 0 || height < 0) {
        luaL_error(L, "Width and height must be non-negative");
        return 0;
    }
    emitter->setSize(width, height);
    return 0;
}

int l_particleEmitter_getSize(lua_State* L) {
    ParticleEmitter* emitter = static_cast<ParticleEmitter*>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitter) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    lua_pushnumber(L, emitter->getWidth());
    lua_pushnumber(L, emitter->getHeight());
    return 2; // Return width and height
}

int l_particleEmitter_setShape(lua_State* L) {
    ParticleEmitter* emitter = static_cast<ParticleEmitter*>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitter) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    const char* shapeStr = luaL_checkstring(L, 2);
    if (strcmp(shapeStr, "circle") == 0) {
        emitter->setShape(EmitterShape::Circle);
    } else if (strcmp(shapeStr, "cone") == 0) {
        emitter->setShape(EmitterShape::Cone);
    } else {
        luaL_error(L, "Invalid emitter shape: %s (expected 'circle' or 'cone')", shapeStr);
        return 0;
    }
    return 0;
}

int l_particleEmitter_getShape(lua_State* L) {
    ParticleEmitter* emitter = static_cast<ParticleEmitter*>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitter) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    switch (emitter->getShape()) {
        case EmitterShape::Circle:
            lua_pushstring(L, "circle");
            break;
        case EmitterShape::Cone:
            lua_pushstring(L, "cone");
            break;
        default:
            lua_pushstring(L, "unknown");
    }
    return 1;
}

int l_particleEmitter_setDirection(lua_State* L) {
    ParticleEmitter* emitter = static_cast<ParticleEmitter*>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitter) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    Vector2* direction = checkVector2(L, 2);
    float mag = sqrtf(direction->x * direction->x + direction->y * direction->y);
    if (mag > 0) {
        emitter->direction.x = direction->x / mag;
        emitter->direction.y = direction->y / mag;
    } else {
        emitter->direction = {1.0f, 0.0f}; // Default to right if zero vector
    }
    return 0;
}

int l_particleEmitter_getDirection(lua_State* L) {
    ParticleEmitter* emitter = static_cast<ParticleEmitter*>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitter) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    Vector2* direction = new Vector2{emitter->direction.x, emitter->direction.y};
    lua_pushlightuserdata(L, direction);
    luaL_getmetatable(L, "Vector2");
    lua_setmetatable(L, -2);
    return 1;
}

int l_particleEmitter_emit(lua_State* L) {
    ParticleEmitter* emitter = static_cast<ParticleEmitter*>(lua_touserdata(L, 1));
    if (!luaL_checkudata(L, 1, "ParticleEmitter")) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    int count = static_cast<int>(luaL_checkinteger(L, 2));
    if (count < 0) {
        luaL_argerror(L, 2, "Count must be non-negative");
        return 0;
    }
    emitter->emit(count);
    return 0;
}

int l_particleEmitter_render(lua_State* L) {
    ParticleEmitter* emitter = static_cast<ParticleEmitter*>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitter) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }

    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework not found");
        return 0;
    }

    emitter->render(framework);
    return 0;
}

int l_particleEmitter_setLifetime(lua_State* L) {
    ParticleEmitter* emitter = static_cast<ParticleEmitter*>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitter) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    float lifetime = static_cast<float>(luaL_checknumber(L, 2));
    if (lifetime < 0) {
        luaL_argerror(L, 2, "Lifetime must be non-negative");
        return 0;
    }
    emitter->particleLife = lifetime;
    return 0;  // No return values
}

int l_particleEmitter_setSpeed(lua_State* L) {
    ParticleEmitter* emitter = static_cast<ParticleEmitter*>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitter) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    float speed = static_cast<float>(luaL_checknumber(L, 2));
    if (speed < 0) {
        luaL_argerror(L, 2, "Speed must be non-negative");
        return 0;
    }
    emitter->speed = speed;
    return 0;  // No return values
}

int l_particleEmitter_setSpread(lua_State* L) {
    ParticleEmitter* emitter = static_cast<ParticleEmitter*>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitter) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    float spread = static_cast<float>(luaL_checknumber(L, 2));
    if (spread < 0) {
        luaL_argerror(L, 2, "Spread must be non-negative");
        return 0;
    }
    emitter->spread = spread;
    return 0;  // No return values
}

int l_particleEmitter_getLifetime(lua_State* L) {
    ParticleEmitter* emitter = static_cast<ParticleEmitter*>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitter) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    lua_pushnumber(L, emitter->particleLife);
    return 1;  // Return 1 value (the lifetime)
}

int l_particleEmitter_getSpeed(lua_State* L) {
    ParticleEmitter* emitter = static_cast<ParticleEmitter*>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitter) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    lua_pushnumber(L, emitter->speed);
    return 1;  // Return 1 value (the speed)
}

int l_particleEmitter_getSpread(lua_State* L) {
    ParticleEmitter* emitter = static_cast<ParticleEmitter*>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitter) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    lua_pushnumber(L, emitter->spread);
    return 1;  // Return 1 value (the spread)
}