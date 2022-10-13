EntityContainer = {}



EntityContainer[ "Director" ] = {
    on_start = function( self )
        background2d:autoscroll_to_entity( entity_manager:get_entity( "Cloud" ) )
        --0288 (end 0288): temp6_02 = 29;
        --028d (end 028d): temp6_04 = 166;
        if FFVII.Data.progress_game == 27 then
            --029a (end 02af): music:execute_akao( 29, 40, 0, 0, 0, 7200, 22 );
            --02a8 (end 02af): [UNREVERSED] BTLMD(22, 00);
            --02ab (end 02af): [UNREVERSED] BMUSC(0);
        end   
        --02af (end 02c1): music:execute_akao( 2a, 40, 0, 0, 0, f000, f601 );
        --02bd (end 02c1): music:execute_akao( 10, pointer_to_field_AKAO_1 ); -- play field music
        --02bf (end 02c1): [UNREVERSED] BMUSC(2);
        if FFVII.Data.progress_game > 27 then
            --02c9 (end 02cf): field:map_name(4);
            --02cb (end 02cf): field:set_battle_table(1);
        end
        --02cf (end 02d1): field:map_name(3);
        script:request( Script.ENTITY, "Director", "main", 6 )
        return 0
    end,
    
    main = function( self )
        if FFVII.Data.progress_game == 27 then
        --script:request( Script.ENTITY, "Time", "display_time", 6 )
        --02dd (end 02dd): temp6_00 = cl:get_move_triangle_id();
        end
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
        
        -- [[ When skipping to this field from menu, ensure player is able to move ]]
        if FFVII.Data.field_skip == true then
            FFVII.set_party( "Cloud", nil, nil );
            self.cloud:play_animation( "Idle" )
            self.cloud:set_position( -2.67, 1.74, 12.37 )
            self.cloud:set_rotation( 67.7647 )
            FFVII.Data.progress_game = 12
            player_lock( false )
        end

        return 0
    end,

    --[[ Climb up ladder at top of reactor. ]]
    script_3 = function( self )
        player_lock( true )
        --0334 (end 0334): field:menu_lock(true);
        --0336 (end 0336): music:execute_akao( 20, 40, 15 ); -- play sound
        self.cloud:set_solid_range( 1 );
        self.cloud:move_to_position( 1.3125, 1.10938 )
        self.cloud:move_sync()
        self.cloud:set_solid_range( 30 );
        player_lock( false )
        --0349 (end 0349): field:menu_lock(false);
        self.cloud:linear_to_position( 166, 248, 1805, 102, Entity.DOWN_TO_UP ); -- direction needs to be seted to "180" and animation during climbing is "3" with speed "1"  
        return 0
    end,

    --[[ Climb down ladder at top of reactor. ]]
    script_4 = function( self )
        player_lock( true )
        --035d (end 035d): field:menu_lock(true);
        --035f (end 035f): music:execute_akao( 20, 40, 15 ); -- play sound
        self.cloud:set_solid_range( 1 );
        self.cloud:move_to_position( 1.29688, 1.9375 )
        self.cloud:move_sync()
        self.cloud:set_solid_range( 30 );
        player_lock( false )
        --0372 (end 0372): field:menu_lock(false);
        self.cloud:linear_to_position( 168, 142, 1582, 79, Entity.UP_TO_DOWN ); -- direction needs to be seted to "180" and animation during climbing is "3" with speed "1"
        return 0
    end,

    --[[ Unknown cloud sitting down scene ]]
    script_5 = function( self )
        self.cloud:set_default_animation( "SittingDown" ) -- speed=1
        self.cloud:play_animation( "SittingDown" )
        --0387 (end 0387): music:execute_akao( 20, 40, 104 ); -- play sound
        self.cloud:move_to_position( 913, 1638, false );
        self.cloud:set_move_speed( 3.75 )
        self.cloud:move_to_position( 904, 562, false );
        self.cloud:set_move_speed( 0.9375 )
        self.cloud:play_animation( "5", 0.466667, 1.7, 1, true );
        --03a5 (end 03a5): music:execute_akao( 20, 40, 3e ); -- play sound
        self.cloud:jump_to_position( 957, 708, 34, 0.566667 );
        script:wait( 0.666667 )
        self.cloud:set_default_animation( "Idle" ) -- speed=1
        self.cloud:play_animation( "Idle" )
        return 0
    end,
}



EntityContainer[ "Jessie" ] = {
    jessie = nil,

    on_start = function( self )
        self.jessie = entity_manager:get_entity( "Jessie" )
        -- assosiate entity with model (CHAR) argument doesn't matter
        self.jessie:set_position( -3.83594, -0.414063, 11.7188 ) -- triangle_id=72
        if FFVII.Data.progress_game >= 13 then
            self.jessie:set_talkable( false );
            self.jessie:set_solid( false )
            self.jessie:set_visible( false )
        end
        self.jessie:set_solid( false )
        script:request( Script.ENTITY, "Jessie", "main", 6 )
        return 0
    end,

    --[[ Run down the stairs. ]]
    main = function( self )
        if FFVII.Data.progress_game < 13 then
            self.jessie:set_move_auto_speed( 2.8125 )
            self.jessie:move_to_position( 2.97656, -0.257813 )
            self.jessie:move_sync()
            self.jessie:move_to_position( 5.37969, 6.43906 )
            self.jessie:move_sync()
            self.jessie:move_to_position( 6.34063, 6.39063 )
            self.jessie:move_sync()
            self.jessie:set_move_auto_speed( 3.75 )
            FFVII.Data.progress_game = 13
            self.jessie:move_to_position( 6.22313, -1.27 )
            self.jessie:move_sync()
            self.jessie:set_move_auto_speed( 2.8125 )
            self.jessie:move_to_position( -6.375, -1.96875 )
            self.jessie:move_sync()
            self.jessie:set_talkable( false );
            self.jessie:set_solid( false )
            self.jessie:set_visible( false )
        end
        return 0
    end,
}



EntityContainer[ "Wedge" ] = {
    wedge = nil,

    on_start = function( self )
        self.wedge = entity_manager:get_entity( "Wedge" )
        self.wedge:set_position( 5.94141, 5.875, 5.07031 )
        self.wedge:set_rotation( 16.875 )
        self.wedge:set_default_animation( "LookAround" )
        self.wedge:play_animation( "LookAround" )

        if FFVII.Data.progress_game >= 7 then
            self.wedge:set_solid( false )
            self.wedge:set_visible( false )
        else
            self.wedge:set_solid( true )
            self.wedge:set_visible( true )
        end

        return 0
    end,

    scene_part_12 = function( self )
        self.wedge:set_move_auto_speed( 1.17187 )
        self.wedge:move_to_position( 4.57422, 6.82812 )
        self.wedge:move_sync()
        self.wedge:move_to_position( 5.29297, 13.1875 )
        self.wedge:move_sync()
        self.wedge:set_solid( false )
        self.wedge:set_visible( false )

        return 0
    end,
}



EntityContainer[ "DoorLeft" ] = {
    door = nil,

    on_start = function( self )
        self.door = entity_manager:get_entity( "DoorLeft" )
        self.door:set_position( 4.25, 7.20703, 5.07031 )
        self.door:set_rotation( 354.375 )

        if FFVII.Data.progress_game >= 7 then
            self.door:set_visible( false )
        else
            self.door:set_visible( true )
        end

        self.door:set_solid( false )

        return 0
    end,

    open = function( self )
        self.door:offset_to_position( -0.789062, 0, 0, Entity.LINEAR, 1.66667 )
        self.door:offset_sync()
        self.door:set_visible( false )

        return 0
    end,
}



EntityContainer[ "DoorRight" ] = {
    door = nil,

    on_start = function( self )
        self.door = entity_manager:get_entity( "DoorRight" )
        self.door:set_position( 4.93359, 7.125, 5.07031 )
        self.door:set_rotation( 174.375 )

        if FFVII.Data.progress_game >= 7 then
            self.door:set_visible( false )
        else
            self.door:set_visible( true )
        end

        self.door:set_solid( false )

        return 0
    end,

    open = function( self )
        self.door:offset_to_position( 0.71875, 0, 0, Entity.LINEAR, 1.66667 )
        self.door:offset_sync()
        self.door:set_visible( false )

        return 0
    end,
}



EntityContainer[ "Movie" ] = {
    movie_check = false,

    on_start = function( self )
        while true do
            if self.movie_check == true then
                --0717 (end 0729): temp6_09 = field:get_movie_frame();
                --071a (end 0729): if ( temp6_09 > 30 ) then continue else jumpto(0729);
                script:request( Script.ENTITY, "Cloud", "scene_part_16", 6 )
                break
            end

            script:wait( 0 )
        end

        return 0
    end,
}



--[[
fan
script_0:
072c (end 072c): field:background_clear( "2" ); -- turns off all layer of background animation with given id
072f (end 072f): return;
0730 (end 0730): script:wait( 0.1 )
0733 (end 0733): field:background_on( "2", 0 ); -- turns on layer of background animation with given id
0737 (end 0737): script:wait( 0.133333 )
073a (end 073a): field:background_off( "2", 0 ); -- turns off layer of background animation with given id
073e (end 073e): field:background_on( "2", 1 ); -- turns on layer of background animation with given id
0742 (end 0742): script:wait( 0.133333 )
0745 (end 0745): field:background_off( "2", 1 ); -- turns off layer of background animation with given id
0749 (end 0749): jumpto( 0730 );

light
script_0:
074b (end 074b): temp5_16 = 66;
074f (end 074f): return;
0750 (end 0750): [UNREVERSED] STPAL( 00, 0d, 00, ff );
0755 (end 0755): if ( temp5_16 < 62 ) then continue else jumpto(0775);
075b (end 0775): temp5_16 = temp5_16 + 2;
075f (end 0775): [UNREVERSED] MPPAL2( 00, 55, 50, 00, 10, 16, 16, 16, ff );
0769 (end 0775): [UNREVERSED] CPPAL( 00, 00, 10, 00 );
076e (end 0775): [UNREVERSED] LDPAL( 00, 10, 0d, ff );
0773 (end 0775): jumpto( 0755 );
0775 (end 0775): if ( temp5_16 > 32 ) then continue else jumpto(0795);
077b (end 0795): temp5_16 = temp5_16 - 2;
077f (end 0795): [UNREVERSED] MPPAL2( 00, 55, 50, 00, 10, 16, 16, 16, ff );
0789 (end 0795): [UNREVERSED] CPPAL( 00, 00, 10, 00 );
078e (end 0795): [UNREVERSED] LDPAL( 00, 10, 0d, ff );
0793 (end 0795): jumpto( 0775 );
0795 (end 0795): jumpto( 0755 );

light2
script_0:
0797 (end 0797): temp5_17 = 66;
079b (end 079b): return;
079c (end 079c): script:wait( 0.666667 )
079f (end 079f): [UNREVERSED] STPAL( 00, 0e, 20, ff );
07a4 (end 07a4): if ( temp5_17 < 62 ) then continue else jumpto(07c4);
07aa (end 07c4): temp5_17 = temp5_17 + 1;
07ae (end 07c4): [UNREVERSED] MPPAL2( 00, 55, 50, 20, 30, 17, 17, 17, ff );
07b8 (end 07c4): [UNREVERSED] CPPAL( 00, 20, 30, 00 );
07bd (end 07c4): [UNREVERSED] LDPAL( 00, 30, 0e, ff );
07c2 (end 07c4): jumpto( 07a4 );
07c4 (end 07c4): if ( temp5_17 > 16 ) then continue else jumpto(07e4);
07ca (end 07e4): temp5_17 = temp5_17 - 1;
07ce (end 07e4): [UNREVERSED] MPPAL2( 00, 55, 50, 20, 30, 17, 17, 17, ff );
07d8 (end 07e4): [UNREVERSED] CPPAL( 00, 20, 30, 00 );
07dd (end 07e4): [UNREVERSED] LDPAL( 00, 30, 0e, ff );
07e2 (end 07e4): jumpto( 07c4 );
07e4 (end 07e4): jumpto( 07a4 );
]]



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
            load_field_map_request( "ffvii_elevtr1", "Spawn_elevtr1_1f" )
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
            load_field_map_request( "ffvii_nmkin_3", "Spawn_nmkin_2" )
        end

        return 0
    end,

    on_leave_line = function( self, entity )
        return 0
    end,
}
