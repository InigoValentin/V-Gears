--- Generates a control key for savegames.
--
-- It's a random character string, used to differenciate savegames.
generate_control_key = function()
    ControlKey = ""
    for i = 1, 8 do
        ControlKey = ControlKey .. string.char(math.random(97, 97 + 25))
    end
end

--- Saves the game.
--
-- @param slot The slot to save at (0-15).
-- @param force If false, exisiting game data will not be overwritten if control keys are different.
save_game = function(slot, force)
    savemap_manager:set_control_key(ControlKey)
    savemap_manager:set_window_colours(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12) -- TODO Actual data
    savemap_manager:set_money(Inventory.money)
    savemap_manager:set_game_time(123456) -- TODO Actual data
    savemap_manager:set_countdown_time(0) -- TODO Actual data
    for i = 1, 50 do
       if Inventory.key[i] == true then
           savemap_manager:set_key_item(i - 1, true)
       end
    end
    savemap_manager:set_party(Party[1] or -1, Party[2] or -1, Party[3] or -1)
    for i = 1, MAX_INVENTORY_SLOTS do
        if Inventory[i] ~= nil then
            savemap_manager:set_item(i - 1, Inventory[i].item, Inventory[i].quantity)
        end
    end
    for i = 1, MAX_MATERIA_SLOTS do
        if Materia[i] ~= nil then
            savemap_manager:set_materia(i - 1, Materia[i].id, Materia[i].ap)
            if Materia[i].id == Materia.ENEMY_SKILL_ID then
                for s = 1, 24 do
                    if Materia[i].skills[s] == true then
                        savemap_manager:set_e_skill_materia(i - 1, s, true)
                    end
                end
            end
        end
    end
    -- TODO: Materia stash, when implemented.
    local player = entity_manager:get_player_entity()
    local x, y, z = player:get_position()

    savemap_manager:set_location(x, y, z, player:get_move_triangle_id(), player:get_rotation(), System["MapChanger"].current_map_name, System["MapChanger"].location_name) -- TODO Actual data
    -- TODO Settings, when implemented
    for i = 0, 11 do
        if Characters[i] ~= nil and Characters[i].char_id ~= nil then
            local c = Characters[i]
            savemap_manager:set_character_info(i, c.char_id, c.name, c.enabled, c.locked, c.level, c.kills, c.back_row, c.exp, c.exp_to_next, c.limit.current, c.limit.bar, c.weapon.id, c.armor.id, c.accessory or -1)
            savemap_manager:set_character_stat(i, 0, c.stats.str.base, c.stats.str.bonus)
            savemap_manager:set_character_stat(i, 1, c.stats.vit.base, c.stats.vit.bonus)
            savemap_manager:set_character_stat(i, 2, c.stats.mag.base, c.stats.mag.bonus)
            savemap_manager:set_character_stat(i, 3, c.stats.spr.base, c.stats.spr.bonus)
            savemap_manager:set_character_stat(i, 4, c.stats.dex.base, c.stats.dex.bonus)
            savemap_manager:set_character_stat(i, 5, c.stats.lck.base, c.stats.lck.bonus)
            savemap_manager:set_character_stat(i, 6, c.stats.hp.base, c.stats.hp.current)
            savemap_manager:set_character_stat(i, 7, c.stats.mp.base, c.stats.mp.current)
            for l = 1, 4 do
                savemap_manager:set_character_limit_learned(i, l - 1, 0, c.limit.learned[l][1], c.limit.uses[l])
                savemap_manager:set_character_limit_learned(i, l - 1, 1, c.limit.learned[l][2], c.limit.uses[l])
            end
            for m = 0, 8 do
                if c.weapon.materia[m] ~= nil then
                    savemap_manager:set_character_materia(i, true, m, c.weapon.materia[m].id, c.weapon.materia[m].ap)
                    if c.weapon.materia[m].id == Materia.ENEMY_SKILL_ID then
                        for s = 1, 24 do
                            if c.weapon.materia[m].skills[s] == true then
                                savemap_manager:set_character_e_skill_materie(i, true, m, s, true)
                            end
                        end
                    end
                end
                if c.armor[m] ~= nil then
                    savemap_manager:set_character_materia(i, false, m, c.armor.materia[m].id, c.weapon.materia[m].ap)
                    if c.weapon.materia[m].id == Materia.ENEMY_SKILL_ID then
                        for s = 1, 24 do
                            if c.armor.materia[m].skills[s] == true then
                                savemap_manager:set_character_e_skill_materie(i, true, m, s, true)
                            end
                        end
                    end
                end
            end
            -- TODO: Character statuses
        end
    end
    for b = 0, Banks.COUNT do
        for a = 0, Banks.SIZE do
            if (Banks[b] ~= nil and Banks[b][a] ~= nil and Banks[b][a] ~= 0) then
                savemap_manager:set_data(b, a, tonumber(Banks[b][a]))
            end

        end
    end
    savemap_manager:save(slot, force or true)
end

--- Laods a game.
--
-- @param slot The slot to load (0-15).
load_game = function(slot)
    -- Populate banks
    for b = 0, Banks.COUNT do
        for a = 0, Banks.SIZE do
            Banks[b][a] = savemap_manager:get_slot_data(slot, b, a)
        end
    end
    -- TODO: Window colours
    Inventory.money = savemap_manager:get_slot_money(slot)
    -- TODO: Game time
    -- TODO: Countdown
    for p = 1, 3 do
        Party[p] = savemap_manager:get_slot_party_member(slot, p - 1)
        if Party[p] == -1 then
            Party[p] = nil
        end
    end
    for i = 0, MAX_INVENTORY_SLOTS do
        Inventory[i] = {item = 0, quantity = 0}
        Inventory[i].quantity = savemap_manager:get_slot_item_at_pos_qty(slot, i)
        if Inventory[i].quantity > 0 then
            Inventory[i].id = savemap_manager:get_slot_item_at_pos_id(slot, i)
        else
            Inventory[i] = nil
        end
    end
    for i = 0, MAX_MATERIA_SLOTS do
        Materia[i] = {id = -1, ap = 0}
        Materia[i].id = savemap_manager:get_slot_materia_at_pos_id(slot, i)
        if Materia[i].id >= 0 then
            Materia[i].ap = savemap_manager:get_slot_materia_at_pos_ap(slot, i)
            if Materia[i].id == Materia.ENEMY_SKILL_ID then
                Materia[i].ap = 0
                Materia[i].skills = {}
                for s = 1, 24 do
                    Materia[i].skills[s] = savemap_manager:is_slot_materia_at_pos_e_skill_learned(slot, i, s - 1)
                end
            end
        else
            Materia[i] = nil
        end
    end
    -- TODO: Stash (same as materia)
    for i = 0, MAX_KEY_SLOTS do
        Inventory.key[i] = savemap_manager:get_slot_key_item(slot, i)
    end
    -- Characters
    for c = 0, 11 do
        Characters[c].char_id = savemap_manager:get_slot_character_char_id(slot, c)
        if Characters[c].char_id == -1 then
            Characters[c] = nil
        else
            Characters[c] = {}
            Characters[c].name = savemap_manager:get_slot_character_name(slot, c)
            Characters[c].level = savemap_manager:get_slot_character_level(slot, c)
            Characters[c].enabled = savemap_manager:is_slot_character_enabled(slot, c)
            Characters[c].locked = savemap_manager:is_slot_character_locked(slot, c)
            Characters[c].kills = savemap_manager:get_slot_character_kills(slot, c)
            Characters[c].back_row = savemap_manager:is_slot_character_back_row(slot, c)
            Characters[c].exp = savemap_manager:get_slot_character_exp(slot, c)
            Characters[c].exp_to_next = savemap_manager:get_slot_character_exp_to_next(slot, c)
            Characters[c].stats = {}
            Characters[c].stats.str = {
                base = savemap_manager:get_slot_character_stat_base(slot, c, 0),
                bonus = savemap_manager:get_slot_character_stat_extra(slot, c, 0),
                total = 0
            }
            Characters[c].stats.vit = {
                base = savemap_manager:get_slot_character_stat_base(slot, c, 1),
                bonus = savemap_manager:get_slot_character_stat_extra(slot, c, 1),
                total = 0
            }
            Characters[c].stats.mag = {
                base = savemap_manager:get_slot_character_stat_base(slot, c, 2),
                bonus = savemap_manager:get_slot_character_stat_extra(slot, c, 2),
                total = 0
            }
            Characters[c].stats.spr = {
                base = savemap_manager:get_slot_character_stat_base(slot, c, 3),
                bonus = savemap_manager:get_slot_character_stat_extra(slot, c, 3),
                total = 0
            }
            Characters[c].stats.dex = {
                base = savemap_manager:get_slot_character_stat_base(slot, c, 4),
                bonus = savemap_manager:get_slot_character_stat_extra(slot, c, 4),
                total = 0
            }
            Characters[c].stats.lck = {
                base = savemap_manager:get_slot_character_stat_base(slot, c, 5),
                bonus = savemap_manager:get_slot_character_stat_extra(slot, c, 5),
                total = 0
            }
            Characters[c].stats.hp = {
                current = savemap_manager:get_slot_character_stat_extra(slot, c, 6),
                base = savemap_manager:get_slot_character_stat_base(slot, c, 6),
                max = 65535
            }
            Characters[c].stats.mp = {
                current = savemap_manager:get_slot_character_stat_extra(slot, c, 7),
                base = savemap_manager:get_slot_character_stat_base(slot, c, 7),
                max = 65535
            }
            Characters[c].limit = {}
            Characters[c].limit.current = savemap_manager:get_slot_character_limit_level(slot, c)
            Characters[c].limit.bar = savemap_manager:get_slot_character_limit_bar(slot, c)
            Characters[c].limit.learned = {}
            Characters[c].limit.uses = {}
            for l = 1, 4 do
                Characters[c].limit.learned[l] = {
                    [1] = savemap_manager:is_slot_character_limit_learned(slot, c, l, 0),
                    [2] = savemap_manager:is_slot_character_limit_learned(slot, c, l, 1)
                }
                Characters[c].limit.uses[l] = savemap_manager:get_slot_character_limit_uses(slot, c, l)
            end
            Characters[c].accessory = savemap_manager:get_slot_character_accessory_id(slot, c)
            Characters[c].weapon = {}
            Characters[c].armor = {}
            Characters[c].weapon.id = savemap_manager:get_slot_character_weapon_id(slot, c)
            Characters[c].armor.id = savemap_manager:get_slot_character_armor_id(slot, c)
            Characters[c].weapon.materia = {}
            Characters[c].armor.materia = {}
            for i = 0, 8 do
                Characters[c].weapon.materia[i] = {id = -1, ap = 0}
                Characters[c].armor.materia[i] = {id = -1, ap = 0}
                Characters[c].weapon.materia[i].id = savemap_manager:get_slot_character_materia_id(slot, c, true, i)
                Characters[c].armor.materia[i].id = savemap_manager:get_slot_character_materia_id(slot, c, false, i)
                if Characters[c].weapon.materia[i].id >= 0 then
                    Characters[c].weapon.materia[i].ap = savemap_manager:get_slot_character_materia_ap(slot, i, true, i)
                    if Characters[c].weapon.materia[i].id == Materia.ENEMY_SKILL_ID then
                        Characters[c].weapon.materia[i].ap = 0
                        Characters[c].weapon.materia[i].skills = {}
                        for s = 1, 24 do
                            Characters[c].weapon.materia[i].skills[s] = savemap_manager:is_slot_character_materia_e_skill_learned(slot, i, c, true, s - 1)
                        end
                    end
                else
                    Characters[c].weapon.materia[i] = nil
                end
                if Characters[c].armor.materia[i].id >= 0 then
                    Characters[c].armor.materia[i].ap = savemap_manager:get_slot_character_materia_ap(slot, i, false, i)
                    if Characters[c].armor.materia[i].id == Materia.ENEMY_SKILL_ID then
                        Characters[c].armor.materia[i].ap = 0
                        Characters[c].armor.materia[i].skills = {}
                        for s = 1, 24 do
                            Characters[c].armor.materia[i].skills[s] = savemap_manager:is_slot_character_materia_e_skill_learned(slot, i, c, false, s - 1)
                        end
                    end
                else
                    Characters[c].armor.materia[i] = nil
                end
            end
            -- TODO: Status
            Characters[c].status = {}
            Characters.recalculate_stats(c)
        end

    end
    export_character_names()
    -- TODO: Settings
    -- TODO: Location
    -- TODO: Update character stats
    -- TODO: Handle world map when implemented

    console("camera_free false")
    load_field_map_request(savemap_manager:get_slot_location_field(slot), "")
    System["MapChanger"].location_name = savemap_manager:get_slot_location_name(slot)
    System["MapChanger"].current_map_name = savemap_manager:get_slot_location_field(slot)
    -- TODO: For now, assume the PC is Cloub, but it must be saved.
    local player = nil
    while player == nil do
        entity_manager:set_player_entity("Cloud")
        player = entity_manager:get_player_entity()
        script:wait(0)
    end
    --local player = entity_manager:get_entity("Cloud")
    if player ~= nil then
        player:set_position(savemap_manager:get_slot_location_x(slot), savemap_manager:get_slot_location_y(slot), savemap_manager:get_slot_location_z(slot))
        player:set_move_triangle_id(savemap_manager:get_slot_location_triangle(slot))
        player:set_rotation(savemap_manager:get_slot_location_angle(slot))
        background2d:autoscroll_to_entity(player)
        local x, y, z = player:get_position()
    end
    MenuSettings.pause_available = true
end
