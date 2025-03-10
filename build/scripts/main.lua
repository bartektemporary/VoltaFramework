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

function drawRotatedRectangle(centerX, centerY, width, height, angle, fill)
    -- Create the center position as a Vector2
    local center = volta.vector2.new(centerX, centerY)

    -- Calculate the four corners of the rectangle (before rotation)
    -- Corners are relative to the center
    local halfWidth = width / 2
    local halfHeight = height / 2

    -- Define the corners relative to (0,0) before translation to center
    local corners = {
        volta.vector2.new(-halfWidth, -halfHeight), -- Bottom-left
        volta.vector2.new(halfWidth, -halfHeight),  -- Bottom-right
        volta.vector2.new(halfWidth, halfHeight),   -- Top-right
        volta.vector2.new(-halfWidth, halfHeight)   -- Top-left
    }

    -- Rotate each corner around (0,0) and then translate to the center
    local rotatedCorners = {}
    for i, corner in ipairs(corners) do
        -- Rotate the corner around (0,0)
        local rotated = corner:rotate(angle)
        -- Translate to the center position
        rotatedCorners[i] = volta.vector2.new(
            center.x + rotated.x,
            center.y + rotated.y
        )
    end

    if fill then
        -- For a filled rectangle, we can approximate by drawing triangles
        -- Use two triangles to form the quad (since l_rectangle doesn't support rotation)
        -- However, since volta.graphics doesn't have a triangle function,
        -- we'll simulate with lines for now or assume a future C++ extension
        -- For simplicity, we'll draw as lines and suggest a C++ helper below
        drawRotatedOutline(rotatedCorners)
    else
        -- Draw the outline by connecting the rotated corners with lines
        drawRotatedOutline(rotatedCorners)
    end
end

-- Helper function to draw the outline of the rotated rectangle
function drawRotatedOutline(corners)
    -- Draw lines between consecutive corners (and close the loop)
    for i = 1, #corners do
        local start = corners[i]
        local next = corners[i % #corners + 1] -- Wrap around to the first corner
        volta.graphics.drawLine(start, next)
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

        -- Set the color (e.g., red)
        volta.graphics.setColor(1, 0, 0)

        -- Draw a rotated rectangle
        -- Center at (200, 200), size 100x50, rotated 45 degrees, not filled
        drawRotatedRectangle(200, 200, 100, 50, 45, false)
    
        -- Draw another rotated rectangle
        -- Center at (300, 300), size 80x120, rotated 30 degrees, filled (approximated as outline for now)
        volta.graphics.setColor(0, 1, 0)
        drawRotatedRectangle(300, 300, 80, 120, 30, true)
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