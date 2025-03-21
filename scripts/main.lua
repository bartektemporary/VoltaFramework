local vector2 = volta.vector2

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

-- Define colors for tweening
local greenColor = volta.color.new(0.5, 1, 0)  -- Starting color (green)
local blueColor = volta.color.new(0, 0, 1)     -- Target color (blue)

function volta.init()
    volta.window.setState("borderlessMaximized")
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

    volta.graphics.loadFont("Minecraft.ttf", 24)
    volta.graphics.setFont("Minecraft.ttf")

    local red = volta.color.new(1.0, 0.0, 0.0)
    local green = volta.color.fromHSV(120, 1.0, 1.0)
    
    print("Red:", red.r, red.g, red.b)
    print("Green:", green.r, green.g, green.b)
    
    local styles = {"linear", "sine", "quad", "cubic", "quart", "quint", "exponential", "circular", "back", "elastic", "bounce"}
    local directions = {"in", "out", "inout"}
    
    print("Tweening examples (Red to Green) at alpha = 0.5:")
    for _, style in ipairs(styles) do
        for _, direction in ipairs(directions) do
            local result = red:tween(green, 0.5, direction, style)
            print(string.format("%s %s: %.6f %.6f %.6f", style:gsub("^%l", string.upper), direction, result.r, result.g, result.b))
        end
    end
end

function volta.update(dt)
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
    volta.graphics.setColor(volta.color.new(0.5, 0.2, 0.1)) -- Dark red-orange
    fire_embers:render()
    
    -- Base flame (orange-red)
    volta.graphics.setColor(volta.color.new(1.0, 0.4, 0.0)) -- Orange-red
    fire_base:render()

    -- Flickering tips (yellow)
    volta.graphics.setColor(volta.color.new(1.0, 0.8, 0.0)) -- Yellow
    fire_tips:render()

    -- Core (white-yellow)
    volta.graphics.setColor(volta.color.new(1.0, 1.0, 0.8)) -- White-yellow
    fire_core:render()

    local treePos = leftBound:tween(rightBound, t, "inout", "quad")

    volta.graphics.setColor(volta.color.new(1, 1, 1))

    -- Tween the rectangle's color from green to blue using "back" style
    local tweenedColor = greenColor:tween(blueColor, t, "inout", "quad")
    volta.graphics.setColor(tweenedColor)
    volta.graphics.rectangle(true, defaultPosition, vector2.new(50, 50))

    volta.graphics.drawLine(vector2.new(100, 100), vector2.new(500, 500), 10)

    volta.graphics.setColor(volta.color.new(1, 1, 1))
    volta.graphics.drawImage("tree.png", treePos, vector2.new(200, 200))

    volta.graphics.setFont("Minecraft.ttf")
    volta.graphics.drawText("Hello, World!", volta.vector2.new(1000, 150), 1.5) -- Text at (100,100), 1.5x scale
end

volta.input.keyPressed("up", function()
    volta.audio.setGlobalVolume(volta.audio.getGlobalVolume() + 0.1)
end)

volta.input.keyPressed("down", function()
    volta.audio.setGlobalVolume(volta.audio.getGlobalVolume() - 0.1)
end)

volta.input.keyPressed("escape", function()
    os.exit()
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
end)