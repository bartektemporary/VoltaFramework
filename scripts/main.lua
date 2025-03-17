t = {}
mt = { __index = function() end }
setmetatable(t, mt)
print(t.newkey)  -- Should trigger cache refresh
t.newkey = 42     -- Should use cached __newindex
print(t.newkey)  -- Should trigger cache refresh

local time = 0

local vertexShader = [[
    #version 330 core
    layout (location = 0) in vec2 aPos;
    out vec2 FragCoord;

    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        FragCoord = (aPos + 1.0) * 0.5; // Normalize to [0, 1] for easier distance calc
    }
]]

local fragmentShader = [[
    #version 330 core
    in vec2 FragCoord;
    out vec4 FragColor;

    uniform vec3 uColor;
    uniform float uTime;
    uniform vec2 uRectPos;    // Bottom-left in [0, 1] coords
    uniform vec2 uRectSize;   // Size in [0, 1] coords

    // Signed distance function for a rectangle in [0, 1] space
    float sdRectangle(vec2 p, vec2 pos, vec2 size) {
        vec2 d = abs(p - (pos + size * 0.5)) - (size * 0.5);
        return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
    }

    void main() {
        // Compute distance from fragment to rectangle (in [0, 1] space)
        float d = sdRectangle(FragCoord, uRectPos, uRectSize);

        // Base color (rectangle interior)
        vec3 baseColor = uColor * step(0.0, -d);

        // Glow effect outside the rectangle with very tight range
        float glow = 0.0;
        if (d > 0.0) {
            glow = 1.0 * exp(-d * 100.0); // Glow intensity
            glow = clamp(glow, 0.0, 1.0);
        }

        // Pulse effect for brightness variation
        float pulse = sin(uTime * 2.0) * 0.25 + 1.25; // Range: 1.0 to 1.5

        // Apply pulse to both base color and glow to sync brightness
        vec3 rectColor = baseColor * pulse;
        vec3 glowColor = (glow * uColor) * pulse;

        // Combine colors
        vec3 finalColor = rectColor + glowColor;

        // Normalize to prevent hue shift while increasing brightness
        float maxComponent = max(max(finalColor.r, finalColor.g), finalColor.b);
        if (maxComponent > 1.0) {
            finalColor = finalColor / maxComponent; // Preserve hue by normalizing
        }

        // Background
        if (d > 0.01) { // Reduced threshold to match tight glow range
            finalColor = vec3(0.1, 0.1, 0.1); // Dark gray background
        }

        // Debug: Visualize distance (uncomment to test)
        // FragColor = vec4(vec3(d), 1.0);

        FragColor = vec4(finalColor, 1.0);
    }
]]

function init()
    local success = volta.graphics.setCustomShader(vertexShader, fragmentShader)
    if not success then
        print("Failed to set custom shader")
        return
    end

    volta.graphics.setUseCustomShader(true)
    print("Custom shader applied")
end

function update(dt)
    time = time + dt

    -- Set color every frame
    volta.graphics.setColor(1.0, 0.5, 0.0) -- Orange

    -- Update uniforms
    volta.graphics.setCustomShaderUniform("uTime", volta.getRunningTime())

    local windowWidth, windowHeight = volta.window.getSize()
    local centerX, centerY = windowWidth / 2, windowHeight / 2

    -- Rectangle position and size in window coordinates
    local radius = 100
    local speed = 2.0
    local rectPos = volta.vector2.new(
        centerX + math.cos(time * speed) * radius,
        centerY + math.sin(time * speed) * radius
    )
    local rectSize = volta.vector2.new(50, 50)

    -- Convert to [0, 1] coordinates (normalized screen space)
    local normPosX = rectPos.x / windowWidth
    local normPosY = 1.0 - (rectPos.y / windowHeight) -- Invert Y for [0, 1] top-left origin
    local normSizeX = rectSize.x / windowWidth
    local normSizeY = rectSize.y / windowHeight
    local glPos = volta.vector2.new(normPosX, normPosY)
    local glSize = volta.vector2.new(normSizeX, normSizeY)

    volta.graphics.setCustomShaderUniform("uRectPos", glPos)
    volta.graphics.setCustomShaderUniform("uRectSize", glSize)

    -- Render a full-screen quad (using the framework's rectangle with max size)
    volta.graphics.rectangle(true, volta.vector2.new(0, 0), volta.vector2.new(windowWidth, windowHeight))
end

--[[local vector2 = volta.vector2

local defaultPosition = vector2.new(400, 300)
local speed<const> = 200

local t = 0
local tweenDirection = 1
local tweenSpeed = 0.5
local leftBound = vector2.new(100, 100)
local rightBound = vector2.new(700, 100)
local sound

local fpsCounter = 0
local fpsTimer = 0
local currentFps = 0

local fire_emitter 
local lastVelocity = vector2.new(0, 0) -- Track last movement direction

function init()
    volta.window.setState("maximized")
    volta.graphics.setFilter("nearest")
    volta.window.setIcon("tree.png")

    sound = volta.audio.loadAudio("music.mp3")
    if sound then
        sound:setVolume(1)
        sound:setLooped(true)
        sound:play()
    end

    fire_base = volta.particleEmitter.new(
        volta.vector2.new(400, 300),
        1.0,                        -- lifetime
        120,                        -- moderate speed
        90,                         -- wide spread for base
        volta.vector2.new(0, -1),   -- upward
        "cone"
    )

    -- Core flame (narrow cone for bright center)
    fire_core = volta.particleEmitter.new(
        volta.vector2.new(400, 300),
        0.7,                        -- shorter lifetime
        180,                        -- faster speed
        20,                         -- narrow spread
        volta.vector2.new(0, -1),
        "cone"
    )

    -- Flickering tips (circle for top wisps)
    fire_tips = volta.particleEmitter.new(
        volta.vector2.new(400, 250), -- higher position
        0.5,                        -- short lifetime
        100,                        -- moderate speed
        360,                        -- full spread
        volta.vector2.new(0, -1),
        "circle"
    )

    -- Smoke/embers (sparse circle)
    fire_embers = volta.particleEmitter.new(
        volta.vector2.new(400, 200), -- even higher
        1.5,                        -- longer lifetime
        80,                         -- slower speed
        360,                        -- full spread
        volta.vector2.new(0, -1),
        "circle"
    )

    -- Check if a gamepad is connected
    if volta.input.isGamepadConnected(0) then
        print("Gamepad 0 is connected")
    end

    -- Check if a specific gamepad button is down
    if volta.input.isGamepadButtonDown(0, 0) then
        print("Button 0 is down on gamepad 0")
    end

end

function update(dt)
    fpsCounter = fpsCounter + 1
    fpsTimer = fpsTimer + dt
    if fpsTimer >= 1 then
        currentFps = fpsCounter
        fpsCounter = 0
        fpsTimer = fpsTimer - 1
        print("FPS: " .. currentFps)
    end

    local newX = defaultPosition.x
    local newY = defaultPosition.y
    local velocity = vector2.new(0, 0) -- Current frame's velocity

    local moved = false
    if volta.input.isKeyDown("w") then
        newY = newY - speed * dt
        moved = true
    end
    if volta.input.isKeyDown("s") then
        newY = newY + speed * dt
        moved = true
    end
    if volta.input.isKeyDown("a") then
        newX = newX - speed * dt
        moved = true
    end
    if volta.input.isKeyDown("d") then
        newX = newX + speed * dt
        moved = true
    end

    -- Update emitter position
    defaultPosition = vector2.new(newX, newY)

    t = t + dt * tweenSpeed * tweenDirection
    if t >= 1 then
        t = 1 - (t - 1)
        tweenDirection = -1
    elseif t <= 0 then
        t = -t
        tweenDirection = 1
    end

    local treePos = leftBound:tween(rightBound, t, "in", "back")

    volta.graphics.setColor(1, 1, 1)

    volta.graphics.setColor(1, 1, 1)
    volta.graphics.drawImage("tree.png", treePos, vector2.new(200, 200))

    volta.graphics.setColor(0.5, 1, 0)
    volta.graphics.rectangle(true, defaultPosition, vector2.new(50, 50))

    volta.graphics.drawLine(vector2.new(100, 100), vector2.new(500, 500), 10)

    -- Dynamic sway based on time
    local time = volta.getRunningTime()
    local sway = math.sin(time * 2) * 10
    local base_x = 400 + sway
    local flicker = math.sin(time * 5) * 0.5 + 0.5 -- 0 to 1 range

    -- Update positions with sway
    fire_base:setPosition(volta.vector2.new(base_x, 300))
    fire_core:setPosition(volta.vector2.new(base_x, 300))
    fire_tips:setPosition(volta.vector2.new(base_x, 250 + sway * 0.5))
    fire_embers:setPosition(volta.vector2.new(base_x, 200 + sway * 0.3))

    -- Dynamic adjustments
    fire_base:setSpeed(120 + flicker * 20)
    fire_core:setSpeed(180 + flicker * 30)
    fire_tips:setSpeed(100 + flicker * 20)
    fire_base:setSpread(90 + flicker * 20)

    -- Emit particles
    fire_base:emit(6 + math.floor(flicker * 2))    -- 6-8 particles
    fire_core:emit(3 + math.floor(flicker * 2))    -- 3-5 particles
    fire_tips:emit(2 + math.floor(flicker * 1))    -- 2-3 particles
    fire_embers:emit(1)                            -- sparse embers

    -- Render layers (back to front)
    -- Embers/Smoke (dark red/gray)
    volta.graphics.setColor(0.5, 0.2, 0.1) -- Dark red-orange
    fire_embers:render()

    -- Base flame (orange-red)
    volta.graphics.setColor(1.0, 0.4, 0.0) -- Orange-red
    fire_base:render()

    -- Flickering tips (yellow)
    volta.graphics.setColor(1.0, 0.8, 0.0) -- Yellow
    fire_tips:render()

    -- Core (white-yellow)
    volta.graphics.setColor(1.0, 1.0, 0.8) -- White-yellow
    fire_core:render()
end

volta.input.keyPressed("up", function()
    volta.audio.setGlobalVolume(volta.audio.getGlobalVolume() + 0.1)
end)

volta.input.keyPressed("down", function()
    volta.audio.setGlobalVolume(volta.audio.getGlobalVolume() - 0.1)
end)

volta.input.keyPressed("i", function()
    print(volta.getRunningTime())
    local pressedKeys = volta.input.getPressedKeys()
    for i = 1, #pressedKeys do
        print("Pressed key:", pressedKeys[i])
    end

    -- Example usage in Lua for mouse buttons
    local pressedButtons = volta.input.getPressedMouseButtons()
    for i = 1, #pressedButtons do
        print("Pressed mouse button:", pressedButtons[i])
    end
end)

-- Register a callback for when a gamepad is connected
volta.input.gamepadConnected(function(gamepadId)
    print("Gamepad connected:", gamepadId)
end)

-- Register a callback for when a gamepad is disconnected
volta.input.gamepadDisconnected(function(gamepadId)
    print("Gamepad disconnected:", gamepadId)
end)

-- Register a callback for when a specific gamepad button is pressed
-- Example: Button 0 (A button on Xbox controller)
volta.input.getGamepadButtonPressed(0, 0, function(gamepadId, button)
    print("Gamepad button pressed:", gamepadId, button)
end)]]