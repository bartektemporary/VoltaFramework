function volta.init()
    -- Define start and end positions
    local startPos = volta.vector2.new(100, 100)
    local endPos = volta.vector2.new(700, 300)
    
    -- List of all easing styles
    local easingStyles = {
        "linear",
        "sine",
        "quad",
        "cubic",
        "quart",
        "quint",
        "exponential",
        "circular",
        "back",
        "bounce",
        "elastic"
    }
    
    -- List of all directions
    local directions = {
        "in",
        "out",
        "inout"
    }
    
    -- Time points to test
    local tValues = {0, 0.25, 0.5, 0.75, 1}
    
    -- Table to store results
    local results = {}
    
    -- Calculate tweens for each combination
    for _, style in ipairs(easingStyles) do
        results[style] = {}
        for _, direction in ipairs(directions) do
            results[style][direction] = {}
            for _, t in ipairs(tValues) do
                local tweened = startPos:tween(endPos, t, direction, style)
                results[style][direction][t] = {x = tweened.x, y = tweened.y}
                -- Print results immediately for verification
                print(string.format("Style %s direction %s t=%.2f x: %.2f y: %.2f",
                    style, direction, t, tweened.x, tweened.y))
            end
        end
    end
    
    return results
end

-- Call init directly (no update loop needed)
volta.init()