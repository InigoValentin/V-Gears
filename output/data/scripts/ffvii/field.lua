-- helper function that changes field maps through fadein and fadeout
load_field_map_request = function( map_name, point_name )
    if type( map_name ) == "string" and map_name ~= "" then
        System.MapChanger.map_name = map_name
        System.MapChanger.point_name = point_name
        script:request( Script.SYSTEM, "MapChanger", "ffvii_field", 0 )
    end
end

set_entity_to_character = function( entity_name, character_name )
    if character_name == FFVII.Party[ 1 ] and entity_name ~= "" then
        if System.MapChanger.point_name ~= "" then
            local point = entity_manager:get_entity_point( System.MapChanger.point_name )
            if point ~= nil then
                local x, y, z = point:get_position()
                local rotation = point:get_rotation()
                local player = entity_manager:get_entity( entity_name )
                player:set_position( x, y, z )
                player:set_rotation( rotation )
                player:set_solid( true )
                player:set_visible( true )
            end
        end

        entity_manager:set_player_entity( entity_name )
    end
    entity_manager:set_entity_to_character(entity_name, character_name)
end

--[[
 Joins the party members in the field.
 @param speed Speed at which the characters join
]]
join_party = function(speed)
    local player = entity_manager:get_player_entity()
    if (player  == nil) then
        do return end
    end
    local x, y, z = player:get_position()
    for c = 2, 3 do
        if FFVII.Party[c] ~= nil then
            local character = entity_manager:get_entity_from_character_id(FFVII.Party[c])
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

--[[
 Splits the party members in the field.
 @param ax_addr Bank for ax, or zero if ax is specified as a literal value.
 @param ay_addr Bank for ay, or zero if ay is specified as a literal value.
 @param ar_addr Bank for ar, or zero if ar is specified as a literal value.
 @param bx_addr Bank for bx, or zero if bx is specified as a literal value.
 @param by_addr Bank for by, or zero if by is specified as a literal value.
 @param br_addr Bank for br, or zero if by is specified as a literal value.
 @param ax X-coordinate of the second character in the party after the split, or address for the value if ax_addr is non-zero.
 @param ay Y-coordinate of the second character in the party after the split, or address for the value if ax_addr is non-zero.
 @param ar Direction the second character faces after the split, or address for the value if ar_addr is non-zero.
 @param bx X-coordinate of the third character in the party after the split, or address for the value if bx_addr is non-zero.
 @param by Y-coordinate of the third character in the party after the split, or address for the value if bx_addr is non-zero.
 @param br Direction the third character faces after the split, or address for the value if br_addr is non-zero.
 @param speed Speed at which the characters split.
]]
split_party = function(ax_addr, ay_addr, ar_addr, bx_addr, by_addr, br_addr, ax, ay, ar, bx, by, br, speed)
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
                character:set_position(x, y, z)
                character:set_visible(true)
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

--[[
 Locks or unlocks player movement and menu access.
 @param lock True to lock, false to unlock.
]]
player_lock = function( lock )
    entity_manager:player_lock( lock )
    FFVII.MenuSettings.available = ( lock == false )
end

--[[
 Sets a bit variable on (1).
 @param bank Memory bank (0-index, as in game).
 @param variable Bank address (0-index, as in game).
 @param bit Address bit (0-index, as in game).
]]
bit_on = function(bank, variable, bit)
    FFVII.BitBanks[bank + 1][variable + 1][bit + 1] = 1
end

--[[
 Sets a bit variable off (0).
 @param bank Memory bank (0-index, as in game).
 @param variable Bank address (0-index, as in game).
 @param bit Address bit (0-index, as in game).
]]
bit_off = function(bank, variable, bit)
    FFVII.BitBanks[bank + 1][variable + 1][bit + 1] = 0
end

--[[
 Retrieves a bit variable.
 @param bank Memory bank (0-index, as in game).
 @param variable Bank address (0-index, as in game).
 @param bit Address bit (0-index, as in game).
 @return 1 or 0
]]
bit = function(bank, variable, bit)
    return FFVII.BitBanks[bank + 1][variable + 1][bit + 1]
end

--[[
 Checks if a key is being pressed.
 @param key_code The code of the key to check.
 @return True if the key is being pressed, false otherwise.
 @todo Implement.
]]
KeyOn = function(key_code)
    return true;
end

System[ "MapChanger" ] = {
    map_name = "",
    point_name = "",

    ffvii_field = function( self )
        if self.map_name ~= "" then
            player_lock( true ) -- disable menu and pc movement while load map
            script:request_end_sync( Script.UI, "Fade", "fade_out", 0 )
            map( self.map_name )
            player_lock( false ) -- enable menu and pc movement after load map
            self.map_name = ""
            script:request_end_sync( Script.UI, "Fade", "fade_in", 0 )
        end

        return 0
    end,

    ffvii_battle = function( self )
        if self.map_name ~= "" then
            player_lock( true ) -- disable menu and pc movement while load map
            script:request_end_sync( Script.UI, "Fade", "fade_out", 0 )
            map( self.map_name )
-- load battle ui
-- load player entity
            self.map_name = ""
            script:request_end_sync( Script.UI, "Fade", "fade_in", 0 )
        end

        return 0
    end,
}
