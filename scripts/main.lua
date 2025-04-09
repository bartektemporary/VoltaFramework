-- Define initial cube properties
local cube_position = volta.vector3.new(0, 0, 0)  -- Center of the 3D space
local cube_size = volta.vector3.new(2, 2, 2)      -- 2x2x2 cube
local cube_rotation = volta.vector3.new(0, 0, 0)  -- Initial rotation
local rotation_speed = 45                         -- Degrees per second

-- Set initial color (white)
volta.graphics.setColor(1, 1, 1)

-- Set up a 3D camera
local camera = volta.camera3d.new(
    0, 1, 5,    -- Initial position (x, y, z) - slightly above and back from the cube
    0, 0, 0     -- Initial rotation (x, y, z) in degrees
)
camera:setFOV(75)      -- Field of view (degrees)
camera:setNearPlane(0.1) -- Near clipping plane
camera:setFarPlane(100)  -- Far clipping plane
volta.setCamera3D(camera)

-- Movement speed
local move_speed = 5    -- Units per second
local rotate_speed = 90 -- Degrees per second

-- Update function to be called each frame
function volta.update(dt)
    -- Rotate the cube
    local rotation_increment = volta.vector3.new(
        rotation_speed * dt,
        rotation_speed * dt,
        rotation_speed * dt
    )
    cube_rotation = cube_rotation:add(rotation_increment)
    
    -- Keep rotations within 0-360 degrees
    local function mod360(v)
        return volta.vector3.new(
            v.x % 360,
            v.y % 360,
            v.z % 360
        )
    end
    cube_rotation = mod360(cube_rotation)

    -- Camera movement
    local cam_pos = camera:getPosition()
    local cam_rot = camera:getRotation()
    local move_dir = volta.vector3.new(0, 0, 0)

    -- WASD movement (forward, backward, left, right)
    if volta.input.isKeyDown("W") then
        -- Move forward along camera's direction (z-axis in local space)
        move_dir.z = move_dir.z - move_speed * dt
    end
    if volta.input.isKeyDown("S") then
        -- Move backward
        move_dir.z = move_dir.z + move_speed * dt
    end
    if volta.input.isKeyDown("A") then
        -- Strafe left
        move_dir.x = move_dir.x - move_speed * dt
    end
    if volta.input.isKeyDown("D") then
        -- Strafe right
        move_dir.x = move_dir.x + move_speed * dt
    end
    if volta.input.isKeyDown("SPACE") then
        -- Move up
        move_dir.y = move_dir.y + move_speed * dt
    end
    if volta.input.isKeyDown("LEFT_CONTROL") then
        -- Move down
        move_dir.y = move_dir.y - move_speed * dt
    end

    -- Apply rotation to movement direction based on camera yaw (y-axis rotation)
    local yaw_rad = cam_rot.y * math.pi / 180
    local forward_x = -math.sin(yaw_rad) * move_dir.z + math.cos(yaw_rad) * move_dir.x
    local forward_z = -math.cos(yaw_rad) * move_dir.z - math.sin(yaw_rad) * move_dir.x
    move_dir.x = forward_x
    move_dir.z = forward_z

    -- Update camera position
    if move_dir:magnitude() > 0 then
        camera:move(move_dir)
    end

    -- Camera rotation (basic yaw and pitch with Q/E for roll)
    local rot_change = volta.vector3.new(0, 0, 0)
    if volta.input.isKeyDown("LEFT") then
        rot_change.y = rot_change.y + rotate_speed * dt -- Yaw left
    end
    if volta.input.isKeyDown("RIGHT") then
        rot_change.y = rot_change.y - rotate_speed * dt -- Yaw right
    end
    if volta.input.isKeyDown("UP") then
        rot_change.x = rot_change.x - rotate_speed * dt -- Pitch up
    end
    if volta.input.isKeyDown("DOWN") then
        rot_change.x = rot_change.x + rotate_speed * dt -- Pitch down
    end
    if volta.input.isKeyDown("Q") then
        rot_change.z = rot_change.z + rotate_speed * dt -- Roll left
    end
    if volta.input.isKeyDown("E") then
        rot_change.z = rot_change.z - rotate_speed * dt -- Roll right
    end

    -- Apply rotation changes
    if rot_change:magnitude() > 0 then
        camera:rotateBy(rot_change)
    end

    -- Draw the cube with the new rotation
    volta.graphics.drawCube(cube_position, cube_size, cube_rotation)
end