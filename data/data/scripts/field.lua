-- When V-Gears is run, export character names.
export_character_names()

--- Helper function that changes field maps through fade in and fadeout
--
-- @param map_name The target map name.
-- @param point_name Spawn point name in the target map for the PC.
load_field_map_request = function(map_name, point_name)
    if type(map_name) == "string" and map_name ~= "" then

        System["MapChanger"].map_name = map_name
        System["MapChanger"].point_name = point_name
        script:request(Script.SYSTEM, "MapChanger", "ffvii_field", 0)
    end
end

--- Starts a minigame (the MINIGAME opcode).
--
-- The minigames are not implemented yet: the game goes on as if the minigame had been played, and
-- jumps to the field that the original game goes to when it ends. The scripts of that field decide
-- from the state of the game what comes next (the bike of blackbgb leads to the fight against the
-- Motor Ball, because the script of the field set that state before starting the minigame).
--
-- @param game Number of the minigame.
-- @param map_name Field where the game goes on.
-- @param point_name Point of that field where the player is placed.
minigame = function(game, map_name, point_name)
    print("The minigame " .. tostring(game) .. " is not implemented: skipping it.")
    load_field_map_request(map_name, point_name)
end

dispatch_menu = function(menu_id, parameter)
    menu_id = tonumber(menu_id)
    parameter = tonumber(parameter)
    if not menu_id or not parameter then
        return
    end

    if menu_id == 5 then
        ending_credits()
    elseif menu_id == 6 then
        if parameter == 64 then
            name_chocobo()
        else
            name_character(parameter)
        end
    elseif menu_id == 7 then
        if parameter == 1 then
            Party.split_teams(3)
        elseif parameter == 2 then
            Party.split_teams(2)
        else
            Party.choose_party()
        end
    elseif menu_id == 8 then
        open_shop(parameter)
    elseif menu_id == 9 then
        open_menu("party")
    elseif menu_id == 14 then
        open_menu("save")
    elseif menu_id == 15 then
        Party.steal_materia()
    elseif menu_id == 17 then
        Characters.unequip_materia(parameter)
    elseif menu_id == 18 then
        Characters.remove_materia(parameter)
    elseif menu_id == 19 then
        Characters.restore_materia(parameter)
    end
end

--- Makes an entity the player entity and places it where the previous map sent it to.
--
-- This is the entry point of the map (when it came from another field) or the field entry of the
-- world map. Fields that don't have a PC opcode for the player need it too, or the player would
-- stay at the origin of the field, out of its walkmesh.
--
-- @param entity_name The name of the entity that the player controls.
place_player_at_map_entry = function(entity_name)
    if System.MapChanger.point_name ~= "" then
        local point = entity_manager:get_entity_point(System.MapChanger.point_name)
        if point ~= nil then
            local x, y, z = point:get_position()
            local rotation = point:get_rotation()
            local player = entity_manager:get_entity(entity_name)
            if player ~= nil then
                player:set_position(x, y, z)
                player:set_rotation(rotation)
                player:set_solid(true)
                player:set_visible(true)
            else
                print("place_player_at_map_entry: there is no entity '" .. entity_name .. "'.")
            end
        end
    end

    entity_manager:set_player_entity(entity_name)
    local world_map_entry = System.MapChanger.world_map_entry
    if world_map_entry then
        entity_manager:place_player_at_field_entry(
            world_map_entry.x, world_map_entry.y,
            world_map_entry.triangle, world_map_entry.direction
        )
        System.MapChanger.world_map_entry = nil
    end
end

--- Assigns an entity to a playable character.
--
-- @param entity_name The entity name.
-- @param character_id Id of the character to assign.
set_entity_to_character = function(entity_name, character_id)
    if character_id == Party[1] and entity_name ~= "" then
        place_player_at_map_entry(entity_name)
    end
    entity_manager:set_entity_to_character(entity_name, character_id)
    -- The model of a playable character is shown only if it is the leader of the party. The other
    -- members are not drawn following the leader (join_party hides them too), and the scenes show
    -- their models when they need them. Models are visible by default, so the ones of the
    -- characters that are not the leader, such as Cid in mds6_1 or Aerith in mrkt2 after the
    -- first scene, have to be hidden here. With no party at all (a new game that has not set it
    -- yet) nothing is hidden.
    if character_id ~= Party[1] and entity_name ~= "" and Party[1] ~= nil then
        local entity = entity_manager:get_entity(entity_name)
        if entity ~= nil then
            entity:set_visible(false)
            entity:set_solid(false)
            entity:set_talkable(false)
        end
    end
end

--- Joins the party members in the field.
--
-- @param speed Speed at which the characters join
join_party = function(speed)
    local player = entity_manager:get_player_entity()
    if (player  == nil) then
        do return end
    end
    local x, y, z = player:get_position()
    for c = 2, 3 do
        if Party[c] ~= nil then
            local character = entity_manager:get_entity_from_character_id(Party[c])
            --print("    Name: " .. character:get_name())
            if character ~= nil then
                local cur_speed = character:get_move_auto_speed()
                character:set_solid(false)
                -- Approximated speed, good enough for now.
                -- TODO: Calculate speed based on time, as the orignal opcode.
                character:set_move_auto_speed(speed / 16)
                character:move_to_position(x, y)
                character:move_sync()
                character:set_move_auto_speed(cur_speed)
                character:set_visible(false)
                character:set_talkable(false)
            end
        end
    end
end

--- Splits the party members in the field.
--
-- @param ax_addr Bank for ax, or zero if ax is specified as a literal value.
-- @param ay_addr Bank for ay, or zero if ay is specified as a literal value.
-- @param ar_addr Bank for ar, or zero if ar is specified as a literal value.
-- @param bx_addr Bank for bx, or zero if bx is specified as a literal value.
-- @param by_addr Bank for by, or zero if by is specified as a literal value.
-- @param br_addr Bank for br, or zero if by is specified as a literal value.
-- @param ax X-coordinate of the second character in the party after the split, or address for the value if ax_addr is non-zero.
-- @param ay Y-coordinate of the second character in the party after the split, or address for the value if ax_addr is non-zero.
-- @param ar Direction the second character faces after the split, or address for the value if ar_addr is non-zero.
-- @param bx X-coordinate of the third character in the party after the split, or address for the value if bx_addr is non-zero.
-- @param by Y-coordinate of the third character in the party after the split, or address for the value if bx_addr is non-zero.
-- @param br Direction the third character faces after the split, or address for the value if br_addr is non-zero.
-- @param speed Speed at which the characters split.
split_party = function(ax, ay, ar, bx, by, br, speed)
    local player = entity_manager:get_player_entity()
    local x, y, z = player:get_position()
    for c = 2, 3 do
        if Party[c] ~= nil then
            local character = entity_manager:get_entity_from_character_id(Party[c])
            if character ~= nil then
                local cur_speed = character:get_move_auto_speed()
                -- Approximated speed, good enough for now.
                -- TODO: Calculate speed based on time, as the orignal opcode.
                character:set_move_auto_speed(speed / 16)
                --if character:is_visible() == false then
                    -- In theory, a member is never visible before a split, but some maps
                    -- are wrong. In these cases, don't change position and move the entity
                    -- from it's current position.
                    character:set_position(x, y, z)
                    character:set_visible(true)
                --end
                character:set_solid(false)
                if c == 2 then
                    character:move_to_position(ax, ay)
                elseif c == 3 then
                    character:move_to_position(bx, by)
                end
                character:move_sync()
                if c == 2 then
                    character:set_rotation(ar)
                elseif c == 3 then
                    character:set_rotation(br)
                end
                character:set_move_auto_speed(cur_speed)
                character:set_solid(true)
                character:set_talkable(true)
            end
        end
    end
end

--- Locks or unlocks player movement and menu access.
-- @param lock True to lock, false to unlock.
player_lock = function(lock)
    entity_manager:player_lock(lock)
    MenuSettings.available = (lock == false)
end

--- Locks or unlocks the menu button (MENU2), with no effect on the player.
-- @param lock True to lock, false to unlock.
menu_lock = function(lock)
    MenuSettings.available = (lock == false)
end

--- Sets a bit variable on (1).
--
-- @param bank Memory bank (0-index, as in game).
-- @param variable Bank address (0-index, as in game).
-- @param bit Address bit (0-index, as in game).
bit_on = function(bank, variable, bit)
    BitBanks[bank][variable][bit] = 1
end

--- Sets a bit variable off (0).
--
-- @param bank Memory bank (0-index, as in game).
-- @param variable Bank address (0-index, as in game).
-- @param bit Address bit (0-index, as in game).
bit_off = function(bank, variable, bit)
    BitBanks[bank][variable][bit] = 0
end

--- Retrieves a bit variable.
--
-- @param bank Memory bank (0-index, as in game).
-- @param variable Bank address (0-index, as in game).
-- @param bit Address bit (0-index, as in game).
-- @return 1 or 0
bit = function(bank, variable, bit)
    return BitBanks[bank][variable][bit]
end

--- Stores the coordinates and walkmesh triangle in memory.
--
-- Stores the values as in the original game, without scaling.
-- @param bx Bank at which to store the X coordinate.
-- @param by Bank at which to store the Y coordinate.
-- @param bz Bank at which to store the Z coordinate.
-- @param bt Bank at which to store the triangle.
-- @param id The entity ID.
-- @param ax Address at which to store the X coordinate.
-- @param ay Address at which to store the Y coordinate.
-- @param az Address at which to store the Z coordinate.
-- @param at Address at which to store the triangle.
-- @param scale Map scale to multiply values
-- The coordinates are rounded: the walkmesh is saved with six digits, so a height of 193 units comes
-- as 192.9997 once scaled, and the scripts compare it with the exact value (blin68_1 asks for it to
-- talk to a character).
axyzi = function(bx, by, bz, bt, id, ax, ay, az, at, scale)
    local entity = entity_manager:get_entity_from_index(id)
    if entity == nil then
        do return end
    end
    local x, y, z = entity:get_position()
    local t = entity:get_move_triangle_id()
    Banks[bx][ax] = math.floor(x * scale + 0.5)
    Banks[by][ay] = math.floor(y * scale + 0.5)
    Banks[bz][az] = math.floor(z * scale + 0.5)
    Banks[bt][at] = t
    return 0
end

--- Stores the X and Y position of an entity in two variables.
--
-- @param bx Bank for X.
-- @param by Bank for Y.
-- @param id Index of the entity in the field.
-- @param ax Address at which to store the X coordinate.
-- @param ay Address at which to store the Y coordinate.
-- @param scale Map scale to multiply values
getaxy = function(bx, by, id, ax, ay, scale)
    local entity = entity_manager:get_entity_from_index(id)
    if entity == nil then
        do return end
    end
    local x, y = entity:get_position()
    Banks[bx][ax] = math.floor(x * scale + 0.5)
    Banks[by][ay] = math.floor(y * scale + 0.5)
    return 0
end

--- Shows the ending credits.
ending_credits = function()
    -- TODO: Implement. Probably in a looooong loooong time.
end

--- Displays a window where a name can be assigned to a character.
--
-- @param id ID of the character to name.
name_character = function(id)
    -- TODO: Implement.
    if Characters[id] == nil then
        print("Tried to rename invalid character with ID " .. tostring(id))
        return 0
    end
    -- While the name menu is not enabled, the character just gets its default name.
    if not UiContainer.NameMenu.enabled then
        UiContainer.NameMenu:apply_default_name(id)
        return 0
    end
    UiContainer.NameMenu.id = id
    UiContainer.NameMenu.chocobo = false
    script:request(Script.UI, "NameMenu", "show", 0)
end

--- Displays a window where a name can be assigned to a chocobo.
name_chocobo = function()
    -- TODO: Implement.
end

--- Automatically opens a menu.
--
-- @param menu The name of the menu.
open_menu = function(menu)
    if menu == "party" then
        -- The button that asked for the menu is still down: wait for it, or it would close the menu.
        while entity_manager:is_key_on(16 + 64) do script:wait(0) end
        script:request_end_sync(Script.UI, "MainMenu", "show", 0)
        while ui_manager:get_widget("MainMenu"):is_visible() do script:wait(0) end
        -- The same with the button that closed it: the script would see it as a new request.
        while entity_manager:is_key_on(16 + 64) do script:wait(0) end
    elseif menu == "save" then
        -- The button that asked for the menu (a script may read it) is down: wait for it.
        while entity_manager:is_key_on(32 + 64) do script:wait(0) end
        UiContainer.SaveMenu.operation = "save"
        UiContainer.SaveMenu.standalone = true
        script:request_end_sync(Script.UI, "SaveMenu", "show", 0)
        while UiContainer.current_menu == "save" do script:wait(0) end
        while entity_manager:is_key_on(32 + 64) do script:wait(0) end
    end
end

--- Opens a shop.
--
-- @param id Shop ID.
open_shop = function(id)
    -- The button that talked to the shop (a script may read it) is down: wait for it, or it would
    -- act in the menu.
    while entity_manager:is_key_on(32 + 64) do script:wait(0) end
    UiContainer.ShopMenu.shop_id = tonumber(id)
    script:request_end_sync(Script.UI, "ShopMenu", "show", 0)
    while UiContainer.current_menu == "shop" do script:wait(0) end
    while entity_manager:is_key_on(32 + 64) do script:wait(0) end
end

--- Plays one of the tracks assigned to the field.
--
-- @param id ID of the track in the field.
play_map_music = function(id)
    local track_id = audio_manager:get_track_id(id)
    -- A track of the field with no music (-1, an empty AKAO frame of the field file) is silence: the
    -- scenes use it between two tracks, and some fields (trackin) have no music at all.
    if track_id ~= -1 then
        audio_manager:play_music(tostring(track_id))
    else
        audio_manager:stop_music()
    end
end

--- Changes the music of the field to one of its tracks (CMUSC).
--
-- The original opcode can fade the current music out and the new one in. Here the change is at once:
-- a track that is changed without fades (the usual jingle of an inn) is played once, so the script
-- that waits for the music to end (CHMST) goes on; one with fades is played looped.
-- TODO: Implement the fades.
--
-- @param id ID of the track in the field.
-- @param fade_out Fade out parameter of the original opcode (20 means no fade).
-- @param fade_in Fade in parameter of the original opcode (0 means no fade).
change_map_music = function(id, fade_out, fade_in)
    local track_id = audio_manager:get_track_id(id)
    -- A track with no music is silence (see play_map_music).
    if track_id == -1 then
        audio_manager:stop_music()
        return
    end
    if fade_in == 0 then
        audio_manager:play_music_once(tostring(track_id))
    else
        audio_manager:play_music(tostring(track_id))
    end
end

--- Sets the map name
--
-- It's not set from a literal text, but from a dialog identifier.
--
-- @param text_id ID of the dialog text that holds the map name.
set_map_name = function(text_id)
    dialog:set_map_name(text_id)
    System["MapChanger"].location_name = dialog:get_map_name()
end

--- Shows a choice dialog and stores the result in the selected bank address.
--
-- @param window_id ID of the window to show the dialog in.
-- @param message_id ID of the text to show in the window.
-- @param first First selectable line.
-- @param last Last selectable line.
-- @param bank Data bank to store the selected line in.
-- @param address Addres of the bank at which to store the selected line in.

dialog_ask = function(window_id, message_id, first, last, bank, address)
    dialog:set_clickable(window_id, true)
    dialog:set_cursor(window_id, first, last)
    dialog:dialog_set_text(window_id, message_id)
    dialog:dialog_wait_for_close(window_id)
    Banks[bank][address] = dialog:get_cursor(window_id)
end

--- Utility to change fields and enter battles.
System["MapChanger"] = {

    --- ID of the map to change to.
    map_name = "",

    --- Point of the map to change to.
    point_name = "",

    world_map_entry = nil,

    --- ID of the current field.
    current_map_name = "",

    --- Location name, to display in the main menu and in save slots.
    location_name = "",

    --- Jumps to a new map.
    ffvii_field = function(self)
        if self.map_name ~= "" then
            local target = self.map_name
            player_lock(true) -- disable menu and pc movement while load map
            script:request_end_sync(Script.UI, "Fade", "fade_out", 0)
            -- The fields go to the world map as "wm<n>", n being the FIELD.TBL record of the exit.
            local world_record = tonumber(string.match(target, "^wm(%d+)$"))
            if world_record then
                enter_world_map(world_record, self.point_name)
            else
                map(target)
            end
            player_lock(false) -- enable menu and pc movement after load map
            self.current_map_name = target
            -- A field can ask for another change as soon as it starts (blin60_1 goes on to
            -- blin60_2 at once). That request must not be erased here.
            if self.map_name == target then self.map_name = "" end
            script:request_end_sync(Script.UI, "Fade", "fade_in", 0)
        end
        return 0
    end,
    
    

    --- Starts a battle.
    --
    -- @todo Implement.
    ffvii_battle = function(self)
        if self.map_name ~= "" then
            player_lock(true) -- disable menu and pc movement while load map
            script:request_end_sync(Script.UI, "Fade", "fade_out", 0)
            map(self.map_name)
            -- load battle ui
            -- load player entity
            --self.map_name = ""
            script:request_end_sync(Script.UI, "Fade", "fade_in", 0)
        end

        return 0
    end,
}


--- Runs a script of the entity of a character (the PREQ, PRQSW and PRQEW opcodes).
--
-- @param character Slot of the party (0 is the leader), not the ID of a character: the scripts of a
-- scene with another party (Aerith leading the party, for example) ask for the slot, and the
-- request goes to the entity of the character that is in it (the one given with the PC opcode).
-- @param script_name Name of the script function.
-- @param priority Priority of the script.
-- @param mode "request" does not wait, "start_sync" waits for the script to start and "end_sync" for
-- it to end.
party_request = function(character, script_name, priority, mode)
    local character_id = Party[character + 1]
    local entity = character_id ~= nil and entity_manager:get_entity_from_character_id(character_id) or nil
    if entity == nil then
        print("party_request: there is no entity for the party slot " .. tostring(character) .. ".")
        return
    end
    local name = entity:get_name()
    if mode == "start_sync" then
        script:request_start_sync(Script.ENTITY, name, script_name, priority)
    elseif mode == "end_sync" then
        script:request_end_sync(Script.ENTITY, name, script_name, priority)
    else
        script:request(Script.ENTITY, name, script_name, priority)
    end
end
