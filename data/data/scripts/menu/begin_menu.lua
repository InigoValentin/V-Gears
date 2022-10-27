if UiContainer == nil then UiContainer = {} end



UiContainer.BeginMenu = {
    position = 1,
    position_total = 6,



    on_start = function( self )
        local cursor = ui_manager:get_widget( "BeginMenu.Container.Cursor" )
        cursor:set_default_animation( "Position" .. self.position )
        return 0
    end,



    on_button = function( self, button, event )
        if ui_manager:get_widget( "BeginMenu" ):is_visible() ~= false then
            local cursor = ui_manager:get_widget( "BeginMenu.Container.Cursor" )

            if button == "Enter" and event == "Press" then
                if self.position == 1 then
                    load_field_map_request( "md1stin", "" )
                    console( "camera_free false" )
                    --console( "debug_walkmesh true" )
                    script:request_end_sync( Script.UI, "BeginMenu", "hide", 0 )
                    FFVII.MenuSettings.pause_available = true
                elseif self.position == 2 then
                    script:request_end_sync( Script.UI, "BeginMenu", "hide", 0 )
                    console( "camera_free true" )
                    console( "debug_walkmesh true" )
                    map( "test_3" )
                    FFVII.MenuSettings.pause_available = true
                elseif self.position == 3 then
                    -- NMKIN_3 START
                    --[[FFVII.Data.progress_game = 23
                    load_field_map_request("nmkin_3", "")
                    console( "camera_free false" )
                    console( "debug_walkmesh true" )
                    script:wait(1)
                    script:request_end_sync( Script.UI, "BeginMenu", "hide", 0 )
                    entity_manager:get_entity("Cloud"):set_position(-1.090750, 15.310307, 17.281244)
                    entity_manager:set_player_entity("Cloud")
                    script:wait(1)
                    entity_manager:get_player_entity():set_position(-1.093750, 15.320307, 17.281244)
                    entity_manager:get_entity("Cloud"):jump_to_position(0.46875, 14.9922, -1, 1, 42) -- Triangle 42, 2 steps.
                    entity_manager:get_entity("Cloud"):jump_sync()
                    entity_manager:player_lock(false)
                    FFVII.MenuSettings.pause_available = true]]
                    -- NMKIN_3 END
                    -- NMKIN_4 START
                    FFVII.Data.progress_game = 14
                    --load_field_map_request("nmkin_4", "")
                    map("nmkin_4")
                    console("camera_free false")
                    console("debug_walkmesh true")
                    script:wait(1.5)
                    script:request_end_sync( Script.UI, "BeginMenu", "hide", 0 )
                    entity_manager:get_entity("Cloud"):set_position(-0.820312, 0.500000, 17.281244)
                    entity_manager:set_player_entity("Cloud")
                    FFVII.set_party(0, 1, 4)
                    script:wait(0.5)
                    entity_manager:get_player_entity():set_position(-0.820312, 0.500000, 17.281244)
                    entity_manager:get_entity("Cloud"):jump_to_position(-0.800312, 0.500000, -1, 1) -- Triangle 42, 2 steps.
                    entity_manager:get_entity("Cloud"):jump_sync()
                    entity_manager:player_lock(false)
                    print("TRIANGLE " .. entity_manager:get_entity("Cloud"):get_move_triangle_id())
                    FFVII.MenuSettings.pause_available = true
                    -- NMKIN_4 END
                elseif self.position == 4 then
                    script:request_end_sync( Script.UI, "BeginMenu", "hide", 0 )
                    console( "camera_free true" )
                    console( "debug_walkmesh true" )
                    map( "test_2" )
                    FFVII.MenuSettings.pause_available = true
                elseif self.position == 5 then
                    script:request_end_sync( Script.UI, "BeginMenu", "hide", 0 )
                    console( "camera_free true" )
                    world_map_module:init()
                elseif self.position == 6 then
                    load_field_map_request( "startmap", "" )
                    console( "camera_free false" )
                    console( "debug_walkmesh false" )
                    script:request_end_sync( Script.UI, "BeginMenu", "hide", 0 )
                    FFVII.MenuSettings.pause_available = true
                end
            elseif button == "Down" then
                self.position = self.position + 1
                if self.position > self.position_total then
                    self.position = 1;
                end
                cursor:set_default_animation( "Position" .. self.position )
            elseif button == "Up" then
                self.position = self.position - 1
                if self.position <= 0 then
                    self.position = self.position_total;
                end
                cursor:set_default_animation( "Position" .. self.position )
            end
        end

        return 0
    end,



    show = function( self )
        ui_manager:get_widget( "BeginMenu" ):set_visible( true )

        return 0;
    end,



    hide = function( self )
        ui_manager:get_widget( "BeginMenu" ):set_visible( false )

        return 0;
    end,
}
