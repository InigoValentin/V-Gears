save_game = function(slot)
    savemap_manager:set_control_key("CONTROL_EXAMPLE") -- TODO Actual data
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
    savemap_manager:set_location(10, 20, -1, 3, 120, "FIELD_ID", dialog:get_map_name()) -- TODO Actual data
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
    for b = 0, 14 do
        for a = 0, 254 do
            if (Banks[b] ~= nil and Banks[b][a] ~= nil and Banks[b][a] ~= 0) then
                savemap_manager:set_data(b, a, tonumber(Banks[b][a]))
            end

        end
    end
    savemap_manager:save(slot, false)
end
