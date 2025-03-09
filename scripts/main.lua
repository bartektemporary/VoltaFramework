local x, y = 400, 300
local rectangleX = x
local rectangleY = y

--circle movement
local speed<const> = 200
local starting<const> = x
local destination<const> = x + 400

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

    local buf = volta.buffer.alloc(16) -- 16 bytes

    buf:writeUInt8(255, 0)
    print(buf:readUInt8(0, 0)) -- 255

    buf:writeInt16(-12345, 2)
    print(buf:readInt16(0, 2)) -- -12345

    buf:writeUInt32(4294967295, 4) -- Max uint32
    print(buf:readUInt32(0, 2)) -- 4294967295

    buf:writeInt64(-9223372036854775808, 8) -- Min int64
    print(buf:readInt64(0, 8)) -- -9223372036854775808
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

    if volta.input.isKeyDown("w") then
        rectangleY = rectangleY - speed * dt
    end
    if volta.input.isKeyDown("s") then
        rectangleY = rectangleY + speed * dt
    end
    if volta.input.isKeyDown("a") then
        rectangleX = rectangleX - speed * dt
    end
    if volta.input.isKeyDown("d") then
        rectangleX = rectangleX + speed * dt
    end

    if t <= 1 then
        x = math.lerp(starting, destination, t)
        t = t + 0.01
    end

    volta.graphics.drawImage("tree.png", 100, 100, 200, 200)

    volta.graphics.setColor(1, 0, 0)
    volta.graphics.circle(true, x, y, 50)
    volta.graphics.setColor(0.5, 1, 0)
    volta.graphics.rectangle(true, rectangleX, rectangleY, 50, 50)
end

volta.input.keyPressed("up", function()
    volta.audio.setGlobalVolume(volta.audio.getGlobalVolume() + 0.1)
end)

volta.input.keyPressed("down", function()
    volta.audio.setGlobalVolume(volta.audio.getGlobalVolume() - 0.1)
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