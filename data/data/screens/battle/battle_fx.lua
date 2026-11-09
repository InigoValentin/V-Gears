if UiContainer == nil then UiContainer = {} end

--- Battle visual effects.
--
-- Simple generic effects: bursts of coloured particles by element, screen flashes and floating
-- numbers. Everything is made with the sprites and text areas of the BattleFx screen, moved from
-- here every frame. Positions of the scene are projected to the UI with camera_manager.
UiContainer.BattleFx = {

    --- Size of the pools of the screen. Must match battle_fx.xml.
    PARTICLES = 60,
    NUMBERS = 8,

    --- Height, in scene units, above the position of a combatant where effects are shown.
    BODY_HEIGHT = 1.6,

    --- Effects of each element: colour, particle count, speed (UI units per second), gravity (UI
    -- units per second squared, negative makes them rise), size, life (seconds) and the flash
    -- (colour and strength) of the spells of the element. "converge" makes the particles come to
    -- the center instead of leaving it.
    ELEMENTS = {
        [0] = {colour = {1, 0.55, 0.1}, count = 24, speed = 90, gravity = -140, size = 10, life = 0.9, flash = {1, 0.5, 0.1, 0.3}},
        [1] = {colour = {0.6, 0.9, 1}, count = 18, speed = 110, gravity = 180, size = 8, life = 0.8, flash = {0.6, 0.9, 1, 0.3}},
        [2] = {colour = {1, 1, 0.4}, count = 16, speed = 260, gravity = 0, size = 6, life = 0.35, flash = {1, 1, 0.6, 0.55}},
        [3] = {colour = {0.65, 0.45, 0.25}, count = 18, speed = 140, gravity = 320, size = 9, life = 0.9, up = true},
        [4] = {colour = {0.5, 0.9, 0.3}, count = 16, speed = 50, gravity = -70, size = 9, life = 1.1, flash = {0.5, 0.9, 0.3, 0.2}},
        [5] = {colour = {0.6, 0.3, 0.9}, count = 20, speed = 120, gravity = 0, size = 8, life = 0.8, converge = true, flash = {0.3, 0.1, 0.5, 0.35}},
        [6] = {colour = {0.3, 0.6, 1}, count = 20, speed = 90, gravity = 240, size = 8, life = 0.9, flash = {0.3, 0.6, 1, 0.25}},
        [7] = {colour = {0.8, 1, 0.9}, count = 16, speed = 240, gravity = 0, size = 5, life = 0.5, flash = {0.8, 1, 0.9, 0.2}},
        [8] = {colour = {1, 1, 0.85}, count = 22, speed = 70, gravity = -100, size = 9, life = 1.0, flash = {1, 1, 0.9, 0.45}},
    },

    --- Effect of healing.
    HEAL = {colour = {0.4, 1, 0.55}, count = 16, speed = 45, gravity = -80, size = 9, life = 1.0, flash = {0.4, 1, 0.55, 0.2}},

    --- Effect of attacks without an element.
    PHYSICAL = {colour = {1, 1, 1}, count = 9, speed = 150, gravity = 260, size = 6, life = 0.35},

    --- Colours of the floating numbers.
    COLOUR_DAMAGE = {1, 1, 1},
    COLOUR_HEAL = {0.4, 1, 0.5},
    COLOUR_MP = {0.5, 0.7, 1},
    COLOUR_MISS = {0.7, 0.7, 0.7},

    --- Active particles and numbers.
    particles = {},
    numbers = {},

    --- Active screen flash.
    flash_state = nil,

    --- Game time of the last update.
    last_time = 0,

    --- Shows the effects screen and clears everything.
    show = function(self)
        self:clear()
        self.last_time = timer:get_game_time_total()
        ui_manager:get_widget("BattleFx"):set_visible(true)
    end,

    --- Hides the effects screen.
    hide = function(self)
        self:clear()
        ui_manager:get_widget("BattleFx"):set_visible(false)
    end,

    --- Removes every effect.
    clear = function(self)
        for _, particle in pairs(self.particles) do ui_manager:get_widget("BattleFx.P" .. tostring(particle.index)):set_visible(false) end
        for _, number in pairs(self.numbers) do ui_manager:get_widget("BattleFx.N" .. tostring(number.index)):set_visible(false) end
        ui_manager:get_widget("BattleFx.Flash"):set_visible(false)
        self.particles = {}
        self.numbers = {}
        self.flash_state = nil
    end,

    --- Gets the position of a combatant in the UI.
    --
    -- @param side Battle.TargetGroup.PARTY or ENEMIES.
    -- @param index Index of the combatant.
    -- @param height Height above the combatant, in scene units.
    -- @return X and Y in UI units, nil if the combatant has no entity.
    get_position = function(self, side, index, height)
        local entity = UiContainer.BattleUi:get_entity(side, index)
        if entity == nil then return nil end
        local x, y, z = entity:get_position()
        local ux, uy = camera_manager:project_to_ui(x, y, z + height)
        return ux, uy
    end,

    --- Finds a free particle of the pool.
    free_particle = function(self)
        for i = 1, self.PARTICLES do
            if self.particles[i] == nil then return i end
        end
        return nil
    end,

    --- Creates a particle.
    --
    -- @param p Table with x, y, vx, vy (per second), gravity, life, size, colour and alpha.
    spawn = function(self, p)
        local i = self:free_particle()
        if i == nil then do return end end
        p.index = i
        p.age = 0
        self.particles[i] = p
        local widget = ui_manager:get_widget("BattleFx.P" .. tostring(i))
        widget:set_colour(p.colour[1], p.colour[2], p.colour[3])
        widget:set_alpha(p.alpha or 1)
        widget:set_width(0, p.size)
        widget:set_height(0, p.size)
        widget:set_x(0, p.x - p.size / 2)
        widget:set_y(0, p.y - p.size / 2)
        widget:set_visible(true)
    end,

    --- Creates a burst of particles from a preset around a position.
    burst = function(self, preset, x, y)
        for _ = 1, preset.count do
            local angle = math.random() * 2 * math.pi
            local speed = preset.speed * (0.4 + 0.6 * math.random())
            local vx, vy = math.cos(angle) * speed, math.sin(angle) * speed
            local px, py = x, y
            if preset.converge then
                -- Start on a ring and move to the center.
                px, py = x - vx * 0.5, y - vy * 0.5
                vx, vy = vx, vy
            end
            if preset.up then vy = -math.abs(vy) * 1.3 end
            -- Slightly different tones.
            local shade = 0.8 + 0.2 * math.random()
            self:spawn({
                x = px, y = py, vx = vx, vy = vy, gravity = preset.gravity,
                life = preset.life * (0.7 + 0.5 * math.random()), size = preset.size * (0.6 + 0.6 * math.random()),
                colour = {preset.colour[1] * shade, preset.colour[2] * shade, preset.colour[3] * shade},
            })
        end
    end,

    --- Shows a screen flash that fades out.
    --
    -- @param colour Table with r, g, b.
    -- @param strength Starting alpha.
    -- @param life Seconds it lasts.
    flash = function(self, colour, strength, life)
        self.flash_state = {colour = colour, strength = strength, life = life or 0.35, age = 0}
        local widget = ui_manager:get_widget("BattleFx.Flash")
        widget:set_colour(colour[1], colour[2], colour[3])
        widget:set_alpha(strength)
        widget:set_visible(true)
    end,

    --- Gets the effect preset of an attack, for the element it has or if it heals.
    get_preset = function(self, attack, heals)
        if heals then return self.HEAL end
        if attack ~= nil and attack.elements ~= nil and #attack.elements > 0 then
            return self.ELEMENTS[attack.elements[1]] or self.PHYSICAL
        end
        return self.PHYSICAL
    end,

    --- Effect of casting a spell: sparks around the caster.
    cast = function(self, attack, side, index)
        -- Only magic is cast. Other attacks just hit.
        if attack == nil or attack.id == nil or attack.id < 0 or (attack.damage.formula or 0) ~= 2 then do return end end
        local x, y = self:get_position(side, index, self.BODY_HEIGHT)
        if x == nil then do return end end
        local preset = self:get_preset(attack, (attack.damage.modifier or 0) == 5 and (attack.damage.formula or 0) == 2)
        for _ = 1, 8 do
            local angle = math.random() * 2 * math.pi
            self:spawn({
                x = x + math.cos(angle) * 30, y = y + math.sin(angle) * 30,
                vx = -math.cos(angle) * 40, vy = -math.sin(angle) * 40 - 30, gravity = 0,
                life = 0.8, size = 7, colour = preset.colour,
            })
        end
    end,

    --- Effect of an attack that hits a target: particles and, for spells, a screen flash.
    --
    -- @param attack Attack data.
    -- @param result The result of the attack, as in BattleUi.calculate_attack.
    hit = function(self, attack, side, index, result)
        local x, y = self:get_position(side, index, self.BODY_HEIGHT)
        if x == nil then do return end end
        local preset = self:get_preset(attack, result.amount < 0)
        self:burst(preset, x, y)
        if preset.flash ~= nil and attack.id ~= nil and attack.id >= 0 then
            self:flash({preset.flash[1], preset.flash[2], preset.flash[3]}, preset.flash[4], 0.4)
        end
    end,

    --- Shows a floating text over a combatant.
    --
    -- @param text The text to show.
    -- @param colour Table with r, g, b.
    text = function(self, side, index, text, colour)
        local x, y = self:get_position(side, index, self.BODY_HEIGHT + 0.6)
        if x == nil then do return end end
        local slot = nil
        for i = 1, self.NUMBERS do
            if self.numbers[i] == nil then slot = i break end
        end
        -- If all are used, take the one that has been there the longest.
        if slot == nil then
            local oldest = 1
            for i = 2, self.NUMBERS do
                if self.numbers[i].age > self.numbers[oldest].age then oldest = i end
            end
            slot = oldest
        end
        -- Spread the numbers of the same target a bit so they don't pile up.
        self.numbers[slot] = {
            index = slot, x = x + (math.random() - 0.5) * 40 - 7 * #text / 2, y = y - 10 * math.random(),
            age = 0, life = 1.1, text = text, colour = colour,
        }
        local number = self.numbers[slot]
        local widget = ui_manager:get_widget("BattleFx.N" .. tostring(slot))
        -- The colour is applied when the text is set.
        widget:set_colour(colour[1], colour[2], colour[3])
        widget:set_alpha(1)
        widget:set_text(text)
        widget:set_x(0, number.x)
        widget:set_y(0, number.y)
        widget:set_visible(true)
    end,

    --- Shows the HP or MP lost or recovered by a combatant.
    --
    -- @param amount Positive for damage, negative for recovery.
    -- @param mp True if it's MP.
    number = function(self, side, index, amount, mp)
        local colour = self.COLOUR_DAMAGE
        if amount < 0 then colour = self.COLOUR_HEAL end
        if mp then colour = self.COLOUR_MP end
        self:text(side, index, tostring(math.abs(amount)), colour)
    end,

    --- Advances the effects.
    --
    -- @param now Game time.
    update = function(self, now)
        local dt = math.max(0, math.min(0.1, now - self.last_time))
        self.last_time = now
        for i, p in pairs(self.particles) do
            p.age = p.age + dt
            local widget = ui_manager:get_widget("BattleFx.P" .. tostring(i))
            if p.age >= p.life then
                widget:set_visible(false)
                self.particles[i] = nil
            else
                p.x = p.x + p.vx * dt
                p.y = p.y + p.vy * dt
                p.vy = p.vy + p.gravity * dt
                local fade = 1 - p.age / p.life
                widget:set_alpha((p.alpha or 1) * fade)
                widget:set_x(0, p.x - p.size / 2)
                widget:set_y(0, p.y - p.size / 2)
            end
        end
        for i, n in pairs(self.numbers) do
            n.age = n.age + dt
            local widget = ui_manager:get_widget("BattleFx.N" .. tostring(i))
            if n.age >= n.life then
                widget:set_visible(false)
                self.numbers[i] = nil
            else
                -- Rise, and fade out in the last part. Text colour is baked when it's set, so the
                -- text is set again with the new alpha.
                local alpha = math.min(1, (n.life - n.age) / (n.life * 0.4))
                widget:set_colour(n.colour[1], n.colour[2], n.colour[3])
                widget:set_alpha(alpha)
                widget:set_text(n.text)
                widget:set_x(0, n.x)
                widget:set_y(0, n.y - 45 * (n.age / n.life))
            end
        end
        local f = self.flash_state
        if f ~= nil then
            f.age = f.age + dt
            local widget = ui_manager:get_widget("BattleFx.Flash")
            if f.age >= f.life then
                widget:set_visible(false)
                self.flash_state = nil
            else
                widget:set_alpha(f.strength * (1 - f.age / f.life))
            end
        end
    end,
}
