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
        -- Sprites follow the combatant they were made for, because the camera can move (see
        -- get_position).
        if p.anchor == nil and self.spawn_anchor ~= nil then
            p.anchor = self.spawn_anchor
            p.ax, p.ay = self:project(p.anchor)
        end
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
                local shift_x, shift_y = 0, 0
                if p.anchor ~= nil and p.ax ~= nil then
                    local nx, ny = self:project(p.anchor)
                    if nx ~= nil then shift_x, shift_y = nx - p.ax, ny - p.ay end
                end
                widget:set_x(0, p.x - p.size / 2 + shift_x)
                widget:set_y(0, p.y - p.size / 2 + shift_y)
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
        -- Some meshes have no texture in the files: the original code binds one when it draws them.
        if p.texture ~= nil then p.entity:set_texture(p.texture) end
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

        -- Shiva (Diamond Dust): ice spikes of the summon over the shards of the Ice spell.
        [57] = function(self, x, y, z, power, ux, uy)
            self.MODEL_EFFECTS[1](self, x, y, z, 2)
            local meshes = {"summon_siva_ice_1", "summon_siva_ice_2", "summon_siva_ice_3"}
            local count = 6
            for i = 1, count do
                local angle = (i / count) * 2 * math.pi + math.random() * 0.4
                local radius = (i == 1) and 0 or 0.35 + 0.35 * math.random()
                local size = 0.011 + 0.004 * math.random()
                self:spawn_piece({
                    mesh = meshes[(i - 1) % 3 + 1],
                    x = x + math.cos(angle) * radius, y = y + math.sin(angle) * radius, z = z,
                    rx = 90, life = 1.4, delay = 0.08 * i,
                    update = function(piece, u, entity)
                        local grow = math.min(1, u / 0.3)
                        entity:set_scale(size * (0.5 + 0.5 * grow), size * (0.2 + 0.8 * grow), size * (0.5 + 0.5 * grow))
                        entity:set_alpha(self.fade(u, 0.15, 0.4))
                    end,
                })
            end
        end,

        -- Ifrit (Hellfire): the flames of Fire with the explosion rings of the summon.
        [58] = function(self, x, y, z, power, ux, uy)
            self.MODEL_EFFECTS[0](self, x, y, z, 3)
            self:ground_ring("summon_ifu_ring_1", 7, x, y, z, 1.8, 1.3)
            self:ground_ring("summon_ifu_ring_1", 7, x, y, z, 1.2, 1.3, 0.25)
        end,

        -- Ramuh (Judgement Bolt): bolts that fall on the target.
        [59] = function(self, x, y, z, power, ux, uy)
            self.MODEL_EFFECTS[2](self, x, y, z, 3)
        end,
    },

    --- Summons, by attack id. The times are in ticks of the original effect (tps per second) and
    -- come from ff7.exe (see the notes on each one). The creature is the mesh (in
    -- models/battle/attacks) and its animation, which has one frame per tick, made by the
    -- installer at 30 frames per second.
    --
    -- appear, leave: ticks when the creature shows up and goes away.
    -- hit: tick when the damage lands, total: tick when the whole effect ends.
    -- animations: list of {tick, name}, the animation of the creature that starts at each tick.
    -- base: distance (original units) from the targets to the point the effect works from, camera:
    -- names of the two camera scripts (Game.EffectCameras, by address in ff7.exe). The game picks
    -- one at random: rand() & 0x100 chooses the first (found in the code of the effects).
    -- events: list of {tick, effect} (id of the ATTACK_EFFECTS to play on every target) or
    -- {tick, sound} (name of a sound). height: curve of the height of the creature over the
    -- ground, a list of {tick, height} in original units, with straight lines between the points
    -- (before the first and after the last one the height is the one of the point).
    -- scales: curve of the size of the creature, a list of {tick, scale} (1 is the size of the
    -- model in the files), used instead of scale when there is one.
    -- scale: size of the creature, place: where it stands between the caster (0) and the targets (1).
    SUMMONS = {
        -- Shiva. Main function 0x58f6fe of ff7.exe (1998 build): the creature (0x5924a8) is
        -- there from tick 34 to 198, the hit slots of the targets are created at tick 235 and the
        -- effect ends at tick 255. The ice (tick 200) is a guess of which callbacks are the ice.
        [57] = {
            mesh = "attacks/cyvadat", scale = 1.2, place = 0.55,
            animations = {{tick = 34, name = "cyvadat_00"}},
            appear = 34, leave = 198, hit = 235, total = 255,
            -- She comes down 2000 units (original units) in the 75 ticks after she appears, to 1000
            -- units over the ground (the y of the code goes from -3000 to -1000).
            height = {{34, 3000}, {109, 1000}},
            -- She stands 5000 units from the targets, toward the caster, and the camera of the
            -- effect (Game.EffectCameras, by address in ff7.exe) works from that point.
            base = 5000, camera = {"0x8ae268", "0x8ae340"},
            -- Sounds: the code plays three at tick 170 (612, 613, 614) and two at 200 (615, 616), but
            -- there is only one voice for sounds, so only the first of each group is played.
            -- The pieces of the callbacks: the spotlight (0x590781, tick 5), the snowflakes (every tick from
            -- tick 5), the glints (5 per tick from 105 to 164) and the ice (0x5912bf, tick 200). The storm
            -- models (0x5909c4, 0x590afe, tick 120), the particles and sparks (0x590cfa, tick 170) and the
            -- smoke (0x591fd3, tick 200) are not done.
            -- The ice (0x5912bf and 0x591745): four shapes in the data of the executable (the models of
            -- ICE_1 to ISE_4 are the same ones at a smaller size), that grow in 5 ticks to 1.125, stay
            -- until the tick 25, and then they break into their triangles, that go away along their
            -- normals while the whole shape turns, for 30 ticks. They are in the places of the table at
            -- 0x8ae508, turned 180 degrees around the first one, as seen in the original.
            meshes = (function()
                local spots = {{0, 3000}, {2500, 2500}, {-3000, 2000}, {4800, 1200}}
                local blocks = {"0x8aa590", "0x8ac6f8", "0x8ad278", "0x8ada58"}
                local list = {}
                for i = 1, 4 do
                    local rates = {}
                    for k = 1, 3 do rates[k] = (math.random(0, 255) - 128) * 360 / 4096 end
                    local place = {-spots[i][1], 0, 8000 - spots[i][2]}
                    list[i] = {
                        name = blocks[i], texture = "battle_fx/common_water1.png", uv = {u = 0, v = 0, w = 256, h = 256},
                        blend = 1, start = 200, length = 55,
                        shatter = function(elapsed)
                            local age = math.floor(elapsed)
                            local size = age < 5 and age / 5 * 1.125 or 1.125
                            if age < 25 then return size, nil, place, 1, nil end
                            local ticks = age - 25 + 1
                            return size, {rates[1] * ticks, rates[2] * ticks, rates[3] * ticks}, place, 1 - (age - 25) / 30, ticks
                        end,
                    }
                end
                return list
            end)(),
            emitters = {
                {kind = "siva_spot", start = 5, ticks = 1},
                {kind = "siva_snow", start = 5, ticks = 230},
                {kind = "siva_glints", start = 105, ticks = 60},
                {kind = "siva_storm", start = 120, ticks = 40},
                {kind = "siva_crystals", start = 170, ticks = 30},
                {kind = "siva_smoke", start = 200, ticks = 1},
            },
            events = {
                {tick = 170, sound = "612"},
                {tick = 200, sound = "615"},
            },
        },

        -- Ifrit. Main function 0x593a05: the creature is shown at tick 14 and released at 182 (the
        -- two animations, of 100 and 45 frames, follow each other: 14 + 100 = 114), the hit slots
        -- are created at 197 and it ends at 212. The fire (tick 162, where a callback and sounds
        -- start) is a guess. Sounds: three at 94 (428-430), three at 162 (431-433). The callback
        -- 0x594cf2 (registered at tick 14) moves him from 1200 to 3200 units over the ground in 30
        -- ticks, and grows his size from 0x1a00 to 0x1c00 (1.625 to 1.75, in 1/4096) in the next 50.
        -- The hole he comes out of is drawn by 0x594022 (not done).
        [58] = {
            mesh = "attacks/ifreetda", scale = 1.625, place = 0.55,
            animations = {{tick = 14, name = "ifreetda_00"}, {tick = 114, name = "ifreetda_01"}},
            height = {{14, 1200}, {44, 3200}},
            scales = {{14, 1.625}, {44, 1.625}, {94, 1.75}},
            -- The animations of Ifrit raise his root bone 1103 units (in every frame).
            root_y = 1103,
            -- The ground (0x594022, registered at tick 4): a mesh of the data of the executable that
            -- grows in 10 ticks, changes through nine shapes (every 4 ticks) and goes away at 40.
            meshes = {
                {name = "0x8af9b0", texture = "battle_fx/common_graund.png", uv = {u = 128, v = 128, w = 128, h = 128}, start = 4, length = 40, key_ticks = 4, ramp = 10, lift = 160},
                -- The flame halo around Ifrit (0x596853, tick 144, 18 ticks): the mesh of the executable at
                -- 0x8b4588 (a big flared cone with the texture of the flames) squashed to 0.25 x 0.25 x 0.0625
                -- and tilted 22.5 degrees, turned 180, over his head at (0, -2500, 1500) that moves
                -- (0, -10, -20) every tick, and goes away 1000 and 2000 higher in the last 3. It flickers.
                {name = "0x8b4588", texture = "battle_fx/summon_ifu_flame_1.png", uv = {u = 0, v = 0, w = 256, h = 128}, blend = 1,
                 start = 144, length = 18,
                 transform = function(n)
                     local ty, tz = -2500 - 10 * math.min(n, 15), 1500 - 20 * math.min(n, 15)
                     if n > 15 then ty, tz = ty + 1000 * (n - 15), tz + 2000 * (n - 15) end
                     local flicker = (math.floor(n) % 2 == 0) and 0.5 or 0
                     local bright = n < 15 and (1 + (flicker - 1) * n / 15) or 0
                     return {0.25, 0.25, 0.0625}, {22.5, 180, 0}, {0, ty, tz}, bright
                 end},
                -- The jet of fire (0x595002, tick 162, 20 ticks): the same mesh, flipped so the wide end
                -- trails (x -0.1875, y 0.1875, z from -0.1875 to -1.4), 1000 units over the ground, that
                -- goes 1500 units forward every tick from -5000 (it crosses the targets).
                {name = "0x8b4588", texture = "battle_fx/summon_ifu_flame_1.png", uv = {u = 0, v = 0, w = 256, h = 128}, blend = 1,
                 start = 162, length = 20,
                 transform = function(n)
                     return {-0.1875, 0.1875, -(0.1875 + n * 0x1500 / 20 / 4096)}, nil, {0, -1000, n * 1500 - 5000}, 1
                 end},
            },
            appear = 14, leave = 182, hit = 197, total = 212,
            base = 5000, camera = {"0x8b60c0", "0x8b61a8"},
            -- The sounds are the first of each group. The fire on the targets is a placeholder at the time
            -- the jet crosses them (162 + 6.6), until the callbacks of the hit (0x595975) are done.
            emitters = {
                -- The column of flames at the hole (0x594b77, tick 14, 15 ticks).
                {kind = "flame_column", start = 14, ticks = 1},
                {kind = "ifu_dome", start = 44, ticks = 5},
                {kind = "ifu_smoke", start = 115, ticks = 29},
                {kind = "ifu_flames", start = 182, ticks = 6},
            },
            events = {
                {tick = 94, sound = "428"},
                {tick = 169, effect = 58},
                {tick = 162, sound = "431"},
                -- The hit (0x595975, tick 197): the original draws 42 quads of the screen in a pink
                -- (0x2e8080ff), 5 ticks in and 5 out. This is a pink flash of 10 ticks: an approximation.
                {tick = 197, flash = {colour = {1, 0.5, 0.5}, strength = 0.35, life = 10 / 15}},
            },
        },

        -- Ramuh. Main function 0x598071: the creature is shown at tick 104 and released at 209
        -- (animations of 60 and 20 frames: 104 + 60 = 164, and 164 + 20 = 184, where a segment
        -- starts), the hit slots are created at 239 and it ends at 259. The bolts (tick 184) are a
        -- guess.
        [59] = {
            mesh = "attacks/lamdat", scale = 1.0, place = 0.55,
            animations = {{tick = 104, name = "lamdat_00"}, {tick = 164, name = "lamdat_01"}},
            appear = 104, leave = 209, hit = 239, total = 259,
            base = 5000, camera = {"0x8ba728", "0x8ba7f0"},
            -- The mountain (0x598f74, registered at tick 34): a mesh of the data of the executable that
            -- rises to the shapes 1, 3 and 4 (15 ticks each, with holds of 5, 5 and 119 ticks), from the
            -- durations that follow its table of shapes in the executable (schedule = "tail").
            -- The camera looks at his height: from 3000 to 5800 units in 80 ticks from tick 104.
            meshes = {{name = "0x8b7f88", texture = "battle_fx/common_rock.png", uv = {u = 0, v = 0, w = 256, h = 128}, start = 34, length = 174, lift = 20,
                       schedule = "tail"}},
            height = {{104, 3000}, {184, 5800}},
            scales = {{104, 1.0}},
            -- Rocks that fly from random vertices of the mountain while it grows: 3 per tick for 55
            -- ticks (0x599bad, from the function of the mountain). They live 12 ticks. The mesh is
            -- STONE_1, tiny in the files (14 units): size is a factor to see it (tested by eye).
            debris = {{mesh = "summon_lamu_stone_1", from = 1, start = 34, ticks = 55, per_tick = 3, life = 12, size = 14}},
            -- Sprites (see EMITTERS): the first puff of smoke at tick 4 (0x598907, 15 ticks), more puffs from
            -- tick 24 to 181 (0x598d99), the ring of 8 lightnings at the top of the mountain at
            -- tick 194 (0x59a662, 43 ticks each) and the sparks around the targets, 2 per tick for 45
            -- ticks from tick 194 (0x59ac0d), and the yellow flash at tick 184 (0x59a408). The other flash
            -- of that tick (0x599feb, a sprite of 5 ticks) isn't done.
            emitters = {
                {kind = "smoke_first", start = 4, ticks = 1},
                {kind = "smoke_puffs", start = 24, ticks = 157},
                {kind = "flash_disc", start = 184, ticks = 1},
                {kind = "bolt_ring", start = 194, ticks = 1},
                {kind = "sparks", start = 194, ticks = 45},
            },
            -- The screen shakes (0x598763): the amplitude (pixels of a 320 x 240 screen) from each tick.
            shake = {{34, 50}, {44, 30}, {89, 10}, {104, 0}},
            -- His origin is at his base and the camera looks at the middle of his body: his mesh
            -- is about 850 units tall (-390 to 458), so he goes down half of that (tested by eye:
            -- he was about half his height too high).
            root_y = 424,
            -- (The placeholder bolts, ATTACK_EFFECTS[59], aren't used anymore: the sparks of the
            -- emitters are the original ones.)
        },
    },

    --- Ticks per second of the original effects. It's the one of the battle, 15 (from forum posts
    -- about the PC version, and it makes the creature animation, 165 frames, last as long as the
    -- creature is shown in the original code).
    SUMMON_TPS = 15,
    -- The bone of Shiva's hand where the storm gathers (a guess: change it if it's in the wrong place).
    SIVA_HAND_BONE = 12,

    --- Frames per second the animations of the files are made at by the installer.
    ANIMATION_FPS = 30,

    --- Summons being played.
    timelines = {},

    --- True if the attack is a summon with a sequence.
    has_summon = function(self, id)
        return id ~= nil and self.SUMMONS[id] ~= nil and self.models_available
    end,

    --- Plays the sequence of a summon: the creature appears, plays its animation, the effects
    -- happen on the targets and the hit lands, at the ticks of its definition.
    --
    -- @param attack Attack data.
    -- @param side Side of the caster.
    -- @param actor Index of the caster.
    -- @param target_group Side of the targets.
    -- @param targets List of indices of the targets.
    -- @param on_hit Function called when the hit lands.
    -- @param on_done Function called when the sequence ends.
    play_summon = function(self, attack, side, actor, target_group, targets, on_hit, on_done)
        local def = self.SUMMONS[attack.id]
        local timeline = {
            age = 0, def = def, on_hit = on_hit, on_done = on_done, hit_done = false,
            target_group = target_group, targets = targets, events_done = {},
        }
        -- Stands between the caster and the center of the targets.
        local caster = UiContainer.BattleUi:get_entity(side, actor)
        local cx, cy, cz = 0, 0, 0
        if caster ~= nil then cx, cy, cz = caster:get_position() end
        local tx, ty, count = 0, 0, 0
        for _, t in ipairs(targets) do
            local entity = UiContainer.BattleUi:get_entity(target_group, t)
            if entity ~= nil then
                local x, y = entity:get_position()
                tx, ty, count = tx + x, ty + y, count + 1
            end
        end
        if count > 0 then tx, ty = tx / count, ty / count else tx, ty = cx, cy end
        timeline.x = cx + (tx - cx) * def.place
        timeline.y = cy + (ty - cy) * def.place
        timeline.z = cz
        -- The space of the effect: forward goes from the caster to the targets, and the origin is
        -- `base` units from the targets, toward the caster.
        local fx, fy = tx - cx, ty - cy
        local l = math.sqrt(fx * fx + fy * fy)
        if l > 0.0001 then fx, fy = fx / l, fy / l else fx, fy = -1, 0 end
        timeline.f = {x = fx, y = fy}
        -- To the right of someone looking toward the targets (with the Z axis up).
        timeline.lateral = {x = fy, y = -fx}
        local base = (def.base or 0) * self.MODEL_UNIT
        timeline.origin = {x = tx - fx * base, y = ty - fy * base, z = cz}
        if def.base ~= nil then timeline.x, timeline.y = timeline.origin.x, timeline.origin.y end
        -- The camera of the effect.
        if def.camera ~= nil and EffectCamera ~= nil then
            local words = EffectCamera.get_script(def.camera[math.random(#def.camera)])
            if words ~= nil then
                timeline.camera = EffectCamera.new(words)
                timeline.camera_tick = 0
            end
        end
        -- Faces the targets, as the party members do.
        timeline.rotation = side == Battle.TargetGroup.PARTY and 0 or 180
        self.timelines[#self.timelines + 1] = timeline
    end,

    --- Creates the entity of the creature of a summon.
    start_creature = function(self, t)
        local name = battle_manager:add_effect_model(t.def.mesh, t.x, t.y, t.z, t.def.scale)
        if name == nil or name == "" then return end
        t.entity_name = name
        t.entity = entity_manager:get_entity(name)
        t.entity:set_rotation(t.rotation)
        t.entity:set_blend(Entity.BLEND_ALPHA)
        t.entity:set_alpha(0)
        -- The animation has one frame per tick of the effect.
        t.entity:set_animation_speed(self.SUMMON_TPS / self.ANIMATION_FPS)
        t.animation_index = 0
    end,

    --- Draws the meshes of the data of the executable that a summon has (def.meshes), from the
    -- tick they start to the end of their length.
    update_meshes = function(self, t, tick)
        if EffectMesh == nil then return end
        t.meshes = t.meshes or {}
        for i, def in ipairs(t.def.meshes or {}) do
            local elapsed = tick - def.start
            local mesh = t.meshes[i]
            if elapsed >= 0 and elapsed < def.length then
                if mesh == nil and not t.mesh_failed then
                    local data = EffectMesh.get_data(def.name)
                    if data ~= nil then mesh = EffectMesh.new(data, def) end
                    if mesh == nil then t.mesh_failed = true else t.meshes[i] = mesh end
                end
                if mesh ~= nil then
                    mesh:advance(math.floor(elapsed))
                    local scale = def.ramp ~= nil and math.min(1, elapsed / def.ramp) or 1
                    -- The lift moves the mesh up (original units): its lowest points are below the ground
                    -- of the effect, and the floor of the battle would hide them (the original game
                    -- doesn't test depth).
                    local lift = def.lift or 0
                    -- A transform gives the size of every axis, the rotation (degrees around X, Y and Z),
                    -- the position (effect units) and the brightness for the time of the mesh.
                    local sizes, rotation, offset, brightness = scale, nil, nil, 1
                    if def.transform ~= nil then sizes, rotation, offset, brightness = def.transform(elapsed) end
                    if def.shatter ~= nil then
                        -- (the ice of Shiva, that breaks into its triangles)
                        local ticks
                        sizes, rotation, offset, brightness, ticks = def.shatter(elapsed)
                        mesh:draw_shatter(function(x, y, z)
                            return self:effect_to_scene(t, {x + offset[1], y + offset[2], z + offset[3]})
                        end, sizes, brightness, rotation, ticks)
                    else
                    mesh:draw(function(x, y, z)
                        if rotation ~= nil then x, y, z = EffectSprite.rotate(x, y, z, rotation[1], rotation[2], rotation[3]) end
                        if offset ~= nil then x, y, z = x + offset[1], y + offset[2], z + offset[3] end
                        return self:effect_to_scene(t, {x, y - lift, z})
                    end, sizes, brightness)
                    end
                end
            elseif mesh ~= nil then
                mesh:remove()
                t.meshes[i] = nil
            end
        end
    end,

    --- Removes the meshes of the executable of a summon.
    remove_meshes = function(self, t)
        for i, mesh in pairs(t.meshes or {}) do mesh:remove() end
        t.meshes = {}
    end,

    --- Value of a curve made of points with straight lines between them.
    --
    -- @param points List of {tick, value}, in order of tick.
    -- @param tick Where to get the value. Outside of the points, the value of the closest one.
    curve = function(points, tick)
        if tick <= points[1][1] then return points[1][2] end
        for i = 2, #points do
            if tick <= points[i][1] then
                local a, b = points[i - 1], points[i]
                return a[2] + (b[2] - a[2]) * (tick - a[1]) / (b[1] - a[1])
            end
        end
        return points[#points][2]
    end,

    --- Starts the animations of the creature whose tick has come.
    update_animations_of_creature = function(self, t, tick)
        local animations = t.def.animations or {}
        while t.animation_index < #animations and tick >= animations[t.animation_index + 1].tick do
            t.animation_index = t.animation_index + 1
            t.entity:play_animation_stop(animations[t.animation_index].name)
        end
    end,

    --- Plays the effect of an attack on every target of a summon.
    play_effect_on_targets = function(self, t, id)
        local effect = self.ATTACK_EFFECTS[id]
        if effect == nil then return end
        for _, index in ipairs(t.targets) do
            local entity = UiContainer.BattleUi:get_entity(t.target_group, index)
            if entity ~= nil then
                local x, y, z = entity:get_position()
                local ux, uy = self:get_position(t.target_group, index, self.BODY_HEIGHT)
                effect(self, x, y, z, 1, ux, uy)
            end
        end
    end,

    --- Calls a function of an effect and prints the first error it has (once per summon and name),
    -- so a problem isn't silent.
    safe = function(self, t, name, fn, ...)
        local ok, err = pcall(fn, ...)
        if not ok then
            t.reported = t.reported or {}
            if not t.reported[name] then
                t.reported[name] = true
                print("[BattleFx] error in " .. name .. ": " .. tostring(err))
            end
        end
    end,

    --- Factor of the size of the sparks around the targets, chosen by eye (3 was still small).
    SPARK_SIZE = 6,

    --- Sheets of sprites: the image, its size in the texture page of the original game and the sprite
    -- file (Game.EffectAnimations).
    SPRITE_SHEETS = {
        smoke = {texture = "battle_fx/common_lamu01.png", width = 256, height = 256, anim = "summon_lamu_smoke_1"},
        spark = {texture = "battle_fx/common_lamu00.png", width = 256, height = 256, anim = "summon_lamu_jo_c04"},
        lightning = {texture = "battle_fx/common_lamu00.png", width = 256, height = 256, anim = "common_houden_2"},
        -- Ifrit: the smoke (IFLEET03.TIM) and the flames (IFLEET00.TIM for FLAME_A to E and IFLEET01.TIM
        -- for FLAME_F to I; every file is a frame of the flame, see the emitter ifu_flames).
        ifu_smoke = {texture = "battle_fx/common_ifleet03.png", width = 256, height = 256, anim = "summon_ifu_smoke_1"},
        ifu_flame_00 = {texture = "battle_fx/common_ifleet00.png", width = 256, height = 256, anim = "common_flame_a"},
        ifu_flame_01 = {texture = "battle_fx/common_ifleet01.png", width = 256, height = 256, anim = "common_flame_f"},
        siva_smoke = {texture = "battle_fx/common_siva03.png", width = 256, height = 256, anim = "summon_siva_smoke_1"},
        siva_kira = {texture = "battle_fx/common_siva02.png", width = 256, height = 256, anim = "summon_siva_kira_1"},
        -- Shiva: the snowflake (SIVA02.TIM).
        siva_snow = {texture = "battle_fx/common_siva02.png", width = 256, height = 256, anim = "summon_siva_snow_1"},
    },

    --- A point of a mesh of the executable of a summon, in the space of the effect.
    mesh_point = function(self, t, mesh_index, vertex)
        local mesh = t.meshes ~= nil and t.meshes[mesh_index] or nil
        if mesh == nil then return {0, 0, 0} end
        return {mesh.current[vertex * 3 + 1], mesh.current[vertex * 3 + 2], mesh.current[vertex * 3 + 3]}
    end,

    --- Adds a sprite that plays the frames of a sprite file, one per tick.
    --
    -- @param p Table with sheet (name in SPRITE_SHEETS), frames (list of frames, from 0, one per
    -- tick), birth (tick), pos (a position of the effect, or a function that gets it), scale (a
    -- number, or a function of the age in ticks), angles (degrees, around X, Y and Z) and rate (degrees
    -- of change of the angles every tick), and brightness (function of the age, from 0 to 1).
    add_sprite = function(self, t, p)
        t.sprites = t.sprites or {}
        t.sprites[#t.sprites + 1] = p
    end,

    --- Draws the sprites of a summon.
    update_sprites3d = function(self, t, tick)
        if t.sprites == nil then return end
        t.batches = t.batches or {}
        local alive = {}
        local k = self.MODEL_UNIT
        for _, p in ipairs(t.sprites) do
            local age = tick - p.birth
            -- A sprite has one file and the frame to show in each tick (frames), or a file and a frame
            -- for each tick (sequence, for the effects that change file every tick).
            local count = p.sequence ~= nil and #p.sequence or #p.frames
            if age >= count then
                -- It's over.
            else
                alive[#alive + 1] = p
                local step = math.floor(age) + 1
                local sheet_name, frame_number, anim = p.sheet, nil, nil
                if p.sequence ~= nil then
                    local entry = p.sequence[step]
                    sheet_name, anim, frame_number = entry.sheet, entry.anim, entry.frame or 0
                else
                    frame_number = p.frames[step]
                end
                local sheet = self.SPRITE_SHEETS[sheet_name]
                local frames = EffectSprite.get_frames(anim or sheet.anim)
                local frame = frames ~= nil and frames[frame_number + 1] or nil
                if frame ~= nil and age >= 0 then
                    local batch = t.batches[sheet_name]
                    if batch == nil then
                        batch = EffectSpriteBatch.new(sheet.texture, sheet.width, sheet.height, sheet.blend or 1)
                        t.batches[sheet_name] = batch
                    end
                    if batch ~= nil then
                        local pos = type(p.pos) == "function" and p.pos(age) or p.pos
                        local scale = type(p.scale) == "function" and p.scale(age) or p.scale
                        local scale_y = p.scale_y or scale
                        local a = p.angles or {0, 0, 0}
                        local r = p.rate or {0, 0, 0}
                        local ax, ay, az = a[1] + r[1] * age, a[2] + r[2] * age, a[3] + r[3] * age
                        local bright = p.brightness ~= nil and p.brightness(age) or 1
                        local colour = {bright, bright, bright, sheet.alpha or 1}
                        local corner
                        if p.billboard and t.view ~= nil then
                            -- Faces the camera: the corners go along the right and up of the screen
                            -- (the y of a sprite goes down).
                            local cx, cy, cz = self:effect_to_scene(t, pos)
                            local right, up = t.view.right, t.view.up
                            corner = function(x, y)
                                local dx, dy = x * 8 * scale * k, y * 8 * scale_y * k
                                return {cx + right[1] * dx - up[1] * dy, cy + right[2] * dx - up[2] * dy, cz + right[3] * dx - up[3] * dy}
                            end
                        else
                            corner = function(x, y)
                                local rx, ry, rz = EffectSprite.rotate(x * 8 * scale, y * 8 * scale_y, 0, ax, ay, az)
                                local sx, sy, sz = self:effect_to_scene(t, {pos[1] + rx, pos[2] + ry, pos[3] + rz})
                                return {sx, sy, sz}
                            end
                        end
                        for _, q in ipairs(frame) do
                            local x, y, u, v, w, h = q[1], q[2], q[3], q[4], q[5], q[6]
                            batch:add_quad(corner(x, y), corner(x + w, y), corner(x, y + h), corner(x + w, y + h), u, v, u + w, v + h, colour)
                        end
                    end
                end
            end
        end
        t.sprites = alive
        for _, batch in pairs(t.batches) do batch:flush() end
    end,

    --- Draws the flat flashes of a summon: discs of sixteen sides that face the camera, with a
    -- radius and a colour that change every tick (see 0x59a408 and 0x59a4e0 in the executable).
    update_flashes = function(self, t, tick)
        if t.flashes == nil then return end
        t.batches = t.batches or {}
        local alive = {}
        local batch = t.batches.flat
        if batch == nil and #t.flashes > 0 then
            batch = EffectSpriteBatch.new("", 1, 1, 1)
            t.batches.flat = batch
        end
        local k = self.MODEL_UNIT
        for _, f in ipairs(t.flashes) do
            local age = tick - f.birth
            if age < f.life then
                alive[#alive + 1] = f
                if age >= 0 and batch ~= nil and t.view ~= nil then
                    local radius, colour = f.shape(age)
                    local cx, cy, cz = self:effect_to_scene(t, f.pos)
                    local right, up = t.view.right, t.view.up
                    local function rim(step)
                        local a = step * math.pi * 2 / 16
                        local dx, dy = math.cos(a) * radius * k, math.sin(a) * radius * k
                        return {cx + right[1] * dx + up[1] * dy, cy + right[2] * dx + up[2] * dy, cz + right[3] * dx + up[3] * dy}
                    end
                    for step = 0, 15 do
                        batch:add_triangle({cx, cy, cz}, rim(step), rim(step + 1), colour)
                    end
                end
            end
        end
        t.flashes = alive
        if batch ~= nil then batch:flush() end
    end,

    --- Removes the sprites and flashes of a summon.
    remove_sprites = function(self, t)
        for _, batch in pairs(t.batches or {}) do batch:remove() end
        t.batches, t.sprites, t.flashes = {}, {}, {}
    end,

    --- Size of the smoke of Ifrit relative to the sizes of the executable (0.125 to 0.25), chosen by eye
    -- (the animation is drawn by 0x5d1f6b and its unit isn't known).
    SMOKE_SIZE = 8,

    --- Converts a point of the scene to the space of an effect (the opposite of effect_to_scene).
    scene_to_effect = function(self, t, x, y, z)
        local k = self.MODEL_UNIT
        local dx, dy = x - t.origin.x, y - t.origin.y
        return {(dx * t.lateral.x + dy * t.lateral.y) / k, (t.origin.z - z) / k, (dx * t.f.x + dy * t.f.y) / k}
    end,

    --- What each kind of emitter of a summon creates, every tick it works (index is 0 for the first).
    EMITTERS = {
        -- The first puff of smoke, at the base of the mountain (0x598907 with 15 ticks).
        smoke_first = function(self, t, e, tick, index)
            local frames = {}
            for age = 0, 14 do frames[#frames + 1] = 3 - ((15 - age) % 4) end
            self:add_sprite(t, {sheet = "smoke", frames = frames, birth = tick, scale = 4.0, pos = function() return self:mesh_point(t, 1, 27) end})
        end,

        -- A puff every tick, tilted at random (0x598d99): 3 ticks each.
        smoke_puffs = function(self, t, e, tick, index)
            local tilt = function() return (math.random(0, 1023) - 512) * 360 / 4096 end
            self:add_sprite(t, {sheet = "smoke", frames = {0, 1, 2}, birth = tick, scale = 4.0,
                                pos = function() return self:mesh_point(t, 1, 27) end, angles = {tilt(), tilt(), 0}})
        end,

        -- Eight lightnings that spin around the top of the mountain (0x59a662 and 0x59a740).
        bolt_ring = function(self, t, e, tick, index)
            local frames = {}
            for age = 0, 42 do
                local f = age % 8
                frames[#frames + 1] = f < 4 and f or f - 4
            end
            for i = 0, 7 do
                local tilt = (math.random(0, 431) - 1536) * 360 / 4096
                local turn = (i * 512 + math.random(0, 255)) * 360 / 4096
                self:add_sprite(t, {
                    sheet = "lightning", frames = frames, birth = tick,
                    pos = function() return self:mesh_point(t, 1, 27) end,
                    scale = function(age) return age < 1 and 0 or (age < 2 and 3.5 or 7.0) end,
                    brightness = function(age) return age < 38 and 1 or math.max(0, 1 - (age - 38) / 5) end,
                    angles = {tilt, turn, 0}, rate = {0, 0x50 * 360 / 4096, 0},
                })
            end
        end,

        -- (No rotation: the meshes of the installer already have the 180 degrees around X of the P files
        -- applied, so the raw y of -24..0 is 0..24 up. With rx = 180 it was underground and not seen.)
        -- A column of flames at the origin (0x594b77, resource 1: FLAME_2, 1200 units tall in the game,
        -- 50 times its raw size): it gets 2.0 to 2.45 wide and 0 to 2.7 tall in 10 ticks, then 2.5 and
        -- 3.0 for 5 ticks while it fades.
        flame_column = function(self, t, e, tick, index)
            local x, y, z = self:effect_to_scene(t, {0, 0, 0})
            local unit = self.MODEL_UNIT * 50
            self:spawn_piece({
                mesh = "common_flame_2", x = x, y = y, z = z, rx = 0, blend = Entity.BLEND_ADDITIVE,
                texture = "battle_fx/summon_ifu_flame_1.png", -- FLAME_1.TIM, bound by the code of the effect
                life = 15 / self.SUMMON_TPS,
                update = function(piece, u, entity)
                    local age = u * 15
                    local wide, tall, alpha = 2.5, 3.0, 1
                    if age < 10 then wide, tall = 2.0 + age * 0.05, age * 0.3
                    else alpha = 1 - (age - 10) / 5
                    end
                    entity:set_scale(wide * unit, tall * unit, wide * unit)
                    entity:set_alpha(alpha)
                end,
            })
        end,

        -- Smoke from bone 5 of the creature (0x593cb3..0x593d33 of the main callback and the particles of
        -- 0x596d15): two puffs per tick, each with the speed of a table (-10, 0, -10) or (20, 0, -30)
        -- and a bit of chance (-5 to +4), that lose 1/16 of the speed every tick (and the depth speed
        -- goes 5 up), for 16 ticks, one frame of SMOKE_1 per tick.
        ifu_smoke = function(self, t, e, tick, index)
            if t.entity == nil then return end
            local bx, by, bz = t.entity:get_bone_position(5)
            local start = self:scene_to_effect(t, bx, by, bz)
            local speeds = {{-10, 0, -10}, {20, 0, -30}}
            for i = 1, 2 do
                local v = {}
                for c = 1, 3 do v[c] = speeds[i][c] + math.random(0, 9) - 5 end
                local pos, done = {start[1], start[2], start[3]}, 0
                local frames = {}
                for age = 0, 15 do frames[#frames + 1] = age end
                self:add_sprite(t, {
                    sheet = "ifu_smoke", frames = frames, birth = tick, billboard = true,
                    scale = (512 + math.random(0, 511)) / 4096 * self.SMOKE_SIZE,
                    pos = function(age)
                        while done < math.floor(age) do
                            done = done + 1
                            for c = 1, 3 do pos[c] = pos[c] + v[c] end
                            v[1], v[2], v[3] = v[1] * 0.9375, v[2] * 0.9375, v[3] * 0.9375 + 5
                        end
                        return pos
                    end,
                })
            end
        end,

        -- Spheres of light around Ifrit (0x596aee): one per tick for 5 ticks, at a random height
        -- between 2500 and 3500, 20 ticks each. They grow wide (x and z) 1 to 4 times with the
        -- sine of the age and fade out and in again. The size of the model is 200 for 26 of raw size.
        ifu_dome = function(self, t, e, tick, index)
            local base = (384 + math.random(0, 127)) / 4096
            local x, y, z = self:effect_to_scene(t, {0, -3500 + math.random(0, 999), 0})
            local unit = self.MODEL_UNIT * 200 / 26
            self:spawn_piece({
                mesh = "summon_ifu_exp_2", x = x, y = y, z = z, rx = 0, blend = Entity.BLEND_ADDITIVE,
                life = 20 / self.SUMMON_TPS,
                update = function(piece, u, entity)
                    local age = u * 20
                    local wide = base * (1 + 3 * math.sin(math.rad(age * 4.5)))
                    entity:set_scale(wide * unit, base * unit, wide * unit)
                    entity:set_alpha(age < 10 and (1 - age / 10) or ((age - 10) / 10))
                end,
            })
        end,

        -- Fire on the ground after the jet (0x595701 and 0x5957fc): three flames per tick for 6 ticks,
        -- at random places of a field of 6000 x 10000 units around the targets. Each one shows the nine
        -- sprite files FLAME_A to FLAME_I, one per tick, scaled 1.5 to 2.5.
        ifu_flames = function(self, t, e, tick, index)
            local letters = {"a", "b", "c", "d", "e", "f", "g", "h", "i"}
            for _ = 1, 3 do
                local sequence = {}
                for i, letter in ipairs(letters) do
                    sequence[i] = {sheet = i <= 5 and "ifu_flame_00" or "ifu_flame_01", anim = "common_flame_" .. letter, frame = 0}
                end
                self:add_sprite(t, {
                    sequence = sequence, birth = tick, billboard = true,
                    pos = {math.random(-3000, 2999), 0, math.random(0, 9999)},
                    scale = (0x1800 + math.random(0, 0xfff)) / 4096, scale_y = (0x1800 + math.random(0, 0xfff)) / 4096,
                })
            end
        end,

        -- Shiva's snowflakes (0x58fb8b and 0x590585): one every tick, high in the air at a random place of
        -- 6000 x 6000, that stays 100 ticks (the speed is 0 in the data) as the sprite of a snowflake.
        siva_snow = function(self, t, e, tick, index)
            self:add_sprite(t, {
                sheet = "siva_snow", frames = (function() local f = {} for i = 1, 100 do f[i] = 0 end return f end)(),
                birth = tick, billboard = true, scale = 1.0,
                pos = (function()
                    -- The flakes fall from 5000 up to the ground in their 100 ticks.
                    local x, z = math.random(-3000, 2999), math.random(-3000, 2999)
                    return function(age) return {x, -5000 + age * 50, z} end
                end)(),
            })
        end,

        -- Shiva's glints (0x5906c7): five per tick between ticks 105 and 164, tiny points of light for 20
        -- ticks in a volume of 10000 x 10000 x 10000, that get brighter from 20/255 to about 180/255.
        siva_glints = function(self, t, e, tick, index)
            t.flashes = t.flashes or {}
            for _ = 1, 5 do
                t.flashes[#t.flashes + 1] = {
                    birth = tick, life = 20,
                    pos = {math.random(-5000, 4999), math.random(-7000, 2999), math.random(-5000, 4999)},
                    shape = function(age)
                        local b = ((19 - age) * 0 + age * 8 + 20) / 255
                        return 40, {b, b, b, 1}
                    end,
                }
            end
        end,

        -- Shiva's ice (0x5912bf and 0x591745): four spikes at once at the places of a table, each with a
        -- model of ICE_1 to ISE_4 (this one doesn't exist in the files, so it isn't shown): they grow to
        -- 1.125 times the size of the table (800) in 5 ticks, stay until tick 25 and then spin and
        -- break for 30 ticks (here they fade while they spin).
        siva_spikes = function(self, t, e, tick, index)
            local spots = {{0, 3000}, {2500, 2500}, {-3000, 2000}, {4800, 1200}}
            local models = {"summon_siva_ice_1", "summon_siva_ice_2", "summon_siva_ice_3"}
            -- The table size (800) over the biggest coordinate of each model (110, 3 and 50 in the files).
            local reach = {110, 3, 50}
            for i = 1, 3 do
                -- The first place of the table (0, 3000) is taken as the place of the targets (5000 in
                -- front of the origin), and the others keep their distance to it.
                -- Turned 180 degrees around the first one (as seen in the original), so the sides and the
                -- depth of the others are the opposite ones of the table.
                local x, y, z = self:effect_to_scene(t, {-spots[i][1], 0, 8000 - spots[i][2]})
                local unit = self.MODEL_UNIT * 800 / reach[i]
                self:spawn_piece({
                    -- No turn: the model is drawn as it is (the installer already flipped it, so rx = 0).
                    mesh = models[i], x = x, y = y, z = z, rx = 0, blend = Entity.BLEND_ADDITIVE,
                    life = 55 / self.SUMMON_TPS,
                    update = function(p, u, entity)
                        local age = u * 55
                        local size, alpha = 1.125, 1
                        if age < 5 then size = age / 5 * 1.125
                        elseif age >= 25 then
                            -- The original breaks the polygons and throws them away for 30 ticks: here
                            -- the model swells and fades.
                            local f = (age - 25) / 30
                            size, alpha = 1.125 * (1 + f * 0.8), 1 - f * f
                        end
                        entity:set_scale(size * unit, size * unit, size * unit)
                        entity:set_alpha(alpha)
                    end,
                })
            end
        end,

        -- The hand of Shiva that the storm gathers around (the original takes the position from a
        -- bone of the model; the bone number is a guess, see SIVA_HAND_BONE).
        siva_hand = function(self, t)
            if t.entity == nil then return {0, -1500, 1000} end
            local bx, by, bz = t.entity:get_bone_position(self.SIVA_HAND_BONE)
            return self:scene_to_effect(t, bx, by, bz)
        end,

        -- The storm that gathers at the hand (0x5908a1, from tick 120): a disc of STOME_1 that grows for
        -- 50 ticks and then 15 more while it fades (0x5909c4), and every 4 ticks (10 times) a ring of
        -- STOME_2 with a random turn that shrinks from 3.0 to 0 in 10 ticks (0x590afe). Both models are
        -- flat (radius 10, 3204 in the game), so they are drawn facing the targets.
        siva_storm = function(self, t, e, tick, index)
            local hand = self.EMITTERS.siva_hand(self, t)
            local x, y, z = self:effect_to_scene(t, {hand[1], hand[2], hand[3] - 500})
            local unit = self.MODEL_UNIT * 3204 / 10
            if index == 0 then
                self:spawn_piece({
                    mesh = "common_stome_1", x = x, y = y, z = z, rx = 0, blend = Entity.BLEND_ADDITIVE,
                    life = 65 / self.SUMMON_TPS,
                    update = function(p, u, entity)
                        local age = u * 65
                        local size, alpha
                        if age < 50 then size, alpha = (128 + age / 50 * 2944) / 4096, 1
                        else
                            local f = (age - 50) / 15
                            size, alpha = (3072 + f * 13312) / 4096, 1 - f
                        end
                        entity:set_scale(size * unit, size * unit, size * unit)
                        entity:set_alpha(alpha)
                    end,
                })
            end
            if index % 4 == 0 then
                local turn = {math.random(0, 359), math.random(0, 359), math.random(0, 359)}
                local sx, sy, sz = self:effect_to_scene(t, hand)
                self:spawn_piece({
                    mesh = "common_stome_2", x = sx, y = sy, z = sz, rx = turn[1], ry = turn[2], rz = turn[3],
                    blend = Entity.BLEND_ADDITIVE, life = 10 / self.SUMMON_TPS,
                    update = function(p, u, entity)
                        local size = 3.0 * (1 - u)
                        entity:set_scale(size * unit, size * unit, size * unit)
                        entity:set_alpha(0.3 + 0.7 * u)
                    end,
                })
            end
        end,

        -- The crystals that Shiva throws (0x590cfa and 0x590f34, from tick 170 for 30 ticks): 4 per tick, from
        -- (0, -3000, 1000), each one in a random direction of the plane x = 0 (the velocity is a
        -- vector of 1500-1999 units that is turned by random angles) that moves for 15 ticks, with
        -- sparkles (KIRA_1, 0x591204) on its way. The size of the model and its direction are guesses.
        siva_crystals = function(self, t, e, tick, index)
            for _ = 1, 4 do
                -- The speed of the original (1500-1999 per tick) takes them out of the screen in a moment, so
                -- it is divided, and they go in any direction instead of only the plane x = 0, that is seen
                -- as a column.
                local r = (1500 + math.random(0, 499)) / 8
                local angle = math.random() * math.pi * 2
                local spread = math.random() * 2 - 1
                local flat = math.sqrt(1 - spread * spread)
                local vx, vy, vz = r * spread, r * flat * math.sin(angle), -r * flat * math.cos(angle)
                local start = {0, -3000, 1000}
                local function position(age) return {start[1] - vx * age, start[2] - vy * age, start[3] - vz * age} end
                local x, y, z = self:effect_to_scene(t, start)
                local unit = self.MODEL_UNIT * 12
                self:spawn_piece({
                    mesh = "common_partic_1", x = x, y = y, z = z, rx = 0, blend = Entity.BLEND_ADDITIVE,
                    life = 15 / self.SUMMON_TPS,
                    update = function(p, u, entity)
                        local age = u * 15
                        local sx, sy, sz = self:effect_to_scene(t, position(age))
                        entity:set_position(sx, sy, sz)
                        entity:set_scale(unit, unit, unit)
                        entity:set_alpha(1 - u * u)
                    end,
                })
                for age = 0, 14 do
                    if math.random() < 0.625 then
                        local p = position(age)
                        self:add_sprite(t, {
                            sheet = "siva_kira", frames = {0, 1, 2, 3, 4, 5, 6, 7}, birth = tick + age, billboard = true,
                            scale = 1.5, pos = {p[1] + math.random(-100, 99), p[2] + math.random(-100, 99), p[3] + math.random(-100, 99)},
                        })
                    end
                end
            end
        end,

        -- Shiva's smoke (0x591fd3 and 0x5921e1): the 16 frames of SMOKE_1, one per tick, where the ice
        -- appears (the exact places are a guess).
        siva_smoke = function(self, t, e, tick, index)
            local spots = {{0, 3000}, {2500, 2500}, {-3000, 2000}, {4800, 1200}}
            local frames = {}
            for i = 0, 15 do frames[#frames + 1] = i end
            for i = 1, 3 do
                self:add_sprite(t, {
                    sheet = "siva_smoke", frames = frames, birth = tick, billboard = true, scale = 3.0,
                    pos = {-spots[i][1], -300, 8000 - spots[i][2]},
                })
            end
        end,

        -- Shiva's spotlight (0x590781, SPOT_1 of 200 for 20 of raw size, scaled 3.0): a thin beam at the
        -- origin that fades in for 30 ticks, stays for 50 and fades out for 20.
        siva_spot = function(self, t, e, tick, index)
            local x, y, z = self:effect_to_scene(t, {0, 0, 0})
            local unit = self.MODEL_UNIT * 10 * 3
            self:spawn_piece({
                mesh = "common_spot_1", x = x, y = y, z = z, rx = 0, blend = Entity.BLEND_ADDITIVE,
                life = 100 / self.SUMMON_TPS,
                update = function(p, u, entity)
                    local age = u * 100
                    local alpha = age < 30 and age / 30 or (age < 80 and 1 or 1 - (age - 80) / 20)
                    entity:set_scale(unit, unit, unit)
                    entity:set_alpha(alpha)
                end,
            })
        end,

        -- The yellow flash at the base (0x59a408): a disc of radius 1600 that grows for 10 ticks (the
        -- size goes 0.1 more every tick) and then stays at 2.0 while its colour fades in 5 ticks.
        flash_disc = function(self, t, e, tick, index)
            t.flashes = t.flashes or {}
            t.flashes[#t.flashes + 1] = {
                birth = tick, life = 15, pos = {0, 0, 0},
                shape = function(age)
                    if age < 10 then return 1600 * age * 2048 / 5 / 4096, {1, 1, 0, 1} end
                    local v = ((age - 10) * -255 / 5 + 255) / 255
                    return 1600 * 2.0, {v, v, 0, 1}
                end,
            }
        end,

        -- Sparks at random places in front of the mountain: two every tick (0x59ac0d and 0x59acef).
        sparks = function(self, t, e, tick, index)
            for _ = 1, 2 do
                self:add_sprite(t, {
                    sheet = "spark", frames = {0, 1, 2, 3, 4, 5}, birth = tick,
                    -- The executable puts them all at a depth of 4096, which is a flat plane that is only
                    -- seen well from the front: they face the camera and have some depth too.
                    billboard = true,
                    pos = {math.random(-1280, 1279), math.random(-960, 959), 4096 + math.random(-1500, 1500)},
                    -- The sizes of the executable (0.25 to 1) looked too small: six times that.
                    scale = (1024 + math.random(0, 3071)) / 4096 * self.SPARK_SIZE,
                })
            end
        end,
    },

    --- Creates what the emitters of a summon have to create by this tick.
    update_emitters = function(self, t, tick)
        t.emitted = t.emitted or {}
        for i, e in ipairs(t.def.emitters or {}) do
            local last = t.emitted[i] or -1
            local now = math.min(math.floor(tick - e.start), e.ticks - 1)
            while last < now do
                last = last + 1
                self.EMITTERS[e.kind](self, t, e, e.start + last, last)
            end
            t.emitted[i] = last
        end
    end,

    --- Throws one rock of a mountain: it starts at a random vertex of the mesh, with a speed that
    -- goes outward and up, and spins (0x599bad and the code that creates it, in 0x598f74).
    spawn_debris = function(self, t, d, source)
        local current = source.current
        local v = math.random(0, #current / 3 - 1)
        local x, y, z = current[v * 3 + 1], current[v * 3 + 2], current[v * 3 + 3]
        local r = math.random(0, 255)
        -- The speed is in units per tick, whole numbers like in the original.
        local function whole(n) return n >= 0 and math.floor(n) or -math.floor(-n) end
        local vx, vy, vz = whole(x * r / 4096), -(math.random(0, 99) + 200), whole(z * r / 4096)
        local spin = {math.random(0, 15) - 8, math.random(0, 15) - 8, math.random(0, 15) - 8}
        local size = (1 + math.random()) * d.size * self.MODEL_UNIT
        local sx, sy, sz = self:effect_to_scene(t, {x, y, z})
        self:spawn_piece({
            mesh = d.mesh, x = sx, y = sy, z = sz, blend = Entity.BLEND_NORMAL, life = d.life / self.SUMMON_TPS,
            update = function(piece, u, entity)
                local age = u * d.life
                local px, py, pz = self:effect_to_scene(t, {x + vx * age, y + vy * age, z + vz * age})
                entity:set_position(px, py, pz)
                -- The angles go 16 units of 4096 per turn for every unit of speed, each tick.
                local to_degrees = 16 * 360 / 4096 * age
                entity:set_model_rotation(spin[1] * to_degrees, spin[2] * to_degrees, spin[3] * to_degrees)
                entity:set_scale(size, size, size)
            end,
        })
    end,

    --- Throws the rocks of a summon (def.debris) that have to be thrown by this tick.
    update_debris = function(self, t, tick)
        t.debris_done = t.debris_done or {}
        for i, d in ipairs(t.def.debris or {}) do
            local source = t.meshes ~= nil and t.meshes[d.from] or nil
            local last = t.debris_done[i] or -1
            local now = math.min(math.floor(tick - d.start), d.ticks - 1)
            if source ~= nil then
                while last < now do
                    last = last + 1
                    for _ = 1, d.per_tick do self:spawn_debris(t, d, source) end
                end
                t.debris_done[i] = last
            end
        end
    end,

    --- Strength of the shakes relative to the amplitudes of the original code. It was too strong at
    -- 1 (the amplitudes are for a screen of 320 x 240 and the camera moves instead of the image), so
    -- it's 10% of that, chosen by eye.
    SHAKE_SCALE = 0.1,

    --- Gets how far the screen is moved by a shake at this tick, in pixels of a screen of 320 x 240,
    -- like the original code that adds and takes the amplitude every other tick (0x5987b6).
    shake_offset = function(self, t, tick)
        local shake = t.def.shake
        if shake == nil or tick < shake[1][1] then return 0 end
        local amplitude = 0
        for _, point in ipairs(shake) do
            if tick >= point[1] then amplitude = point[2] end
        end
        if amplitude == 0 then return 0 end
        return ((math.floor(tick - shake[1][1]) % 2 == 0) and -amplitude or 0) * self.SHAKE_SCALE
    end,

    --- Converts a point of the space of an effect (original units, y down, z toward the targets) to
    -- the scene. The script of Shiva looks at (0, 0, 5000) at the end, the place of the targets
    -- when the origin is 5000 units from them.
    effect_to_scene = function(self, t, p)
        local k = self.MODEL_UNIT
        return t.origin.x + (t.f.x * p[3] + t.lateral.x * p[1]) * k,
          t.origin.y + (t.f.y * p[3] + t.lateral.y * p[1]) * k,
          t.origin.z - p[2] * k
    end,

    --- Goes back to the camera of the battle that was in use.
    restore_camera = function(self)
        local index = UiContainer.BattleUi ~= nil and UiContainer.BattleUi.camera_index or 0
        battle_manager:set_camera(index or 0)
    end,

    --- Moves the camera of an effect to where its script says at this moment.
    update_camera = function(self, t, tick)
        local target = math.floor(tick)
        while t.camera_tick < target do
            t.camera:tick()
            t.camera_tick = t.camera_tick + 1
        end
        if not t.camera.started then return end
        local cam, look = t.camera:get(tick - target)
        local cx, cy, cz = self:effect_to_scene(t, cam)
        local lx, ly, lz = self:effect_to_scene(t, look)
        local offset = self:shake_offset(t, tick)
        if offset ~= 0 then
            -- The image moves by a part of the screen, so the camera moves the same part of what it
            -- sees at the distance of the point it looks at.
            local dx, dy, dz = lx - cx, ly - cy, lz - cz
            local distance = math.sqrt(dx * dx + dy * dy + dz * dz)
            local rx, ry = dy, -dx
            local rl = math.sqrt(rx * rx + ry * ry)
            if rl > 0.0001 and distance > 0.0001 then
                rx, ry = rx / rl, ry / rl
                -- Up is the right vector crossed with the direction.
                local ux, uy, uz = ry * dz, -rx * dz, rx * dy - ry * dx
                local ul = math.sqrt(ux * ux + uy * uy + uz * uz)
                ux, uy, uz = ux / ul, uy / ul, uz / ul
                local height = 0.828 * distance
                local shift_x, shift_y = offset / 320 * height * 16 / 9, offset / 240 * height
                local mx, my, mz = -rx * shift_x + ux * shift_y, -ry * shift_x + uy * shift_y, uz * shift_y
                cx, cy, cz, lx, ly, lz = cx + mx, cy + my, cz + mz, lx + mx, ly + my, lz + mz
            end
        end
        -- The directions of the screen, for the sprites that face the camera.
        local vx, vy, vz = lx - cx, ly - cy, lz - cz
        local rx, ry = vy, -vx
        local rl = math.sqrt(rx * rx + ry * ry)
        local vl = math.sqrt(vx * vx + vy * vy + vz * vz)
        if rl > 0.0001 and vl > 0.0001 then
            rx, ry = rx / rl, ry / rl
            local ux, uy, uz = ry * vz, -rx * vz, rx * vy - ry * vx
            local ul = math.sqrt(ux * ux + uy * uy + uz * uz)
            t.view = {right = {rx, ry, 0}, up = {ux / ul, uy / ul, uz / ul}}
        end
        battle_manager:set_camera_position(cx, cy, cz, lx, ly, lz)
        t.camera_used = true
    end,

    --- Advances the summons. The hit and the end are always called, even if the creature couldn't
    -- be shown, so the battle doesn't wait for them.
    update_timelines = function(self, dt)
        local alive = {}
        for _, t in ipairs(self.timelines) do
            t.age = t.age + dt
            local tick = t.age * self.SUMMON_TPS
            local def = t.def
            -- The creature is there from appear to leave, fading in and out.
            if tick >= def.appear and tick < def.leave then
                if t.entity == nil and not t.creature_failed then
                    pcall(self.start_creature, self, t)
                    if t.entity == nil then t.creature_failed = true end
                end
                if t.entity ~= nil then
                    local fade = math.min(1, (tick - def.appear) / 8, (def.leave - tick) / 8)
                    pcall(function() t.entity:set_alpha(fade) end)
                    pcall(self.update_animations_of_creature, self, t, tick)
                    if def.height ~= nil then
                        -- The animations lift the model by the height of its hips (root_y, in the
                        -- units of the model), and the heights of the executable don't count that.
                        local scale_now = def.scales ~= nil and self.curve(def.scales, tick) or def.scale
                        local height = self.curve(def.height, tick) - (def.root_y or 0) * scale_now
                        pcall(function() t.entity:set_position(t.x, t.y, t.z + height * self.MODEL_UNIT) end)
                    end
                    if def.scales ~= nil then
                        local s = self.curve(def.scales, tick) * self.MODEL_UNIT
                        pcall(function() t.entity:set_scale(s, s, s) end)
                    end
                end
            elseif t.entity ~= nil then
                battle_manager:remove_effect_model(t.entity_name)
                t.entity, t.entity_name = nil, nil
            end
            if t.camera ~= nil then pcall(self.update_camera, self, t, tick) end
            if def.meshes ~= nil then self:safe(t, "meshes", self.update_meshes, self, t, tick) end
            if def.debris ~= nil then self:safe(t, "debris", self.update_debris, self, t, tick) end
            if def.emitters ~= nil then
                self:safe(t, "emitters", self.update_emitters, self, t, tick)
                self:safe(t, "sprites", self.update_sprites3d, self, t, tick)
                self:safe(t, "flashes", self.update_flashes, self, t, tick)
            end
            for i, event in ipairs(def.events or {}) do
                if tick >= event.tick and not t.events_done[i] then
                    t.events_done[i] = true
                    if event.effect ~= nil then pcall(self.play_effect_on_targets, self, t, event.effect) end
                    if event.sound ~= nil then pcall(function() audio_manager:play_sound(event.sound) end) end
                    if event.flash ~= nil then pcall(self.flash, self, event.flash.colour, event.flash.strength, event.flash.life) end
                end
            end
            if not t.hit_done and tick >= def.hit then
                t.hit_done = true
                t.on_hit()
            end
            if tick >= def.total then
                if not t.hit_done then t.hit_done = true t.on_hit() end
                if t.entity_name ~= nil then battle_manager:remove_effect_model(t.entity_name) end
                if t.camera_used then self:restore_camera() end
                pcall(self.remove_meshes, self, t)
                pcall(self.remove_sprites, self, t)
                t.on_done()
            else
                alive[#alive + 1] = t
            end
        end
        self.timelines = alive
    end,

    --- Removes the summons in progress, without finishing them.
    clear_timelines = function(self)
        for _, t in ipairs(self.timelines) do
            if t.entity_name ~= nil then battle_manager:remove_effect_model(t.entity_name) end
            if t.camera_used then pcall(self.restore_camera, self) end
            pcall(self.remove_meshes, self, t)
            pcall(self.remove_sprites, self, t)
        end
        self.timelines = {}
    end,

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
        if self.SUMMONS[attack.id] ~= nil then return false end
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

    --- Auras around the caster, the light tubes that appear while a character casts.
    --
    -- The original code (ff7.exe, 0x5c01a0) has four kinds, drawn by three functions of 15 ticks
    -- that use the models JO_P01 to JO_P03: quarter cylinders (4 of them make a tube) of radius 10
    -- and height 20 (11 and a flare for JO_P01). The colours are not in the code, they were told by
    -- looking at the original game: spells green, enemy skills pink, limits yellow and orange,
    -- summons yellow and red with sparkles.
    AURA_TPS = 15,
    AURA_TICKS = 15,
    --- Size of a model unit of the aura models, in scene units (a guess).
    AURA_UNIT = 0.06,
    AURA_SEGMENTS = 16,
    --- Extra factor for the radius only (the height keeps AURA_UNIT).
    AURA_RADIUS = 2,
    --- Layers that start one after another, and ticks between them.
    AURA_LAYERS = 4,
    AURA_LAYER_GAP = 3,
    AURAS = {
        -- Spells: JO_P02, grows and pulses.
        [0] = {texture = "battle_fx/common_jo_b02.png", tint = {1, 1, 1}, radius = 10, base = 10, height = 20},
        -- Limits: JO_P01, a flare that is narrow at the bottom.
        [1] = {texture = "battle_fx/common_jo_b03.png", tint = {1, 1, 1}, radius = 10, base = 4.5, height = 11},
        -- Enemy skills: JO_P03, shrinks toward the caster.
        [2] = {texture = "battle_fx/common_jo_b04.png", tint = {1, 1, 1}, radius = 10, base = 10, height = 20},
        -- Summons: JO_P02 with sparkles.
        [3] = {texture = "battle_fx/common_jo_b03.png", tint = {1, 0.55, 0.45}, radius = 10, base = 10, height = 20},
    },
    --- Sound of each kind (ids of audio/sounds, from the opcodes of ff7.exe: E6 13, E0 36, A4 496,
    -- A5 489, played by 0x430ca2).
    AURA_SOUNDS = {[0] = "13", [1] = "36", [2] = "496", [3] = "489"},
    auras = {},

    --- Gets the aura kind of an action: 0 spells, 1 limits, 2 enemy skills, 3 summons.
    aura_kind = function(self, action, summon)
        if summon then return 3 end
        local command = Game.Commands ~= nil and Game.Commands[action.command or -1] or nil
        local name = command ~= nil and string.lower(command.name or "") or ""
        if string.find(name, "e.skill", 1, true) then return 2 end
        if string.find(name, "limit", 1, true) then return 1 end
        return 0
    end,

    --- Starts the aura of a combatant that is casting.
    start_aura = function(self, kind, side, index)
        local def = self.AURAS[kind]
        if def == nil then do return end end
        pcall(function() audio_manager:play_sound(self.AURA_SOUNDS[kind]) end)
        for layer = 0, self.AURA_LAYERS - 1 do
            local name = battle_manager:create_dynamic_mesh(def.texture or "", 1)
            if name ~= nil and name ~= "" then
                self.auras[#self.auras + 1] = {
                    kind = kind, def = def, name = name, side = side, index = index, tick = -1,
                    age = -layer * self.AURA_LAYER_GAP / self.AURA_TPS,
                }
            end
        end
    end,

    --- Builds the tube of an aura for a tick.
    draw_aura = function(self, a, tick)
        local def = a.def
        local entity = UiContainer.BattleUi:get_entity(a.side, a.index)
        if entity == nil then do return end end
        local cx, cy, cz = entity:get_position()
        local c = tick
        local radius_scale, height_scale
        if a.kind == 2 then
            -- Closes in on the caster (the original counts down from 14).
            radius_scale = 0.4 + (14 - c) / 14 * 1.6
            height_scale = 1 + math.sin((14 - c) * 128 / 4096 * 2 * math.pi)
        else
            -- Comes out of the body and expands.
            radius_scale = 0.4 + c / 14 * 1.6
            height_scale = 1 + math.sin(c * 128 / 4096 * 2 * math.pi)
        end
        -- Fades in at the start and out at the end.
        local alpha = math.min(1, (c + 1) / 4) * math.min(1, (self.AURA_TICKS - c) / 5)
        local unit = self.AURA_UNIT
        local r_top = def.radius * radius_scale * unit * self.AURA_RADIUS
        local r_base = def.base * radius_scale * unit * self.AURA_RADIUS
        local h = def.height * height_scale * unit
        local n = self.AURA_SEGMENTS
        local out, k = {}, 0
        local tint = def.tint
        -- The models map the whole texture over each quarter of the tube, bright part at the bottom.
        local function vertex(angle, r, z, u, v)
            out[k + 1], out[k + 2], out[k + 3] = cx + r * math.cos(angle), cy + r * math.sin(angle), cz + z
            out[k + 4], out[k + 5] = u, v
            out[k + 6], out[k + 7], out[k + 8], out[k + 9] = tint[1] * alpha, tint[2] * alpha, tint[3] * alpha, 1
            k = k + 9
        end
        for i = 0, n - 1 do
            local a0 = i / n * 2 * math.pi + c * 0.3
            local a1 = (i + 1) / n * 2 * math.pi + c * 0.3
            local u0, u1 = 0.016 + 0.953 * (i % 4) / 4, 0.016 + 0.953 * (i % 4 + 1) / 4
            -- Both sides of every strip, the tube is seen from inside and outside.
            for _, pass in ipairs({1, 2}) do
                local p0, p1, q0, q1 = a0, a1, u0, u1
                if pass == 2 then p0, p1, q0, q1 = a1, a0, u1, u0 end
                vertex(p0, r_base, 0, q0, 0.992)
                vertex(p1, r_base, 0, q1, 0.992)
                vertex(p0, r_top, h, q0, 0.008)
                vertex(p1, r_base, 0, q1, 0.992)
                vertex(p1, r_top, h, q1, 0.008)
                vertex(p0, r_top, h, q0, 0.008)
            end
        end
        battle_manager:set_dynamic_mesh(a.name, out)
    end,

    --- Advances the auras.
    update_auras = function(self, dt)
        local alive = {}
        for _, a in ipairs(self.auras) do
            a.age = a.age + dt
            local tick = math.floor(a.age * self.AURA_TPS)
            if tick < 0 then
                alive[#alive + 1] = a
            elseif tick >= self.AURA_TICKS then
                battle_manager:remove_dynamic_mesh(a.name)
            else
                if tick ~= a.tick then
                    a.tick = tick
                    pcall(self.draw_aura, self, a, tick)
                    -- Summons: sparkles around the caster (SUMMON.SPT in the original).
                    if a.kind == 3 and a.age < 0.3 then
                        local x, y = self:get_position(a.side, a.index, 0.6)
                        if x ~= nil then
                            for _ = 1, 2 do
                                self:spawn_sprite({
                                    sheet = "sparkle", x = x + (math.random() - 0.5) * 60, y = y + (math.random() - 0.3) * 50,
                                    size = 14 + 10 * math.random(), life = 0.5, vy = -30,
                                    tint = math.random() < 0.5 and {1, 0.3, 0.15} or {1, 0.9, 0.3},
                                })
                            end
                        end
                    end
                end
                alive[#alive + 1] = a
            end
        end
        self.auras = alive
    end,

    --- Removes the auras.
    clear_auras = function(self)
        for _, a in ipairs(self.auras) do battle_manager:remove_dynamic_mesh(a.name) end
        self.auras = {}
    end,

    --- Removes every effect.
    clear = function(self)
        self:clear_auras()
        self:clear_timelines()
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
        -- What is made right after this follows the combatant when the camera moves.
        self.spawn_anchor = {side = side, index = index, height = height}
        return ux, uy
    end,

    --- Projects the point of an anchor ({side, index, height}) again, with the camera as it is now.
    project = function(self, anchor)
        local entity = UiContainer.BattleUi:get_entity(anchor.side, anchor.index)
        if entity == nil then return nil end
        local x, y, z = entity:get_position()
        local ux, uy = camera_manager:project_to_ui(x, y, z + anchor.height)
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
            anchor = self.spawn_anchor, ax = x, ay = y,
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
        self:update_timelines(dt)
        self:update_pieces(dt)
        self:update_sprites(dt)
        self:update_auras(dt)
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
                local shift_x, shift_y = 0, 0
                if n.anchor ~= nil then
                    local nx, ny = self:project(n.anchor)
                    if nx ~= nil then shift_x, shift_y = nx - n.ax, ny - n.ay end
                end
                widget:set_x(0, n.x + shift_x)
                widget:set_y(0, n.y - 45 * (n.age / n.life) + shift_y)
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
