-- When V-Gears is run, export character names.
export_character_names()

--- Helper function that changes field maps through fade in and fadeout
--
-- @param map_name The target map name.
-- @param point_name Spawn point name in the target map for the PC.
load_field_map_request = function(map_name, point_name)
    if type(map_name) == "string" and map_name ~= "" then
        System.MapChanger.map_name = map_name
        System.MapChanger.point_name = point_name
        script:request(Script.SYSTEM, "MapChanger", "ffvii_field", 0)
    end
end

--- Assigns an entity to a playable character.
--
-- @param entity_name The entity name.
-- @param character_id Id of the character to assign.
set_entity_to_character = function(entity_name, character_id)
    if character_id == FFVII.Party[1] and entity_name ~= "" then
        if System.MapChanger.point_name ~= "" then
            local point = entity_manager:get_entity_point(System.MapChanger.point_name)
            if point ~= nil then
                local x, y, z = point:get_position()
                local rotation = point:get_rotation()
                local player = entity_manager:get_entity(entity_name)
                player:set_position(x, y, z)
                player:set_rotation(rotation)
                player:set_solid(true)
                player:set_visible(true)
            end
        end

        entity_manager:set_player_entity(entity_name)
    end
    entity_manager:set_entity_to_character(entity_name, character_id)
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
        if FFVII.Party[c] ~= nil then
            local character = entity_manager:get_entity_from_character_id(FFVII.Party[c])
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
        if FFVII.Party[c] ~= nil then
            local character = entity_manager:get_entity_from_character_id(FFVII.Party[c])
            if character ~= nil then
                local cur_speed = character:get_move_auto_speed()
                -- Approximated speed, good enough for now.
                -- TODO: Calculate speed based on time, as the orignal opcode.
                character:set_move_auto_speed(speed / 16)
                if character:is_visible() == false then
                    -- In theory, a member is never visible before a split, but some maps
                    -- are wrong. In these cases, don't change position and move the entity
                    -- from it's current position.
                    character:set_position(x, y, z)
                    character:set_visible(true)
                end
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
    FFVII.MenuSettings.available = (lock == false)
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
axyzi = function(bx, by, bz, bt, id, ax, ay, az, at, scale)
    local entity = entity_manager:get_entity_from_index(id)
    if entity == nil then
        do return end
    end
    local x, y, z = entity:get_position()
    local t = entity:get_move_triangle_id()
    Banks[bx][ax] = math.floor(x * scale)
    Banks[by][ay] = math.floor(y * scale)
    Banks[bz][az] = math.floor(z * scale)
    Banks[bt][at] = t
    return 0
end

--- Utility to change fields and enter battles.
System["MapChanger"] = {
    map_name = "",
    point_name = "",

    --- Jumps to a new map.
    ffvii_field = function(self)
        if self.map_name ~= "" then
            player_lock(true) -- disable menu and pc movement while load map
            script:request_end_sync(Script.UI, "Fade", "fade_out", 0)
            map(self.map_name)
            player_lock(false) -- enable menu and pc movement after load map
            self.map_name = ""
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
            self.map_name = ""
            script:request_end_sync(Script.UI, "Fade", "fade_in", 0)
        end

        return 0
    end,
}
