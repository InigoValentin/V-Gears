EntityContainer = {}



EntityContainer[ "Director" ] = {
    on_start = function( self )
        background2d:autoscroll_to_entity( entity_manager:get_entity( "Cloud" ) )
        --0358 (end 0358): [UNREVERSED] BTLMD(20, 00);
        --035b (end 035b): music:execute_akao( 10, pointer_to_field_AKAO_0 ); -- play field music
        --035d (end 035d): field:map_name(2);
        
        while true do
            if FFVII.Data.progress_game >= 64 then -- previously 13 
                break
            end

            if EntityContainer[ "Cloud" ].cloud ~= nil then
                local triangle_id = EntityContainer[ "Cloud" ].cloud:get_move_triangle_id()

                --[[ Going down. ]]
                if ( FFVII.Script.is_executing == false ) and ( triangle_id == 8 ) and ( FFVII.Data.elevtr1_floor == 1 ) then
                    -- The entities have to be slightly "inside" the walkmesh for the above statement to be true
                        FFVII.Script.is_executing = true
                        player_lock( true )
                        script:request_end_sync( Script.ENTITY, "Cloud", "press_button", 6 )
                        if FFVII.Data.progress_game < 12 then
                            FFVII.Data.progress_game = 12
                            script:request_end_sync( Script.ENTITY, "Barret", "talk_about_planet", 6 )
                            script:request_end_sync( Script.ENTITY, "Jessie", "leave_elevator", 6 )
                        end
                        script:request_end_sync( Script.ENTITY, "Cloud", "walk_away_from_button", 6 )
                        FFVII.Data.elevtr1_floor = 0 -- This must be set when not in triangle 8
                        player_lock( false )
                end
                
                --[[ Going up. ]]
                if ( FFVII.Script.is_executing == false ) and ( triangle_id == 8 ) and ( FFVII.Data.elevtr1_floor == 0 ) then
                    -- The entities have to be slightly "inside" the walkmesh for the above statement to be true
                        FFVII.Script.is_executing = true
                        player_lock( true )
                        script:request_end_sync( Script.ENTITY, "Cloud", "press_button", 6 )
                        if FFVII.Data.progress_game < 12 then
                            FFVII.Data.progress_game = 12
                            script:request_end_sync( Script.ENTITY, "Barret", "talk_about_planet", 6 )
                            script:request_end_sync( Script.ENTITY, "Jessie", "leave_elevator", 6 )
                        end
                        script:request_end_sync( Script.ENTITY, "Cloud", "walk_away_from_button", 6 )
                        FFVII.Data.elevtr1_floor = 1 -- This must be set when not in triangle 8
                        player_lock( false )
                end
            FFVII.Script.is_executing = false
            end

            script:wait( 0 )
        end
        return 0
    end,
}



EntityContainer[ "Elevator" ] = {
    on_start = function( self )
        return 0
    end,
}



EntityContainer[ "Cloud" ] = {
    cloud = nil,

    on_start = function( self )
        set_entity_to_character( "Cloud", "Cloud" );
        self.cloud = entity_manager:get_entity( "Cloud" )
        self.cloud:set_solid( true )
        self.cloud:set_visible( true )
        script:request( Script.ENTITY, "Cloud", "enter_elevator", 6 )

        return 0
    end,

    script_0 = function( self )
        --0425 (end 0425): -- assosiate entity with model (CHAR) argument doesn't matter
        --0427 (end 0427): set_entity_to_character( "cl", 0 );
        --0429 (end 0429): return
        --042a (end 042a): jumpto( 042a );
        return 0
    end,

    --[[ Enter the elevator. ]]
    enter_elevator = function( self )
        player_lock( true )
        self.cloud:set_move_auto_speed( 1.0 )
        self.cloud:move_to_position( -0.484375, 0 )
        self.cloud:move_sync()
        player_lock( false )
        return 0
    end,

    --[[ Face the control box and slam the elevator button. ]]
    press_button = function( self )
        self.cloud:move_to_position( 0.671875, 0.5 )
        self.cloud:move_sync()
        self.cloud:turn_to_direction( 180, Entity.CLOSEST, Entity.SMOOTH, 0.333333 )
        self.cloud:turn_sync()
        script:wait( 0.133333 )
        self.cloud:play_animation_stop( "ButtonPressUp", 0 , -1 ) -- speed=1
        self.cloud:animation_sync()
        self.cloud:play_animation( "ButtonPressDown", 0 , -1 ) -- speed=1
        self.cloud:animation_sync()
        script:wait( 0.5 )
        if FFVII.Data.progress_game <= 12 then
            script:request( Script.ENTITY, "Jessie", "stop_showing_button", 6 )
        end
        --0443 (end 0443): music:execute_akao( 20, 40, 33 ); -- play sound
        --if ( !(game:variable_get( "act1_1_flags1" ) & (1 << 0)) ) then continue else jumpto(0458);
        --    field:background_off( "2", 1 ); -- turns off layer of background animation with given id
        --    field:background_on( "2", 2 ); -- turns on layer of background animation with given id
        --end
        --0456 (end 0458): jumpto( 0460 );
        --field:background_off( "2", 2 ); -- turns off layer of background animation with given id
        --field:background_on( "2", 1 ); -- turns on layer of background animation with given id
        -- set window parameters (id = 0, x = 150, y = 100, width = 86, height = 25);
        --message:show_text_wait(0, 45, x, y);
        --self.cloud:play_animation_stop( "4" ) -- speed=1
        --self.cloud:animation_sync()
        --self.cloud:set_default_animation( "0" ) -- speed=1
        --self.cloud:play_animation( "0" )
        return 0
    end,
    --[[ Walk away from the button so we dont trigger it again ]]
    walk_away_from_button = function( self )
        self.cloud:move_to_position( 0, 0.4 )
        self.cloud:move_sync()
        return 0
    end,
    --[[ Cloud looks at Barret ]]
    lookat_barret = function( self )
        --self.cloud:turn_to_entity( barret, Entity.CLOSEST, 0.5);
        self.cloud:turn_to_direction( -45, Entity.CLOSEST, Entity.SMOOTH, 0.5 )
        self.cloud:turn_sync()
        script:wait( 0.5 )
        return 0
    end,
    --[[ Cloud moves his hands into a "Don't Know" gesture ]]
    not_my_problem = function( self )
        self.cloud:play_animation_stop( "TalkDontKnow" ) -- speed=1
        self.cloud:animation_sync()
        --047c (end 047c): -- set window parameters (id = 2, x = 100, y = 20, width = 151, height = 41);
        --0486 (end 0486): message:show_text_wait(2, 59, x, y);
        self.cloud:play_animation_stop( "TalkDontKnowFull", 1.46667, 1.96667 ) -- speed=1
        self.cloud:animation_sync()
        script:wait( 0.5 )
        self.cloud:set_default_animation( "Idle" ) -- speed=1
        self.cloud:play_animation( "Idle" )
        return 0
    end,
    --[[ Cloud shakes head "No" and says that all he cares about is finishing the job before security and roboguards come ]]
    get_out_before_security = function( self )
        self.cloud:play_animation( "TalkNo" ) -- speed=1
        self.cloud:animation_sync()
        --0498 (end 0498): -- set window parameters (id = 2, x = 65, y = 20, width = 285, height = 57);
        --04a2 (end 04a2): message:show_text_wait(2, 60, x, y);
        --04a5 (end 04a5): return
        return 0
    end,
}

EntityContainer[ "Barret" ] = {
    barret = nil,

    on_start = function( self )
        self.barret = entity_manager:get_entity( "Barret" )

        self.barret:set_position( 0, 0, 0.0390625 )
        self.barret:set_solid( false )
        self.barret:set_visible( false )

        return 0
    end,
    
    --[[ Barret tells Cloud that the planets dyin ]]
    talk_about_planet = function( self )
        self.barret:set_position( 0.671875, 0.5, 0.0390625  )
        self.barret:set_visible( true )
        script:wait( 0.3 )
        self.barret:set_move_auto_speed( 1.0 )
        self.barret:move_to_position( 0, 0.4 )
        script:wait( 0.3 )
        self.barret:turn_to_direction( 150, Entity.CLOSEST, Entity.SMOOTH, 0.5 )
        self.barret:turn_sync()
        script:wait( 0.5 )
        --04b1 (end 04b1): [UNREVERSED] SPLIT(00, 00, 00, f9, ff, f7, ff, 70, f9, ff, f7, ff, 70, 28);
        script:request_end_sync( Script.ENTITY, "Cloud", "lookat_barret", 6 )
        self.barret:play_animation( "TalkGesture" ) -- speed=1
        self.barret:animation_sync()
        -- set window parameters (id = 2, x = 10, y = 20, width = 245, height = 73);
        --message:show_text_wait(2, 57, x, y);
        script:request_end_sync( Script.ENTITY, "Cloud", "not_my_problem", 6 )
        script:wait( 0.266667 )
        self.barret:play_animation( "Angry2" ) -- speed=1
        self.barret:animation_sync()
        -- set window parameters (id = 2, x = 20, y = 30, width = 268, height = 41);
        --message:show_text_wait(2, 58, x, y);
        script:request_end_sync( Script.ENTITY, "Cloud", "get_out_before_security", 6 )
        self.barret:turn_to_direction( 0, Entity.CLOSEST, Entity.SMOOTH, 0.333333 )
        self.barret:turn_sync()
        self.barret:play_animation_stop( "Angry3Start" ) -- speed=1
        self.barret:animation_sync()
        self.barret:set_default_animation( "Angry3Continue" ) -- speed=1
        self.barret:play_animation( "Angry3Continue" )
        script:wait( 2 )
        self.barret:play_animation_stop( "Angry3End", 0, 0.466667 ) -- speed=1
        self.barret:animation_sync()
        script:wait( 0.666667 )
        self.barret:play_animation_stop( "Angry3End", 0.5, 0.966667 ) -- speed=1
        self.barret:animation_sync()
        self.barret:set_default_animation( "Idle" ) -- speed=1
        self.barret:play_animation( "Idle" )
        self.barret:play_animation( "Steps", 0, -1 );
        --self.barret:turn_to_entity( cl, Entity.CLOSEST, 0.0666667);
        self.barret:turn_to_direction( 150, Entity.CLOSEST, Entity.SMOOTH, 0.5 )
        self.barret:turn_sync()
        script:wait( 0.333333 )
        self.barret:move_to_position( 0.671875, 0.5 )
        script:wait( 0.6 )
        self.barret:set_visible( false )
        script:wait( 0.5 )
        --0516 (end 0516): [UNREVERSED] JOIN(28);
        --0518 (end 0518): temp5_06 = 1;
        return 0
    end,
}



EntityContainer[ "Jessie" ] = {
    jessie = nil,

    on_start = function( self )
        self.jessie = entity_manager:get_entity( "Jessie" )

        -- assosiate entity with model (CHAR) argument doesn't matter
        self.jessie:set_position( -0.75, 0.5625, 0.0390625 ) -- triangle_id=5
        --self.jessie:set_direction( 315 )
        if FFVII.Data.progress_game >= 12 then
            self.jessie:set_talkable( false );
            self.jessie:set_solid( false )
            self.jessie:set_visible( false )
        end
        if FFVII.Data.progress_game < 12 then
            self.jessie:set_solid( false )
            self.jessie:set_rotation( -90 )
            self.jessie:play_animation_stop( "PointToButton", 0.133333, 0.133333 ) -- speed=1
            self.jessie:animation_sync()
            script:wait( 0.333333 )
            self.jessie:play_animation_stop( "PointToButton", 0.133333, 0.4 ) -- speed=1
            self.jessie:animation_sync()
            script:wait( 0.266667 )
            self.jessie:play_animation_stop( "PointToButton", 0.4, 0.8 ) -- speed=1
            self.jessie:animation_sync()
            --063b (end 0648): -- set window parameters (id = 1, x = 15, y = 50, width = 202, height = 41);
            --0645 (end 0648): message:show_text_wait(1, 46, x, y);
            --0648 (end 0648): jumpto( 0648 );
        end

        return 0
    end,
    
    --[[ Jessie stops pointing at the button and looks out the door for possible danger ]]
    stop_showing_button = function( self )
        self.jessie:play_animation_stop( "PointToButton", 0.833333, 1.26667 ) -- speed=1
        self.jessie:animation_sync()
        return 0
    end,
    
    --[[ Jessie runs out of the elevator once you reach the reactor level ]]
    leave_elevator = function( self )
        self.jessie:set_default_animation( "Idle" ) -- speed=1
        self.jessie:play_animation( "Idle" )
        self.jessie:animation_sync()
        self.jessie:set_move_auto_speed( 1.75 )
        self.jessie:move_to_position( -0.76 , 0 )
        self.jessie:move_sync()
        self.jessie:move_to_position( -1.70 , 0 )
        self.jessie:move_sync()
        self.jessie:set_visible( false )
        return 0
    end,
}



EntityContainer[ "Gateway0" ] = {
    on_start = function( self )
        return 0
    end,

    on_enter_line = function( self, entity )
        --if entity == "Cloud" then
        --    load_field_map_request( "ffvii_nmkin_2", "Spawn_nmkin_2_up" )
        --end
        
        return 0
    end,

    on_move_to_line = function( self, entity )
        return 0
    end,

    on_cross_line = function( self, entity )
        if ( FFVII.Data.elevtr1_floor == 1 ) and ( entity == "Cloud" ) then
            load_field_map_request( "ffvii_nmkin_1", "Spawn_elevtr1" )
        end
        if ( FFVII.Data.elevtr1_floor == 0 ) and ( entity == "Cloud" ) then
            load_field_map_request( "ffvii_nmkin_2", "Spawn_elevtr1" )
        end
        --if entity == "Cloud" then
        --    load_field_map_request( "ffvii_nmkin_2", "Spawn_nmkin_2_up" )
        --end

        return 0
    end,

    on_leave_line = function( self, entity )
        return 0
    end,
}



EntityContainer[ "Gateway1" ] = {
    on_start = function( self )
        return 0
    end,

    on_enter_line = function( self, entity )
        return 0
    end,

    on_move_to_line = function( self, entity )
        return 0
    end,

    on_cross_line = function( self, entity )
        if entity == "Player" then
            load_field_map_request( "ffvii_nmkin_2", "Spawn_nmkin_2_up" )
        end

        return 0
    end,

    on_leave_line = function( self, entity )
        return 0
    end,
}