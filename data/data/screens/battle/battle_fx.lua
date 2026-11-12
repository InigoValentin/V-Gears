if UiContainer == nil then UiContainer = {} end

--- Battle visual effects.
--
-- 3D effects and sprites made from the assets of the original game, floating
-- numbers. Everything is made with the sprites and text areas of the BattleFx screen, moved from
-- here every frame. Positions of the scene are projected to the UI with camera_manager.
UiContainer.BattleFx = {

    --- Size of the pools of the screen. Must match battle_fx.xml.
    SPRITES = 40,
    NUMBERS = 8,

    --- Height, in scene units, above the position of a combatant where effects are shown.
    BODY_HEIGHT = 1.6,

    --- Colours of the floating numbers.
    COLOUR_DAMAGE = {1, 1, 1},
    COLOUR_HEAL = {0.4, 1, 0.5},
    COLOUR_MP = {0.5, 0.7, 1},
    COLOUR_MISS = {0.7, 0.7, 0.7},

    --- Active numbers.
    numbers = {},

    --- Active screen flash.
    flash_state = nil,

    --- Game time of the last update.
    last_time = 0,

    --- Sprite sheets of the original effects: the image (in images/), its size in pixels, the
    -- frame list of the sprite file (Game.EffectSprites, from gamedata/effect_sprites.lua) and
    -- the frames to play, in order.
    SHEETS = {
        -- Small star flare that grows and fades. 4 x 2 frames of 32 pixels.
        sparkle = {image = "battle_fx/common_iyashi01.png", width = 128, height = 128,
                   sprites = "common_iyashi2", frames = {1, 2, 4, 5, 6, 7, 8, 9}},
        -- Big star flare. Frames of 64 pixels.
        flare = {image = "battle_fx/common_iyashi00.png", width = 256, height = 128,
                 sprites = "common_iyashi", frames = {1, 2, 3, 4, 5, 6}},
    },

    --- Active animated sprites, by pool index.
    sprites = {},

    --- Creates an animated sprite, that plays the frames of a sheet over its life.
    --
    -- @param p Table with sheet (name in SHEETS), x, y (UI), size (UI units), life, delay, vx, vy
    -- (UI units per second) and tint (r, g, b).
    spawn_sprite = function(self, p)
        local sheet = self.SHEETS[p.sheet]
        local list = Game.EffectSprites and Game.EffectSprites[sheet.sprites]
        if list == nil then do return end end
        local slot = nil
        for i = 1, self.SPRITES do
            if self.sprites[i] == nil then slot = i break end
        end
        if slot == nil then do return end end
        p.slot = slot
        p.sheet_data = sheet
        p.list = list
        p.age = 0
        p.delay = p.delay or 0
        self.sprites[slot] = p
        local widget = ui_manager:get_widget("BattleFx.S" .. tostring(slot))
        widget:set_visible(false)
    end,

    --- Advances the animated sprites.
    update_sprites = function(self, dt)
        for slot, p in pairs(self.sprites) do
            p.age = p.age + dt
            local widget = ui_manager:get_widget("BattleFx.S" .. tostring(slot))
            local u = (p.age - p.delay) / p.life
            if u >= 1 then
                widget:set_visible(false)
                self.sprites[slot] = nil
            elseif u >= 0 then
                local sheet = p.sheet_data
                if not p.started then
                    p.started = true
                    widget:set_image(sheet.image)
                    widget:set_additive(true)
                    local tint = p.tint or {1, 1, 1}
                    widget:set_colour(tint[1], tint[2], tint[3])
                    widget:set_width(0, p.size)
                    widget:set_height(0, p.size)
                    widget:set_visible(true)
                end
                p.x = p.x + (p.vx or 0) * dt
                p.y = p.y + (p.vy or 0) * dt
                local frame = p.list[sheet.frames[math.min(#sheet.frames, math.floor(u * #sheet.frames) + 1)]]
                widget:set_image_region(
                  frame.u / sheet.width, frame.v / sheet.height,
                  (frame.u + frame.w) / sheet.width, (frame.v + frame.h) / sheet.height
                )
                widget:set_x(0, p.x - p.size / 2)
                widget:set_y(0, p.y - p.size / 2)
                widget:set_alpha(1)
            end
        end
    end,

    --- Removes the animated sprites.
    clear_sprites = function(self)
        for slot, _ in pairs(self.sprites) do ui_manager:get_widget("BattleFx.S" .. tostring(slot)):set_visible(false) end
        self.sprites = {}
    end,

    --- Healing: star flares that pop up around the target and rise.
    heal_sparkles = function(self, x, y, power)
        -- A big flare on the target.
        for i = 1, 2 do
            self:spawn_sprite({sheet = "flare", x = x, y = y - 15, size = 55 + 10 * power, life = 0.7, delay = 0.25 * (i - 1), vy = -25})
        end
        -- Small sparkles around, at different times.
        for i = 1, 12 + 4 * power do
            self:spawn_sprite({
                sheet = "sparkle", x = x + (math.random() - 0.5) * 90, y = y + 20 - math.random() * 70,
                size = 10 + 8 * math.random(), life = 0.5 + 0.3 * math.random(), delay = math.random() * 0.9,
                vy = -30 - 30 * math.random(),
            })
        end
    end,

    --- Size of the effect models relative to the original ones. Must match
    -- BattleManager::ATTACK_SCALE.
    MODEL_UNIT = 0.0032,

    --- Active 3D pieces of the effects, made from the meshes extracted from magic.lgp.
    pieces = {},

    --- False if the meshes of the effects aren't installed, so they aren't asked for again.
    models_available = true,

    --- Creates a 3D piece of an effect.
    --
    -- Meshes are static parts of the original effects. Each effect animates them from the update
    -- function: scale, position and opacity. The mesh stands up with rx = 90, as the original
    -- ones are lying along the Z axis of the model.
    --
    -- @param p Table with mesh (name from gamedata/effects.lua), x, y, z (scene position), life
    -- (seconds), delay (seconds before it appears), blend (Entity.BLEND_*, additive by default),
    -- rx, ry, rz (rotation of the mesh), and update, a function(piece, u, entity) called every
    -- frame with u from 0 to 1 over the life of the piece.
    spawn_piece = function(self, p)
        if not self.models_available then do return end end
        p.age = 0
        p.delay = p.delay or 0
        p.life = p.life or 1
        self.pieces[#self.pieces + 1] = p
    end,

    --- Creates the entity of a piece, when its delay ends.
    start_piece = function(self, p)
        if not self.models_available then return false end
        local name = battle_manager:add_effect_model(p.mesh, p.x, p.y, p.z, 1)
        if name == nil or name == "" then
            -- Not installed. Don't try again with any piece.
            self.models_available = false
            return false
        end
        p.entity_name = name
        p.entity = entity_manager:get_entity(name)
        p.entity:set_blend(p.blend or Entity.BLEND_ADDITIVE)
        p.entity:set_model_rotation(p.rx or 0, p.ry or 0, p.rz or 0)
        p.entity:set_alpha(0)
        return true
    end,

    --- Advances the 3D pieces and removes the finished ones.
    update_pieces = function(self, dt)
        local alive = {}
        for _, p in ipairs(self.pieces) do
            p.age = p.age + dt
            local keep = true
            if p.age >= p.delay then
                if p.entity == nil then keep = self:start_piece(p) end
                if keep then
                    local u = math.min(1, (p.age - p.delay) / p.life)
                    local ok = pcall(p.update, p, u, p.entity)
                    if not ok or u >= 1 then keep = false end
                end
            end
            if keep then alive[#alive + 1] = p
            elseif p.entity_name ~= nil then battle_manager:remove_effect_model(p.entity_name)
            end
        end
        self.pieces = alive
    end,

    --- Removes the 3D pieces.
    clear_pieces = function(self)
        for _, p in ipairs(self.pieces) do
            if p.entity_name ~= nil then battle_manager:remove_effect_model(p.entity_name) end
        end
        self.pieces = {}
    end,

    --- Fades in quickly, stays and fades out, for the opacity of a piece. u goes from 0 to 1.
    fade = function(u, fade_in, fade_out)
        if u < fade_in then return u / fade_in end
        if u > 1 - fade_out then return (1 - u) / fade_out end
        return 1
    end,

    --- Effects with meshes by the element of the attack. Each one gets the position of the target
    -- in the scene and the strength of the spell (1 for the basic ones).
    MODEL_EFFECTS = {
        -- Fire: columns of flames that grow around the target.
        [0] = function(self, x, y, z, power)
            local count = 5 + math.floor(power * 2)
            for i = 1, count do
                local angle = (i / count) * 2 * math.pi + math.random() * 0.5
                local radius = (i == 1) and 0 or (0.5 + 0.5 * math.random()) * power
                local width = 11 + 5 * power
                local height = 1 + 0.5 * power
                self:spawn_piece({
                    mesh = "magic_faira_" .. tostring((i - 1) % 8 + 1),
                    x = x + math.cos(angle) * radius, y = y + math.sin(angle) * radius, z = z,
                    rx = 90, life = 0.9 + 0.3 * math.random(), delay = 0.05 * i,
                    update = function(piece, u, entity)
                        local grow = math.min(1, u / 0.35)
                        local unit = self.MODEL_UNIT
                        entity:set_scale(
                          width * unit * (0.5 + 0.5 * grow), width * unit * (0.4 + 1.2 * grow) * height,
                          width * unit * (0.5 + 0.5 * grow)
                        )
                        entity:set_alpha(self.fade(u, 0.2, 0.5))
                    end,
                })
            end
        end,

        -- Ice: shards that rise from the ground.
        [1] = function(self, x, y, z, power)
            local count = 4 + math.floor(power * 2)
            for i = 1, count do
                local angle = (i / count) * 2 * math.pi
                local radius = (i == 1) and 0 or (0.3 + 0.5 * math.random()) * power
                local size = 12 + 6 * power * math.random()
                self:spawn_piece({
                    mesh = "magic_freeze_ice_1",
                    x = x + math.cos(angle) * radius, y = y + math.sin(angle) * radius, z = z + 0.2,
                    rx = 0, life = 1.1, delay = 0.06 * i, blend = Entity.BLEND_ADDITIVE,
                    update = function(piece, u, entity)
                        local grow = math.min(1, u / 0.25)
                        local unit = self.MODEL_UNIT
                        entity:set_scale(size * unit * grow, size * unit * grow * 1.6, size * unit * grow)
                        entity:set_alpha(self.fade(u, 0.15, 0.45))
                    end,
                })
            end
        end,

        -- Bolt: a funnel of light that falls on the target.
        [2] = function(self, x, y, z, power)
            local width = 14 + 6 * power
            for i = 1, 3 do
                self:spawn_piece({
                    mesh = "magic_thunder_thund_3",
                    x = x, y = y, z = z + 2.6,
                    rx = 90, life = 0.35, delay = 0.12 * (i - 1),
                    update = function(piece, u, entity)
                        local unit = self.MODEL_UNIT
                        local drop = 1 - math.min(1, u / 0.5)
                        entity:set_position(x, y, z + 0.2 + 2.4 * drop)
                        entity:set_scale(width * unit * (0.6 + 0.8 * u), width * unit * 4, width * unit * (0.6 + 0.8 * u))
                        entity:set_alpha(self.fade(u, 0.1, 0.5))
                    end,
                })
            end
        end,

    },

    --- Spawns a sphere made of two copies of the half-sphere mesh of the barriers, that grows
    -- around the target, holds and fades out.
    --
    -- @param mesh Mesh name (radius 10 in the original units).
    -- @param radius Final radius in scene units.
    barrier_sphere = function(self, mesh, x, y, z, radius, life)
        for _, rx in ipairs({90, -90}) do
            self:spawn_piece({
                mesh = mesh, x = x, y = y, z = z + 0.9, rx = rx, life = life,
                update = function(piece, u, entity)
                    local s = radius / 10 * (0.7 + 0.3 * math.min(1, u / 0.3))
                    entity:set_scale(s, s, s)
                    entity:set_alpha(self.fade(u, 0.15, 0.5) * 0.8)
                end,
            })
        end
    end,

    --- Spawns a flat ring on the ground that expands and fades out.
    --
    -- @param mesh Mesh name.
    -- @param mesh_radius Radius of the mesh in the original units.
    -- @param radius Final radius in scene units.
    ground_ring = function(self, mesh, mesh_radius, x, y, z, radius, life, delay)
        self:spawn_piece({
            mesh = mesh, x = x, y = y, z = z + 0.05, rx = 90, life = life, delay = delay or 0,
            update = function(piece, u, entity)
                local s = radius / mesh_radius * (0.3 + 0.7 * math.min(1, u / 0.5))
                entity:set_scale(s, s, s)
                entity:set_alpha(self.fade(u, 0.15, 0.5))
            end,
        })
    end,

    --- Effects of specific attacks, by attack id (the position in Game.Attacks). They are used
    -- before the ones by element. Each one gets the position of the target and a flag that is
    -- true when it's the first target of the attack. The ones that have sprites, instead of
    -- meshes, get the position in the UI too.
    ATTACK_EFFECTS = {
        -- Regen: green sparkles that keep rising around the target.
        [3] = function(self, x, y, z, power, ux, uy)
            for i = 1, 16 do
                self:spawn_sprite({
                    sheet = "sparkle", x = ux + (math.random() - 0.5) * 70, y = uy + 25 - math.random() * 30,
                    size = 10 + 8 * math.random(), life = 0.7 + 0.4 * math.random(), delay = math.random() * 1.2,
                    vy = -35 - 25 * math.random(), tint = {0.4, 1, 0.5},
                })
            end
        end,

        -- Barrier: a sphere of light around the target.
        [15] = function(self, x, y, z) self:barrier_sphere("common_bari_m", x, y, z, 1.1, 1.3) end,

        -- MBarrier: the same with its own mesh.
        [16] = function(self, x, y, z) self:barrier_sphere("magic_mbaria_jo_mba", x, y, z, 1.1, 1.3) end,

        -- Reflect and Wall: both barriers at the same time.
        [17] = function(self, x, y, z)
            self:barrier_sphere("common_bari_m", x, y, z, 1.2, 1.5)
            self:ground_ring("magic_stop_stop_1", 9, x, y, z, 1.3, 1.2)
        end,
        [18] = function(self, x, y, z)
            self:barrier_sphere("common_bari_m", x, y, z, 1.1, 1.5)
            self:barrier_sphere("magic_mbaria_jo_mba", x, y, z, 1.3, 1.5)
        end,

        -- Slow: a pair of needles that fall on the target.
        [20] = function(self, x, y, z)
            for i = 1, 3 do
                self:spawn_piece({
                    mesh = "magic_slou_hari", x = x, y = y, z = z + 1.4, life = 0.7, delay = 0.12 * (i - 1),
                    update = function(piece, u, entity)
                        local drop = 1 - math.min(1, u / 0.5)
                        entity:set_position(x, y, z + 0.3 + 1.1 * drop)
                        entity:set_scale(0.06, 0.06, 0.06)
                        entity:set_alpha(self.fade(u, 0.1, 0.4))
                    end,
                })
            end
        end,

        -- Stop: rings that close on the target.
        [21] = function(self, x, y, z)
            self:ground_ring("magic_stop_stop_1", 9, x, y, z, 1.4, 1.1)
            self:ground_ring("magic_stop_stop_2", 9, x, y, z, 1.0, 1.1, 0.2)
        end,

        -- DeBarrier: a big circle of light on the ground.
        [22] = function(self, x, y, z)
            self:ground_ring("magic_debaria_light_1", 39, x, y, z, 1.6, 1.2)
        end,

        -- Death: the same kind of circle.
        [24] = function(self, x, y, z)
            self:ground_ring("magic_dethper_light_1", 39, x, y, z, 1.6, 1.2)
        end,
    },

    --- Strength of a spell for the size of its effect: 1 for the basic one, 2 and 3 for the
    -- upgrades (named like Fire2 and Fire3).
    get_power = function(self, attack)
        local level = tonumber(string.match(attack.name or "", "(%d)%s*$"))
        if level == nil then return 1 end
        return 1 + (level - 1) * 0.5
    end,

    --- Shows the effect of a spell on a target, made from the extracted assets: the one of the
    -- attack if there is one, or the one of its element for the damage spells.
    --
    -- @return True if there is an effect for the attack.
    play_model_effect = function(self, attack, side, index, heals)
        if attack == nil or attack.id == nil or attack.id < 0 then do return false end end
        local effect = self.ATTACK_EFFECTS[attack.id]
        if effect == nil and not heals and (attack.damage.formula or 0) == 2
          and attack.elements ~= nil and #attack.elements > 0 then
            effect = self.MODEL_EFFECTS[attack.elements[1]]
        end
        if effect == nil or not self.models_available then return false end
        local entity = UiContainer.BattleUi:get_entity(side, index)
        if entity == nil then return false end
        local x, y, z = entity:get_position()
        local ux, uy = self:get_position(side, index, self.BODY_HEIGHT)
        effect(self, x, y, z, self:get_power(attack), ux, uy)
        return true
    end,

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
        self:clear_pieces()
        self:clear_sprites()
        for _, number in pairs(self.numbers) do ui_manager:get_widget("BattleFx.N" .. tostring(number.index)):set_visible(false) end
        ui_manager:get_widget("BattleFx.Flash"):set_visible(false)
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

    --- Effect of an attack that hits a target: the effects made from the original assets.
    --
    -- @param attack Attack data.
    -- @param result The result of the attack, as in BattleUi.calculate_attack.
    hit = function(self, attack, side, index, result)
        local x, y = self:get_position(side, index, self.BODY_HEIGHT)
        if x == nil then do return end end
        local heals = result.amount < 0
        self:play_model_effect(attack, side, index, heals)
        if heals and attack ~= nil and attack.id ~= nil and attack.id >= 0 then
            self:heal_sparkles(x, y, self:get_power(attack))
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
        self:update_pieces(dt)
        self:update_sprites(dt)
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
