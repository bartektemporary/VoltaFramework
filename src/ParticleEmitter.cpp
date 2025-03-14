#include "VoltaFramework.hpp"
#include "Vector2.hpp"
#include <random>

ParticleEmitter::ParticleEmitter(Vector2 position, float particleLife, float speed, float spread)
    : position(position), particleLife(particleLife), speed(speed), spread(spread) {}  // Removed emitRate

void ParticleEmitter::emit(int count) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(-spread / 2.0f, spread / 2.0f);
    std::uniform_real_distribution<float> lifeDist(particleLife * 0.8f, particleLife * 1.2f);

    for (int i = 0; i < count; i++) {
        float angle = angleDist(gen) * (M_PI / 180.0f);
        float life = lifeDist(gen);
        Particle p;
        p.position = position;  // Already using Vector2
        p.velocity = {cosf(angle) * speed, sinf(angle) * speed};
        p.life = life;
        p.maxLife = life;
        p.size = 20.0f;
        particles.push_back(p);
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
    float particleLife = static_cast<float>(luaL_checknumber(L, 2));  // Shifted arguments
    float speed = static_cast<float>(luaL_checknumber(L, 3));
    float spread = static_cast<float>(luaL_checknumber(L, 4));

    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework not found");
        return 0;
    }

    ParticleEmitter emitter(*pos, particleLife, speed, spread);  // Updated constructor call
    framework->particleEmitters.push_back(emitter);

    ParticleEmitter* emitterPtr = &framework->particleEmitters.back();
    lua_pushlightuserdata(L, emitterPtr);
    luaL_getmetatable(L, "ParticleEmitter");
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