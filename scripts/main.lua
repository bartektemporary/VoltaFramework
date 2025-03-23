-- Global variables
local player = {
    pos = volta.vector2.new(0, 0), -- Using Vector2 for position
    size = volta.vector2.new(50, 50) -- Using Vector2 for size
}
local camera = nil
local speed = 200 -- Movement speed in world units per second
local emitter

-- Initialize the game
function volta.init()
    -- Create a camera at the origin with default zoom and rotation
    camera = volta.camera2d.new(volta.vector2.new(0, 0), 1, 0)
    volta.setCamera2D(camera)
    volta.window.setState("borderlessMaximized")
    
    emitter = volta.particleEmitter.new(
        volta.vector2.new(0, 0), -- Position
        22.0,                    -- Particle life
        100.0,                  -- Speed
        90.0,                   -- Spread (degrees)
        volta.vector2.new(1, 0),-- Direction (up)
        "cone"                  -- Shape
    )

    -- Set initial background color
    volta.graphics.setColor(0.2, 0.2, 0.2) -- Dark gray background
end

-- Update function called every frame
function volta.update(dt)
    -- Handle player movement
    local move = volta.vector2.new(0, 0)
    if volta.input.isKeyDown("W") then
        move = volta.vector2.new(move.x, move.y - 1)
    end
    if volta.input.isKeyDown("S") then
        move = volta.vector2.new(move.x, move.y + 1)
    end
    if volta.input.isKeyDown("A") then
        move = volta.vector2.new(move.x - 1, move.y)
    end
    if volta.input.isKeyDown("D") then
        move = volta.vector2.new(move.x + 1, move.y)
    end

    -- Normalize movement vector if moving diagonally
    local mag = move:magnitude()
    if mag > 0 then
        local normalizedMove = move:normalize()
        local movement = normalizedMove:multiply(speed * dt) -- Multiply Vector2 by scalar
        player.pos = player.pos:add(movement) -- Add Vector2 to Vector2
    end

    -- Make the camera follow the player
    camera:setPosition(player.pos)

    -- Update and render particles
    emitter:setPosition(volta.vector2.new(2000, 2000)) -- Beyond (960, 540)
    emitter:emit(10) -- Emit 5 particles per frame
    emitter:render()

    -- Draw screen-space UI elements
    volta.graphics.setPositionMode("screen")
    local uiPos1 = volta.vector2.new(10, 10)
    local uiSize1 = volta.vector2.new(100, 20)
    volta.graphics.setColor(1, 1, 1) -- White
    volta.graphics.rectangle(true, uiPos1, uiSize1, 0) -- Top-left screen-space rectangle
    local uiPos2 = volta.vector2.new(50, 50)
    volta.graphics.setColor(0, 0, 1) -- Blue
    volta.graphics.circle(true, uiPos2, 15) -- Screen-space circle

    -- Draw world-space objects
    volta.graphics.setPositionMode("world")
    
    -- Player rectangle (red)
    volta.graphics.setColor(1, 0, 0)
    volta.graphics.rectangle(true, player.pos, player.size, 0)

    -- Scattered shapes in world space using Vector2
    local greenPos = volta.vector2.new(100, 100)
    local greenSize = volta.vector2.new(60, 60)
    volta.graphics.setColor(0, 1, 0) -- Green
    volta.graphics.rectangle(true, greenPos, greenSize, 45) -- Rotated square

    local yellowPos = volta.vector2.new(-50, -50)
    volta.graphics.setColor(1, 1, 0) -- Yellow
    volta.graphics.circle(true, yellowPos, 30) -- Circle to the left

    local cyanPos = volta.vector2.new(-200, 200)
    local cyanSize = volta.vector2.new(80, 80)
    volta.graphics.setColor(0, 1, 1) -- Cyan
    volta.graphics.rectangle(false, cyanPos, cyanSize, 0) -- Outline rectangle

    local magentaPos = volta.vector2.new(300, -100)
    volta.graphics.setColor(1, 0, 1) -- Magenta
    volta.graphics.circle(false, magentaPos, 40) -- Outline circle

    -- Draw some lines in world space using Vector2
    local lineStart1 = volta.vector2.new(-100, -100)
    local lineEnd1 = volta.vector2.new(100, 100)
    local lineStart2 = volta.vector2.new(0, -150)
    local lineEnd2 = volta.vector2.new(0, 150)
    volta.graphics.setColor(0.5, 0.5, 0.5) -- Gray
    volta.graphics.setPositionMode("world")
    volta.graphics.drawLine(lineStart1, lineEnd1, 1) -- Diagonal line
    volta.graphics.drawLine(lineStart2, lineEnd2, 1) -- Vertical line

    -- Yellow image (tree.png)
    volta.graphics.setColor(1, 1, 0)
    volta.graphics.drawImage("tree.png", volta.vector2.new(-50, -50), volta.vector2.new(100, 100), 0)
        
    -- Blue text with Minecraft.ttf
    volta.graphics.setColor(0, 0, 1)
    volta.graphics.drawText("Minecraft Test", volta.vector2.new(100, 100), 1)
end