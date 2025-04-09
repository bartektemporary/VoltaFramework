#include "VoltaFramework.hpp"
#include <cmath>
#include <random>
#include <iostream> // For debugging

std::mt19937 ParticleEmitter::gen(std::random_device{}());

ParticleEmitter::ParticleEmitter(Vector2 position, float particleLife, float speed, float spread, Vector2 direction,
                                 EmitterShape shape, float width, float height, GLuint texture, bool useGPU)
    : position(position), particleLife(particleLife), speed(speed), spread(spread), direction(direction),
      shape(shape), width(width), height(height), particleTexture(texture), useGPU(useGPU), vbo(0) {
    if (shape == EmitterShape::Cone || shape == EmitterShape::Line) {
        float mag = sqrtf(direction.x * direction.x + direction.y * direction.y);
        if (mag > 0) {
            this->direction.x /= mag;
            this->direction.y /= mag;
        } else {
            this->direction = {1.0f, 0.0f};
        }
    }
    if (useGPU) {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 1024 * 4 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

ParticleEmitter::~ParticleEmitter() {
    if (useGPU && vbo != 0) {
        glDeleteBuffers(1, &vbo);
    }
}

void ParticleEmitter::setDirection(Vector2 dir) {
    float mag = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (mag > 0) {
        direction.x = dir.x / mag;
        direction.y = dir.y / mag;
    } else {
        direction = {1.0f, 0.0f};
    }
}

void ParticleEmitter::emit(int count) {
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
            p.size = 32.0f; // Base scale factor for texture
            p.texture = particleTexture;
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
            p.size = 32.0f;
            p.texture = particleTexture;
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
            float dist = sqrtf(offsetX * offsetX + offsetY * offsetY);
            if (dist > 0) {
                p.velocity = {offsetX / dist * speed, offsetY / dist * speed};
            } else {
                p.velocity = {speed, 0.0f};
            }
            p.life = life;
            p.maxLife = life;
            p.size = 32.0f;
            p.texture = particleTexture;
            particles.push_back(p);
        }
    } else if (shape == EmitterShape::Line) {
        std::uniform_real_distribution<float> tDist(-width / 2.0f, width / 2.0f);
        std::uniform_real_distribution<float> angleDist(-spread / 2.0f, spread / 2.0f);
        for (int i = 0; i < count; i++) {
            float t = tDist(gen);
            float angleOffset = angleDist(gen) * (M_PI / 180.0f);
            float offsetX = direction.x * t;
            float offsetY = direction.y * t;
            Particle p;
            p.position = {position.x + offsetX, position.y + offsetY};
            float baseAngle = atan2f(direction.y, direction.x) + (M_PI / 2.0f);
            float angle = baseAngle + angleOffset;
            p.velocity = {cosf(angle) * speed, sinf(angle) * speed};
            float life = lifeDist(gen);
            p.life = life;
            p.maxLife = life;
            p.size = 32.0f;
            p.texture = particleTexture;
            particles.push_back(p);
        }
    }
}

void ParticleEmitter::update(float dt) {
    if (useGPU) {
        std::vector<Particle> aliveParticles;
        aliveParticles.reserve(particles.size());
        for (auto& p : particles) {
            p.life -= dt;
            if (p.life > 0) {
                p.position.x += p.velocity.x * dt;
                p.position.y += p.velocity.y * dt;
                aliveParticles.push_back(p);
            }
        }
        particles = std::move(aliveParticles);
    } else {
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
}

void ParticleEmitter::render(VoltaFramework* framework) {
    if (particles.empty()) return;

    bool isWorldMode = framework->getPositionMode() == VoltaFramework::PositionMode::World;
    Camera2D* camera = isWorldMode ? framework->getCamera2D() : nullptr;
    Vector2 cameraPos = camera ? camera->getPosition() : Vector2{0, 0};
    float zoom = camera ? camera->getZoom() : 1.0f;
    if (zoom <= 0.0f) zoom = 1.0f;

    float windowWidth = (float)framework->getWidth();
    float windowHeight = (float)framework->getHeight();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float texWidth = 16.0f, texHeight = 16.0f;
    if (particleTexture != 0) {
        glBindTexture(GL_TEXTURE_2D, particleTexture);
        GLint w, h;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
        texWidth = static_cast<float>(w);
        texHeight = static_cast<float>(h);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    GLuint shaderProgram = (particleTexture != 0) ? framework->imageShaderProgram : framework->shape2DShaderProgram;
    glUseProgram(shaderProgram);

    if (useGPU) {
        glBindVertexArray(framework->textureVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Explicitly unbind EBO to avoid interference

        std::vector<float> vertexData(particles.size() * 4 * 4);
        for (size_t i = 0; i < particles.size(); ++i) {
            const auto& p = particles[i];
            if (p.life <= 0) continue;

            Vector2 adjustedPos = p.position;
            if (isWorldMode && camera) {
                adjustedPos.x = (p.position.x - cameraPos.x) * zoom + windowWidth / 2.0f;
                adjustedPos.y = (p.position.y - cameraPos.y) * zoom + windowHeight / 2.0f;
            }

            float width = p.size * zoom;
            float hw = width / 2.0f;
            float hh = (texHeight / texWidth) * hw;

            Vector2 vertices[4];
            vertices[0] = {-hw, -hh}; vertices[1] = {hw, -hh};
            vertices[2] = {hw, hh};   vertices[3] = {-hw, hh};

            float glX, glY;
            size_t base = i * 16;
            for (int j = 0; j < 4; j++) {
                framework->windowToGLCoords(adjustedPos.x + vertices[j].x, adjustedPos.y + vertices[j].y, &glX, &glY);
                vertexData[base + j * 4] = glX;
                vertexData[base + j * 4 + 1] = glY;
                vertexData[base + j * 4 + 2] = (j == 1 || j == 2) ? 1.0f : 0.0f;
                vertexData[base + j * 4 + 3] = (j == 2 || j == 3) ? 1.0f : 0.0f;
            }
        }

        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        if (particleTexture != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, particleTexture);
            glUniform1i(framework->imageTextureUniform, 0);
            glUniform3f(framework->imageColorUniform, framework->currentColor.r, framework->currentColor.g, framework->currentColor.b);
        } else {
            glUniform3f(framework->shape2DColorUniform, framework->currentColor.r, framework->currentColor.g, framework->currentColor.b);
        }

        glDrawArrays(GL_QUADS, 0, particles.size() * 4);

        // Full state cleanup
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glUseProgram(0);
    } else {
        glBindVertexArray(framework->shape2DVAO);
        glBindBuffer(GL_ARRAY_BUFFER, framework->shape2DVBO);

        if (particleTexture != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, particleTexture);
            glUniform1i(framework->imageTextureUniform, 0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);
        } else {
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
        }

        for (const auto& p : particles) {
            if (p.life <= 0) continue;

            Vector2 adjustedPos = p.position;
            if (isWorldMode && camera) {
                adjustedPos.x = (p.position.x - cameraPos.x) * zoom + windowWidth / 2.0f;
                adjustedPos.y = (p.position.y - cameraPos.y) * zoom + windowHeight / 2.0f;
            }

            float width = p.size * zoom;
            float hw = width / 2.0f;
            float hh = (texHeight / texWidth) * hw;

            Vector2 vertices[4];
            vertices[0] = {-hw, -hh}; vertices[1] = {hw, -hh};
            vertices[2] = {hw, hh};   vertices[3] = {-hw, hh};

            float modulatedColor[3] = {framework->currentColor.r, framework->currentColor.g, framework->currentColor.b};
            if (particleTexture != 0) {
                glUniform3fv(framework->imageColorUniform, 1, modulatedColor);
                float vertexData[16];
                for (int i = 0; i < 4; i++) {
                    float winX = adjustedPos.x + vertices[i].x;
                    float winY = adjustedPos.y + vertices[i].y;
                    float glX, glY;
                    framework->windowToGLCoords(winX, winY, &glX, &glY);
                    vertexData[i * 4] = glX;
                    vertexData[i * 4 + 1] = glY;
                    vertexData[i * 4 + 2] = (i == 1 || i == 2) ? 1.0f : 0.0f;
                    vertexData[i * 4 + 3] = (i == 2 || i == 3) ? 1.0f : 0.0f;
                }
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_DYNAMIC_DRAW);
            } else {
                glUniform3fv(framework->shape2DColorUniform, 1, modulatedColor);
                float vertexData[8];
                for (int i = 0; i < 4; i++) {
                    float winX = adjustedPos.x + vertices[i].x;
                    float winY = adjustedPos.y + vertices[i].y;
                    float glX, glY;
                    framework->windowToGLCoords(winX, winY, &glX, &glY);
                    vertexData[i * 2] = glX;
                    vertexData[i * 2 + 1] = glY;
                }
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_DYNAMIC_DRAW);
            }

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }

        // Clean up CPU state
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glUseProgram(0);
    }

    glDisable(GL_BLEND);
}

// Lua API Functions
int l_particleEmitter_new(lua_State* L) {
    Vector2* pos = checkVector2(L, 1);
    float particleLife = static_cast<float>(luaL_checknumber(L, 2));
    float speed = static_cast<float>(luaL_checknumber(L, 3));
    float spread = static_cast<float>(luaL_checknumber(L, 4));
    Vector2 direction = {1.0f, 0.0f};
    EmitterShape shape = EmitterShape::Cone;
    float width = 100.0f;
    float height = 100.0f;
    GLuint texture = 0;
    bool useGPU = true; // Default to GPU

    if (lua_gettop(L) >= 5 && !lua_isnil(L, 5)) direction = *checkVector2(L, 5);
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
    if (lua_gettop(L) >= 9 && !lua_isnil(L, 9)) {
        const char* texturePath = luaL_checkstring(L, 9);
        VoltaFramework* framework = getFramework(L);
        texture = framework->loadTexture(texturePath);
        if (texture == 0) luaL_error(L, "Failed to load texture: %s", texturePath);
    }
    if (lua_gettop(L) >= 10 && !lua_isnil(L, 10)) {
        const char* mode = luaL_checkstring(L, 10);
        if (strcmp(mode, "cpu") == 0) useGPU = false;
        else if (strcmp(mode, "gpu") != 0) luaL_error(L, "Invalid mode: %s (expected 'gpu' or 'cpu')", mode);
    }

    VoltaFramework* framework = getFramework(L);
    if (!framework) luaL_error(L, "Framework not found");

    framework->particleEmitters.emplace_back(*pos, particleLife, speed, spread, direction, shape, width, height, texture, useGPU);
    ParticleEmitter* emitterPtr = &framework->particleEmitters.back();

    ParticleEmitter** ud = static_cast<ParticleEmitter**>(lua_newuserdata(L, sizeof(ParticleEmitter*)));
    *ud = emitterPtr;
    luaL_getmetatable(L, "ParticleEmitter");
    lua_setmetatable(L, -2);
    return 1;
}

int l_particleEmitter_emit(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    int count = static_cast<int>(luaL_checkinteger(L, 2));
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    if (count < 0) luaL_argerror(L, 2, "Count must be non-negative");
    (*emitterPtr)->emit(count);
    return 0;
}

int l_particleEmitter_render(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    VoltaFramework* framework = getFramework(L);
    if (!framework) luaL_error(L, "Framework not found");
    (*emitterPtr)->render(framework);
    return 0;
}

int l_particleEmitter_setPosition(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    Vector2* pos = checkVector2(L, 2);
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    (*emitterPtr)->setPosition(*pos);
    return 0;
}

int l_particleEmitter_getPosition(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    Vector2 pos = (*emitterPtr)->getPosition();
    Vector2* newPos = new Vector2{pos.x, pos.y};
    lua_pushlightuserdata(L, newPos);
    luaL_getmetatable(L, "Vector2");
    lua_setmetatable(L, -2);
    return 1;
}

int l_particleEmitter_setLifetime(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    float lifetime = static_cast<float>(luaL_checknumber(L, 2));
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    if (lifetime < 0) luaL_argerror(L, 2, "Lifetime must be non-negative");
    (*emitterPtr)->setParticleLife(lifetime);
    return 0;
}

int l_particleEmitter_getLifetime(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    lua_pushnumber(L, (*emitterPtr)->getParticleLife());
    return 1;
}

int l_particleEmitter_setSpeed(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    float speed = static_cast<float>(luaL_checknumber(L, 2));
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    if (speed < 0) luaL_argerror(L, 2, "Speed must be non-negative");
    (*emitterPtr)->setSpeed(speed);
    return 0;
}

int l_particleEmitter_getSpeed(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    lua_pushnumber(L, (*emitterPtr)->getSpeed());
    return 1;
}

int l_particleEmitter_setSpread(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    float spread = static_cast<float>(luaL_checknumber(L, 2));
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    if (spread < 0) luaL_argerror(L, 2, "Spread must be non-negative");
    (*emitterPtr)->setSpread(spread);
    return 0;
}

int l_particleEmitter_getSpread(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    lua_pushnumber(L, (*emitterPtr)->getSpread());
    return 1;
}

int l_particleEmitter_setDirection(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    Vector2* dir = checkVector2(L, 2);
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    (*emitterPtr)->setDirection(*dir);
    return 0;
}

int l_particleEmitter_getDirection(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    Vector2 dir = (*emitterPtr)->getDirection();
    Vector2* newDir = new Vector2{dir.x, dir.y};
    lua_pushlightuserdata(L, newDir);
    luaL_getmetatable(L, "Vector2");
    lua_setmetatable(L, -2);
    return 1;
}

int l_particleEmitter_setShape(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    const char* shapeStr = luaL_checkstring(L, 2);
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    if (strcmp(shapeStr, "circle") == 0) (*emitterPtr)->setShape(EmitterShape::Circle);
    else if (strcmp(shapeStr, "cone") == 0) (*emitterPtr)->setShape(EmitterShape::Cone);
    else if (strcmp(shapeStr, "rectangle") == 0) (*emitterPtr)->setShape(EmitterShape::Rectangle);
    else if (strcmp(shapeStr, "line") == 0) (*emitterPtr)->setShape(EmitterShape::Line);
    else luaL_error(L, "Invalid emitter shape: %s", shapeStr);
    return 0;
}

int l_particleEmitter_getShape(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    switch ((*emitterPtr)->getShape()) {
        case EmitterShape::Circle: lua_pushstring(L, "circle"); break;
        case EmitterShape::Cone: lua_pushstring(L, "cone"); break;
        case EmitterShape::Rectangle: lua_pushstring(L, "rectangle"); break;
        case EmitterShape::Line: lua_pushstring(L, "line"); break;
        default: lua_pushstring(L, "unknown");
    }
    return 1;
}

int l_particleEmitter_setSize(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    float width = static_cast<float>(luaL_checknumber(L, 2));
    float height = static_cast<float>(luaL_checknumber(L, 3));
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    if (width < 0 || height < 0) luaL_error(L, "Width and height must be non-negative");
    (*emitterPtr)->setSize(width, height);
    return 0;
}

int l_particleEmitter_getSize(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    lua_pushnumber(L, (*emitterPtr)->getWidth());
    lua_pushnumber(L, (*emitterPtr)->getHeight());
    return 2;
}

int l_particleEmitter_setTexture(lua_State* L) {
    ParticleEmitter** emitterPtr = static_cast<ParticleEmitter**>(luaL_checkudata(L, 1, "ParticleEmitter"));
    if (!emitterPtr || !*emitterPtr) luaL_argerror(L, 1, "ParticleEmitter expected");
    VoltaFramework* framework = getFramework(L);
    if (!framework) luaL_error(L, "Framework not found");
    const char* filename = luaL_checkstring(L, 2);
    GLuint texture = framework->loadTexture(filename);
    if (texture == 0) luaL_error(L, "Failed to load texture: %s", filename);
    (*emitterPtr)->setParticleTexture(texture);
    return 0;
}