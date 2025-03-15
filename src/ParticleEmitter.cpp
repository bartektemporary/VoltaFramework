#include "VoltaFramework.hpp"
#include "Vector2.hpp"
#include <random>

ParticleEmitter::ParticleEmitter(Vector2 position, float particleLife, float speed, float spread, Vector2 direction, 
                               EmitterShape shape, float width, float height, GLuint texture)
    : position(position), particleLife(particleLife), speed(speed), spread(spread), direction(direction), 
      shape(shape), width(width), height(height), particleTexture(texture) {
    // Existing normalization logic
    if (shape == EmitterShape::Cone || shape == EmitterShape::Line) {
        float mag = sqrtf(direction.x * direction.x + direction.y * direction.y);
        if (mag > 0) {
            this->direction.x /= mag;
            this->direction.y /= mag;
        } else {
            this->direction = {1.0f, 0.0f};
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
            p.texture = particleTexture;  // Use cached texture ID
            particles.push_back(p);
        }
    } else if (shape == EmitterShape::Cone) {
        // Existing Cone logic (unchanged)
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
        // Existing Rectangle logic (unchanged)
        std::uniform_real_distribution<float> xDist(-width / 2.0f, width / 2.0f);
        std::uniform_real_distribution<float> yDist(-height / 2.0f, height / 2.0f);
        for (int i = 0; i < count; i++) {
            float offsetX = xDist(gen);
            float offsetY = yDist(gen);
            float life = lifeDist(gen);
            Particle p;
            p.position = {position.x + offsetX, position.y + offsetY};
            float dist = sqrtf(offsetX * offsetX + offsetY * offsetY);
            if (dist > 0) {
                p.velocity = {offsetX / dist * speed, offsetY / dist * speed};
            } else {
                p.velocity = {speed, 0.0f};
            }
            p.life = life;
            p.maxLife = life;
            p.size = 20.0f;
            particles.push_back(p);
        }
    } else if (shape == EmitterShape::Line) {
        // New Line shape logic (single-side emission)
        std::uniform_real_distribution<float> tDist(-width / 2.0f, width / 2.0f); // Width determines the length of the line
        std::uniform_real_distribution<float> angleDist(-spread / 2.0f, spread / 2.0f);

        // The line extends along the direction vector, centered at position
        for (int i = 0; i < count; i++) {
            float t = tDist(gen); // Position along the line
            float angleOffset = angleDist(gen) * (M_PI / 180.0f);

            // Calculate the particle's starting position along the line
            float offsetX = direction.x * t;
            float offsetY = direction.y * t;
            Particle p;
            p.position = {position.x + offsetX, position.y + offsetY};

            // Calculate the base angle perpendicular to the line
            float baseAngle = atan2f(direction.y, direction.x) + (M_PI / 2.0f); // Perpendicular to the direction
            float angle = baseAngle + angleOffset;

            // Set velocity based on the angle (single direction)
            p.velocity = {cosf(angle) * speed, sinf(angle) * speed};
            float life = lifeDist(gen);
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
    std::vector<float> texCoords;
    
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

        if (p.texture != 0) {
            texCoords.insert(texCoords.end(), {
                0.0f, 1.0f,  // Top-left -> Bottom-left
                1.0f, 1.0f,  // Top-right -> Bottom-right
                1.0f, 0.0f,  // Bottom-right -> Top-right
                0.0f, 0.0f   // Bottom-left -> Top-left
            });
        }
    }

    glUseProgram(framework->shaderProgram);
    float color[4] = {framework->currentColor[0], framework->currentColor[1], framework->currentColor[2], 1.0f};
    glUniform3fv(framework->colorUniform, 1, color);

    glBindVertexArray(framework->textureVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, framework->textureVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    if (!texCoords.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, framework->textureVBO + 1);
        glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(float), texCoords.data(), GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    }

    size_t particleCount = particles.size();
    size_t offset = 0;
    while (offset < particleCount) {
        GLuint currentTexture = particles[offset].texture;
        size_t batchSize = 1;
        
        while (offset + batchSize < particleCount && particles[offset + batchSize].texture == currentTexture) {
            batchSize++;
        }

        glUniform1i(framework->useTextureUniform, currentTexture != 0);
        if (currentTexture != 0) {
            glBindTexture(GL_TEXTURE_2D, currentTexture);
            glUniform1i(framework->textureUniform, 0);
        }

        glDrawArrays(GL_QUADS, offset * 4, batchSize * 4);
        offset += batchSize;
    }

    glDisableVertexAttribArray(0);
    if (!texCoords.empty()) {
        glDisableVertexAttribArray(1);
    }
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
    Vector2 direction = {1.0f, 0.0f};
    EmitterShape shape = EmitterShape::Cone;
    float width = 100.0f;
    float height = 100.0f;

    if (lua_gettop(L) >= 5 && !lua_isnil(L, 5)) {
        Vector2* dir = checkVector2(L, 5);
        direction = *dir;
    }
    if (lua_gettop(L) >= 6 && !lua_isnil(L, 6)) {
        const char* shapeStr = luaL_checkstring(L, 6);
        if (strcmp(shapeStr, "circle") == 0) shape = EmitterShape::Circle;
        else if (strcmp(shapeStr, "cone") == 0) shape = EmitterShape::Cone;
        else if (strcmp(shapeStr, "rectangle") == 0) shape = EmitterShape::Rectangle;
        else if (strcmp(shapeStr, "line") == 0) shape = EmitterShape::Line;
        else luaL_error(L, "Invalid emitter shape: %s", shapeStr);
    }
    if (lua_gettop(L) >= 7 && !lua_isnil(L, 7)) width = static_cast<float>(luaL_checknumber(L, 7));
    if (lua_gettop(L) >= 8 && !lua_isnil(L, 8)) height = static_cast<float>(luaL_checknumber(L, 8));

    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework not found");
        return 0;
    }

    // Reserve space to prevent reallocation
    if (framework->particleEmitters.capacity() < framework->particleEmitters.size() + 1) {
        framework->particleEmitters.reserve(framework->particleEmitters.size() + 10); // Reserve extra space
    }

    framework->particleEmitters.emplace_back(*pos, particleLife, speed, spread, direction, shape, width, height);
    ParticleEmitter* emitterPtr = &framework->particleEmitters.back();
    
    // Push the pointer as userdata with the metatable
    ParticleEmitter** ud = static_cast<ParticleEmitter**>(lua_newuserdata(L, sizeof(ParticleEmitter*)));
    *ud = emitterPtr;
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
    } else if (strcmp(shapeStr, "rectangle") == 0) {
        emitter->setShape(EmitterShape::Rectangle);
    } else if (strcmp(shapeStr, "line") == 0) {
        emitter->setShape(EmitterShape::Line);
    } else {
        luaL_error(L, "Invalid emitter shape: %s (expected 'circle', 'cone', 'rectangle', or 'line')", shapeStr);
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
        case EmitterShape::Rectangle:
            lua_pushstring(L, "rectangle");
            break;
        case EmitterShape::Line:
            lua_pushstring(L, "line");
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
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitterPtr || !*emitterPtr) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    int count = static_cast<int>(luaL_checkinteger(L, 2));
    if (count < 0) {
        luaL_argerror(L, 2, "Count must be non-negative");
        return 0;
    }
    (*emitterPtr)->emit(count);
    return 0;
}

int l_particleEmitter_render(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitterPtr || !*emitterPtr) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework not found");
        return 0;
    }
    (*emitterPtr)->render(framework);
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

int l_particleEmitter_setPosition(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitterPtr || !*emitterPtr) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    Vector2* position = checkVector2(L, 2);
    (*emitterPtr)->setPosition(*position);
    return 0;
}

int l_particleEmitter_getPosition(lua_State* L) {
    ParticleEmitter* emitter = static_cast<ParticleEmitter*>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitter) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    Vector2 pos = emitter->getPosition();
    Vector2* newPos = new Vector2{pos.x, pos.y};
    lua_pushlightuserdata(L, newPos);
    luaL_getmetatable(L, "Vector2");
    lua_setmetatable(L, -2);
    return 1; // Return the Vector2 position
}

int l_particleEmitter_setTexture(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitterPtr || !*emitterPtr) {
        luaL_argerror(L, 1, "ParticleEmitter expected");
        return 0;
    }
    
    VoltaFramework* framework = getFramework(L);
    if (!framework) {
        luaL_error(L, "Framework not found");
        return 0;
    }

    const char* filename = luaL_checkstring(L, 2);
    GLuint texture = framework->loadTexture(filename);
    
    if (texture == 0) {
        luaL_error(L, "Failed to load texture '%s' - check file path or format", filename);
        return 0;
    }
    
    (*emitterPtr)->setParticleTexture(texture);
    return 0;
}