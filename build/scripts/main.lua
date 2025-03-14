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

local particleEmitter

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

    particleEmitter = volta.particleEmitter.new(defaultPosition, 1, 150, 360)
    
    volta.onEvent("gameStart", function()
        print("moved")
    end)
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

    if moved then
        volta.triggerEvent("gameStart")
        particleEmitter:emit(3)
    end

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
    particleEmitter:render()

    volta.graphics.setColor(1, 1, 1)
    volta.graphics.drawImage("tree.png", treePos, vector2.new(200, 200))

    volta.graphics.setColor(0.5, 1, 0)
    volta.graphics.rectangle(true, defaultPosition, vector2.new(50, 50))

    volta.graphics.drawLine(vector2.new(100, 100), vector2.new(500, 500), 10)

    --works
    volta.graphics.setColor(1, 0, 0)
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

volta.input.keyPressed("k", function()
    particleEmitter:setSpeed(particleEmitter:getSpeed() + 10)
end)

volta.input.keyPressed("l", function()
    particleEmitter:setSpeed(particleEmitter:getSpeed() - 10)
end)