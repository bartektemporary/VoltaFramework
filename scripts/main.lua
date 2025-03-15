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
end)