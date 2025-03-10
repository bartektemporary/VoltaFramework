local vector2 = volta.vector2

-- Initialize position as a Vector2
local defaultPosition = vector2.new(400, 300)

-- Circle movement
local speed<const> = 200

local t = 0
local sound

local fpsCounter = 0
local fpsTimer = 0
local currentFps = 0

function init()
    volta.window.setState("borderlessMaximized")
    volta.graphics.setFilter("nearest")
    volta.window.setIcon("tree.png")

    sound = volta.audio.loadAudio("music.mp3")
    if sound then
        sound:setVolume(1)
        sound:setLooped(true)
        sound:play()
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

    if volta.input.isKeyDown("w") then
        newY = newY - speed * dt
    end
    if volta.input.isKeyDown("s") then
        newY = newY + speed * dt
    end
    if volta.input.isKeyDown("a") then
        newX = newX - speed * dt
    end
    if volta.input.isKeyDown("d") then
        newX = newX + speed * dt
    end

    defaultPosition = vector2.new(newX, newY)

    volta.graphics.drawImage("tree.png", vector2.new(100, 100), vector2.new(200, 200))

    volta.graphics.setColor(0.5, 1, 0)
    volta.graphics.rectangle(true, defaultPosition, vector2.new(50, 50))

    volta.graphics.drawLine(vector2.new(100, 100), vector2.new(500, 500), 10)
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


--[[local time = 0
local fullscreen = false

function init()
    volta.window.setTitle("Test window title")
    volta.window.setFullscreen(true)
    fullscreen = true
end

function update(dt)
    -- Increment time to animate noise
    time = time + dt
    
    -- Exit on escape key
    if volta.input.isKeyDown("escape") then
        os.exit()
    end

    if volta.input.isKeyDown("f11") then
        volta.window.setFullscreen(not fullscreen)
        fullscreen = not fullscreen
    end

    for i = 0, 39 do
        for j = 0, 29 do
            local noise = math.noise2d(i * 0.1 + time, j * 0.1)
            volta.graphics.setColor(noise, noise, noise)
            volta.graphics.rectangle(true, i * 20, j * 20, 20, 20)
        end
    end
end]]