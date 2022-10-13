EntityContainer = {}



EntityContainer[ "Director" ] = {
    on_start = function( self )
        background2d:autoscroll_to_entity( entity_manager:get_entity( "Cloud" ) )
        --0358 (end 0358): [UNREVERSED] BTLMD(20, 00);
        --035b (end 035b): music:execute_akao( 10, pointer_to_field_AKAO_0 ); -- play field music
        --035d (end 035d): field:map_name(2);
        --field:lock_walkmesh( 37, true )
        --field:lock_walkmesh( 10, true )
        --field:lock_walkmesh( 8, true )
        --field:lock_walkmesh( 2, true )
        --field:lock_walkmesh( 51, true )
        --field:lock_walkmesh( 52, true )
        --field:lock_walkmesh( 53, true )
        --field:lock_walkmesh( 61, true )
        if FFVII.Data.progress_game == 27 then
            --music:execute_akao( 29, 40, 0, 0, 0, 7200, 22 );
            --04b6 (end 04bd): [UNREVERSED] BTLMD(22, 00);
            --04b9 (end 04bd): [UNREVERSED] BMUSC(0);
        end
        --music:execute_akao( c0, 7f, 0, 0, 0, f500, f000 );
        --music:lock( false );
        --music:execute_akao( 10, pointer_to_field_AKAO_1 ); -- play field music
        --04cf (end 04d1): [UNREVERSED] BMUSC(2);
        if FFVII.Data.progress_game < 27 then
        --field:random_encounter_on( false );
        --field:map_name(3);
        end
        script:request( Script.ENTITY, "Director", "main", 6 )
        return 0
    end,
    
    main = function( self )
        --script:request( "Chest", "script_3", 6 )
        if FFVII.Data.progress_game == 27 then
            --script:request( "time", "create_timer", 6 )
        end
        --04ec (end 04ec): temp6_00 = cl:get_move_triangle_id();

        return 0
    end,
}



EntityContainer[ "Time" ] = {
    on_start = function( self )
        
        return 0
    end,
    display_time = function( self )
        -- set window mode (id = 3, MessageStyle.NONE, disable input from player: true);
        -- set window parameters (id = 3, x = 10, y = 10, width = 160, height = 70);
        --0505 (end 0505): [UNREVERSED] WSPCL(03, 01, 0a, 0a);
        --050a (end 050a): message:show_text_wait(3, 25, x, y);
        --050d (end 050d): return
        return 0
    end,
    check_time = function( self )
        --050e (end 050e): return
        --050f (end 050f): if ( game:variable_get( "progress_game" ) == 27 ) then continue else jumpto(0537);
        --0517 (end 0537): if ( game:variable_get( "timer_minutes" ) == 0 ) then continue else jumpto(0537);
        --051d (end 0537): if ( game:variable_get( "timer_seconds" ) == 0 ) then continue else jumpto(0537);
        --0523 (end 0537): if ( game:variable_get( "timer_frames" ) == 0 ) then continue else jumpto(0537);
        --0529 (end 0537): game:variable_set("act1_1_flags1", (game:variable_get( "act1_1_flags1" ) | (1 << 7)));
        --052d (end 0537): field:jump_to_map(119, 18, 64372, 1, 0);
        --0537 (end 0537): jumpto( 050f );
        return 0
    end,
}



EntityContainer[ "Cloud" ] = {
    cloud = nil,

    on_start = function( self )
        set_entity_to_character( "Cloud", "Cloud" )
        self.cloud = entity_manager:get_entity( "Cloud" )
        self.cloud:set_solid( true )
        self.cloud:set_visible( true )

        return 0
    end,
    
    script_3 = function( self )
        self.cloud:set_rotation( 167.344 )
        return 0
    end,
    
    script_4 = function( self )
        self.cloud:play_animation_stop( "TalkGesture" ) -- speed=1
        self.cloud:animation_sync()
        --0549 (end 0549): -- set window parameters (id = 1, x = 30, y = 140, width = 293, height = 41);
        --0553 (end 0553): message:show_text_wait(1, 52, x, y);
        return 0
    end,
    
    script_5 = function( self )
        self.cloud:play_animation_stop( "TalkDontKnow" ) -- speed=1
        self.cloud:animation_sync()
        return 0
    end,
    
    script_6 = function( self )
        -- set window parameters (id = 0, x = 30, y = 140, width = 177, height = 57);
        message:show_text_wait(0, 55, x, y);
        return 0
    end,
    
    script_7 = function( self )
        player_lock( true )
        --056b (end 056b): field:menu_lock(true);
        self.cloud:move_to_position( -12.2344, 26.9531 )
        self.cloud:move_sync()
        self.cloud:set_rotation( 279.844 )
        -- set window parameters (id = 0, x = 90, y = 8, width = 133, height = 41);
        --0580 (end 0580): message:show_text_wait(0, 65, x, y);
        player_lock( true )
        --0585 (end 0585): field:menu_lock(false);
        return 0
    end,
    
    script_8 = function( self )
        player_lock( true )
        --058a (end 058a): field:menu_lock(true);
        self.cloud:move_to_position( -10.2969, 20.9297 )
        self.cloud:move_sync()
        self.cloud:set_rotation( 54.8438 )
        -- set window parameters (id = 0, x = 90, y = 8, width = 133, height = 41);
        --059f (end 059f): message:show_text_wait(0, 64, x, y);
        player_lock( true )
        --05a4 (end 05a4): field:menu_lock(false);
        return 0
    end,
    
    script_9 = function( self )
        self.cloud:set_solid( false )
        return 0
    end,
    
    script_10 = function( self )
        self.cloud:set_solid( true )
        return 0
    end,
}



EntityContainer[ "Barret" ] = {
    barret = nil,

    on_start = function( self )
        self.barret = entity_manager:get_entity( "Biggs" )
        self.barret:set_visible( false )
        self.barret:set_solid( false )

        if FFVII.Data.progress_game == 10 then
                self.barret:set_position( -5.39844, 18.0781, -2.15625 ) -- triangle_id=30
                self.barret:set_rotation( 0 )
                self.barret:set_talkable( true );
                self.barret:set_solid( true )
                self.barret:set_visible( true )
        end

        return 0
    end,
    
    script_3 = function( self )
        self.barret:set_move_speed( 2.34375 )
        self.barret:play_animation( "TalkGesture" ) -- speed=1
        self.barret:animation_sync()
        -- set window parameters (id = 0, x = 130, y = 10, width = 247, height = 41);
        --05e2 (end 05e2): message:show_text_wait(0, 51, x, y);
        return 0
    end,
    script_4 = function( self )
        self.barret:play_animation( "TalkGesture" ) -- speed=1
        self.barret:animation_sync()
        -- set window parameters (id = 0, x = 70, y = 10, width = 218, height = 57);
        --05f3 (end 05f3): message:show_text_wait(0, 53, x, y);
        return 0
    end,
    script_5 = function( self )
        self.barret:set_default_animation( "Angry1" ) -- speed=1
        self.barret:play_animation( "Angry1" )
        -- set window parameters (id = 0, x = 70, y = 8, width = 248, height = 73);
        --0604 (end 0604): message:show_text_wait(0, 54, x, y);
        self.barret:set_default_animation( "Idle" ) -- speed=1
        self.barret:play_animation( "Idle" )
        return 0
    end,
}



EntityContainer[ "Biggs" ] = {
    biggs = nil,

    on_start = function( self )
        self.biggs = entity_manager:get_entity( "Biggs" )

        self.biggs:set_position( -6.67188, 18.3203, -2.16406 )
        self.biggs:set_rotation( 0 )
        self.biggs:set_solid( true )
        self.biggs:set_visible( true )
        self.biggs:set_talkable( true );

        if FFVII.Data.progress_game < 27 then
            if FFVII.Data.m1_door1_open == true then
                self.biggs:set_visible( true )
                set_position( -13.2266, 21.8672, -2.13281 ) -- triangle_id=45
            end
        end
        
        self.biggs:set_solid( false )
        self.biggs:set_visible( false )
        self.biggs:set_talkable( false );
        
        if FFVII.Data.m1_door1_open == true then
            self.biggs:set_position( -10.4297, 26.3047, -2.13281 ) -- triangle_id=28
            self.biggs:set_direction( 234.844 )
            self.biggs:set_visible( true );
            self.biggs:set_solid( true )
            self.biggs:set_talkable( true );
            if FFVII.Data.m1_door2_open == true then
                self.biggs:set_position( -9.91406, 23.1953, -2.13281 ) -- triangle_id=39
                self.biggs:set_direction( 234.844 )
                self.biggs:set_talkable( true );
                self.biggs:set_solid( true )
                self.biggs:set_visible( true )
            end
        end
        if FFVII.Data.m1_door1_open == true then
            self.biggs:set_solid( true )
            self.biggs:set_visible( true )
            self.biggs:set_talkable( true )
        end
        script:request( Script.ENTITY, "Biggs", "main", 6 )
        return 0
    end,
    
    main = function( self )
        return 0
    end,
    
    on_interact = function( self )
        if FFVII.Data.progress_game < 27 then
            if FFVII.Data.m1_door1_open == false then
                FFVII.Data.m1_door1_open = true
                --script:request( Script.ENTITY, "Door1", "open", 6 )
            end
            player_lock( true )
            -- set window parameters (id = 0, x = 10, y = 10, width = 266, height = 57);
            --message:show_text_wait(0, 71, x, y);
            player_lock( false )
            return 0
        end
        if FFVII.Data.m1_jessie_unstuck == false then
            -- set window parameters (id = 0, x = 10, y = 10, width = 211, height = 41);
            --message:show_text_wait(0, 76, x, y);
            return 0
        end
        if FFVII.Data.m1_bomb_set == false then
            player_lock( true )
            -- set window parameters (id = 0, x = 10, y = 10, width = 85, height = 41);
            --message:show_text_wait(0, 74, x, y);
            player_lock( false )
            return 0
        end
        if FFVII.Data.m1_door1_open == false then
            FFVII.Data.m1_door1_open = true
            --script:request( Script.ENTITY, "Door1", "open", 6 )
            return 0
        end
        player_lock( true )
        -- set window parameters (id = 0, x = 10, y = 10, width = 85, height = 41);
        --message:show_text_wait(0, 74, x, y);
        player_lock( false )
        return 0
    end,
    
    script_3 = function( self )
        self.biggs:turn_to_direction( 257.344, Entity.CLOSEST, Entity.SMOOTH, 0.333333 )
        self.biggs:turn_sync()
        script:wait( 0.333333 )
        self.biggs:play_animation_stop( "DoorOpen" ) -- speed=1
        self.biggs:animation_sync()
        --08f4 (end 08f4): music:execute_akao( 20, 40, 33 ); -- play sound
        -- set window parameters (id = 0, x = 30, y = 120, width = 127, height = 41);
        --0903 (end 0903): message:show_text_wait(0, 63, x, y);
    end,
    
    script_4 = function( self )
        self.biggs:set_talkable( false );
        self.biggs:set_solid( false )
        self.biggs:set_move_speed( 3.75 )
        self.biggs:move_to_position( -6.95313, 19.8125 )
        self.biggs:move_sync()
        self.biggs:move_to_position( -10.4688, 21.5234 )
        self.biggs:move_sync()
        self.biggs:move_to_position( -13.2266, 21.8672 )
        self.biggs:move_sync()
        self.biggs:set_rotation( 54.8438 )
        self.biggs:set_default_animation( "Idle" ) -- speed=1
        self.biggs:play_animation( "Idle" )
        self.biggs:set_talkable( true );
        self.biggs:set_solid( true )
    end,
    
    script_5 = function( self )
        self.biggs:set_talkable( false );
        self.biggs:set_solid( false )
        self.biggs:set_move_speed( 6.09375 )
        self.biggs:move_to_position( -12.2344, 25.3594 )
        self.biggs:move_sync()
        self.biggs:move_to_position( -10.0156, 23.1875 )
        self.biggs:move_sync()
        self.biggs:set_direction( 54.8438 )
        self.biggs:set_talkable( true );
        self.biggs:set_solid( true )
        return 0
    end,
    
    script_6 = function( self )
    self.biggs:set_talkable( false );
        self.biggs:set_solid( false )
        self.biggs:set_move_speed( 3.75 )
        self.biggs:move_to_position( -8.03906, 21.5625 )
        self.biggs:move_sync()
        --music:execute_akao( 20, 40, 33 ); -- play sound
        -- set window parameters (id = 0, x = 30, y = 120, width = 127, height = 41);
        --message:show_text_wait(0, 63, x, y);
        self.biggs:set_talkable( true );
        self.biggs:set_solid( true )
        return 0
    end,
    
    script_7 = function( self )
        self.biggs:set_solid( false )
        self.biggs:move_to_position( -8.11719, 20.0078 )
        self.biggs:move_sync()
        self.biggs:move_to_position( -5.72656, 19.2031 )
        self.biggs:move_sync()
        self.biggs:move_to_position( -5.50781, 15.9063 )
        self.biggs:move_sync()
        self.biggs:move_to_position( -6.23438, 7.34375 )
        self.biggs:move_sync()
        self.biggs:set_talkable( false );
        self.biggs:set_solid( false )
        self.biggs:set_visible( false )
        return 0
    end,
}



EntityContainer[ "Jessie" ] = {
    jessie = nil,

    on_start = function( self )
        self.jessie = entity_manager:get_entity( "Jessie" )
        -- assosiate entity with model (CHAR) argument doesn't matter
        self.jessie:set_position( -4.49219, 20.5547, -2.14844 ) -- triangle_id=31
        self.jessie:set_rotation( 9.84375 )
        if FFVII.Data.progress_game < 27 then
            if FFVII.Data.m1_door1_open == true then --Var[1][225] bit on 3
                self.jessie:set_position( -13.1719, 23.6016, -2.13281 ) -- triangle_id=45
                self.jessie:set_rotation( 180 )
            end
            if FFVII.Data.m1_door2_open == true then --Var[1][225] bit on 4
                self.jessie:set_position( -12.8359, 34.2344, -2.13281 ) -- triangle_id=56
                self.jessie:set_rotation( 144.844 )
            end
            if FFVII.Data.m1_226 == true then --Var[1][226] bit on 1
                self.jessie:set_talkable( false );
                self.jessie:set_solid( false )
                self.jessie:set_visible( false )
                --09da (end 09dc): jumpto( 0a22 );
            end
        end
        self.jessie:set_talkable( false );
        self.jessie:set_solid( false )
        self.jessie:set_visible( false )
            if FFVII.Data.m1_jessie_unstuck == true then --Var[1][225] bit on 5
                self.jessie:set_position( -14.3438, 28.4688, -2.13281 ) -- triangle_id=54
                self.jessie:set_rotation( 54.8438 )
                self.jessie:set_talkable( true );
                self.jessie:set_solid( true )
                self.jessie:set_visible( true )
            end
            if FFVII.Data.m1_door2_open == true then
                self.jessie:set_position( -11.7109, 21.3203, -2.13281 ) -- triangle_id=43
                self.jessie:set_direction( 54.8438 )
                self.jessie:set_talkable( true );
                self.jessie:set_solid( true )
                self.jessie:set_visible( true )
            end
            if FFVII.Data.m1_door1_open == true then
                self.jessie:set_talkable( false );
                self.jessie:set_solid( false )
                self.jessie:set_visible( false )
            end
        return 0
    end,
    
    main = function( self )
    
        return 0
    end,
    
    on_interact = function( self )
        if FFVII.Data.progress_game < 27 then
            if FFVII.Data.m1_door1_open == true then
                if FFVII.Data.m1_door2_open == true then
                    --script:request( Script.ENTITY, "Door1", "open", 6 )
                end
            end
            if FFVII.Data.m1_door1_open == false then
                player_lock( true )
                --0a44 (end 0a64): field:menu_lock(true);
                -- set window parameters (id = 0, x = 10, y = 10, width = 250, height = 41);
                --message:show_text_wait(0, 70, x, y);
                player_lock( false )
                --0a55 (end 0a64): field:menu_lock(false);
            end
            if FFVII.Data.m1_door2_open == false then
                --script:request( Script.ENTITY, "Door2", "open", 6 )
            end
        end
        if FFVII.Data.m1_door2_open == false then
            --script:request( Script.ENTITY, "Door2", "open", 6 )
        end
        player_lock( true )
        --0a71 (end 0a84): field:menu_lock(true);
        -- set window parameters (id = 0, x = 10, y = 10, width = 149, height = 41);
        --message:show_text_wait(0, 75, x, y);
        player_lock( false )
        --0a82 (end 0a84): field:menu_lock(false);
        return 0
    end,
    
    script_3 = function( self )
        self.jessie:set_talkable( false );
        self.jessie:set_solid( false )
        self.jessie:set_move_speed( 6.09375 )
        self.jessie:move_to_position( -6.85938, 19.4141 )
        self.jessie:move_sync()
        self.jessie:move_to_position( -13.1719, 23.6016 )
        self.jessie:move_sync()
        self.jessie:set_rotation( 180 )
        self.jessie:set_talkable( true );
        self.jessie:set_solid( true )
        return 0
    end,
    
    script_4 = function( self )
        self.jessie:set_direction( 144.844 )
        script:wait( 0.333333 )
        self.jessie:play_animation_stop( "DoorOpen" ) -- speed=1
        self.jessie:animation_sync()
        self.jessie:execute_akao( 20, 40, 33 ); -- play sound
        -- set window parameters (id = 0, x = 30, y = 20, width = 127, height = 41);
        --message:show_text_wait(0, 62, x, y);
        return 0
    end,
    
    script_5 = function( self )
        self.jessie:set_talkable( false );
        self.jessie:set_solid( false )
        self.jessie:set_move_speed( 6.09375 )
        self.jessie:move_to_position( -11.0391, 23.6094 )
        self.jessie:move_sync()
        self.jessie:move_to_position( -12.0234, 26.2656 )
        self.jessie:move_sync()
        self.jessie:set_solid( true )
        self.jessie:set_talkable( true );
        return 0
    end,
    
    script_6 = function( self )
    self.jessie:set_talkable( false );
    self.jessie:set_solid( false )
    self.jessie:move_to_position( -12.8359, 34.2344 )
    self.jessie:move_sync()
    self.jessie:set_rotation( 144.844 )
    self.jessie:set_talkable( true );
    self.jessie:set_solid( true )
        return 0
    end,
    
    script_7 = function( self )
    self.jessie:set_talkable( false );
    self.jessie:set_solid( false )
    self.jessie:set_move_speed( 6.09375 )
    self.jessie:move_to_position( -13.6094, 25 )
    self.jessie:move_sync()
    --music:execute_akao( 20, 40, 33 ); -- play sound
    -- set window parameters (id = 0, x = 30, y = 20, width = 127, height = 41);
    --message:show_text_wait(0, 62, x, y);
    self.jessie:set_talkable( true );
    self.jessie:set_solid( true )
        return 0
    end,
    
    script_8 = function( self )
    self.jessie:set_talkable( false );
    self.jessie:set_solid( false )
    self.jessie:move_to_position( -11.6094, 25.2031 )
    self.jessie:move_sync()
    self.jessie:move_to_position( -11.7109, 21.3203 )
    self.jessie:move_sync()
    self.jessie:set_rotation( 54.8438 )
    self.jessie:set_talkable( true );
    self.jessie:set_solid( true )
        return 0
    end,
    
    script_9 = function( self )
    self.jessie:set_solid( false )
    self.jessie:set_move_speed( 3.75 )
    self.jessie:move_to_position( -8.11719, 20.0078 )
    self.jessie:move_sync()
    self.jessie:move_to_position( -5.72656, 19.2031 )
    self.jessie:move_sync()
    self.jessie:move_to_position( -5.50781, 15.9063 )
    self.jessie:move_sync()
    self.jessie:move_to_position( -6.23438, 7.34375 )
    self.jessie:move_sync()
    self.jessie:set_talkable( false );
    self.jessie:set_solid( false )
    self.jessie:set_visible( false )
        return 0
    end,
    
    script_10 = function( self )
    self.jessie:play_animation_stop( "DoorOpen2", 0, 0.533333 ) -- speed=1
    self.jessie:animation_sync()
    --0b4f (end 0b4f): music:execute_akao( 20, 40, 33 ); -- play sound
        return 0
    end,
    
    script_12 = function( self )
    self.jessie:set_talkable( false );
    self.jessie:set_solid( false )
    self.jessie:play_animation( "DoorOpen2", 0.533333, 0.966667 ) -- speed=1
    self.jessie:animation_sync()
    self.jessie:set_move_speed( 2.34375 )
    self.jessie:move_to_position( -11.7344, 34.5391 )
    self.jessie:move_sync()
    self.jessie:move_to_position( -11.5234, 36.1094 )
    self.jessie:move_sync()
    self.jessie:set_direction( 0 )
    FFVII.Data.progress_game.m1_226 = true
        return 0
    end,
}



EntityContainer[ "Chest" ] = {
    chest = nil,

    on_start = function( self )
    self.chest = entity_manager:get_entity( "Chest" )
    --0b76 (end 0b76): -- assosiate entity with model (CHAR) argument doesn't matter
    self.chest:set_position( 3.00781, 24.4141, -2.125 ) -- triangle_id=95
    self.chest:set_visible( true )
    self.chest:set_solid( true )
    self.chest:set_talkable( true )
    script:request( Script.ENTITY, "Chest", "main", 6 )
        return 0
    end,
    
    main = function( self )
        if FFVII.Data.item_sector1_chest1 == false then
            --0b8a (end 0b9d): [UNREVERSED] KAWAI(05, 0d, 00, 00);
            script:wait( 0.0333333 )
            --0b92 (end 0b9d): [UNREVERSED] KAWAI(04, 0d, 01);
            self.chest:play_animation_stop( "Idle", 0, 0 ) -- speed=1
            self.chest:animation_sync()
        end
        if FFVII.Data.item_sector1_chest1 == true then
        self.chest:play_animation_stop( "Idle", 0.966667, 0.966667 ) -- speed=1
        self.chest:animation_sync()
        end
        return 0
    end,
}



EntityContainer[ "BackgroundLight0" ] = {
    on_start = function( self )
        background2d:play_animation_looped( "Light0" )

        return 0
    end,
}



EntityContainer[ "BackgroundLight1" ] = {
    on_start = function( self )
        script:wait( 0.133333 )
        background2d:play_animation_looped( "Light1" )

        return 0
    end,
}



EntityContainer[ "BackgroundLight2" ] = {
    on_start = function( self )
        while true do
            local number = math.random( 1, 3 )
            background2d:play_animation_once( "Light2_" .. number )
            background2d:animation_sync( "Light2_" .. number )
        end

        return 0
    end,
}



EntityContainer[ "BackgroundWarning" ] = {
    on_start = function( self )
        local temp5_27 = 0;

        while true do
            if temp5_27 > 2 then
                temp5_27 = 0
            end

            local number = math.random( 0, 3 )
            if number == 0 then
                number = 1
            end

            for v = 0, number, 1 do
                background2d:play_animation_once( "Warning" .. temp5_27 .. "Zizizi" )
                background2d:animation_sync( "Warning" .. temp5_27 .. "Zizizi" )
            end

            background2d:play_animation_once( "Warning" .. temp5_27 .. "Light" )
            background2d:animation_sync( "Warning" .. temp5_27 .. "Light" )
            script:wait( 0.0333333 )
            background2d:play_animation_once( "Warning" .. temp5_27 .. "Light" )
            background2d:animation_sync( "Warning" .. temp5_27 .. "Light" )
            script:wait( 0.0333333 )
            background2d:play_animation_once( "Warning" .. temp5_27 .. "Light" )
            background2d:animation_sync( "Warning" .. temp5_27 .. "Light" )
            background2d:play_animation_once( "WarningBlank" )
            background2d:animation_sync( "WarningBlank" )
            script:wait( 0.0333333 )

            temp5_27 = temp5_27 + 1
        end

        return 0
    end,
}



EntityContainer[ "Gateway0" ] = {
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
        if entity == "Cloud" then
            load_field_map_request( "ffvii_nrthmk", "Spawn_nmkin_1" )
        end

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
        if entity == "Cloud" then
            load_field_map_request( "ffvii_elevtr1", "Spawn_elevtr1_1f" )
        end

        return 0
    end,

    on_leave_line = function( self, entity )
        return 0
    end,
}