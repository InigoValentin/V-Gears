EntityContainer = {}



EntityContainer[ "Director" ] = {
    on_start = function( self )
        background2d:autoscroll_to_entity( entity_manager:get_entity( "Cloud" ) )
        script:request( Script.ENTITY, "Director", "main", 6 )
        return 0
    end,
    
    main = function( self )
        while true do
            if FFVII.Data.progress_game >= 64 then
                break
            end

            if EntityContainer[ "Cloud" ].cloud ~= nil then
                local triangle_id = EntityContainer[ "Cloud" ].cloud:get_move_triangle_id()

                --[[ Climbing up the 1st ladder ]]
                if ( FFVII.Data.moving_linear == false ) and ( triangle_id == 28 ) then
                    -- The entities have to be slightly "inside" the walkmesh for the above statement to be true
                    FFVII.Data.moving_linear = true -- This prevents this area from being executed constantly while player is in the walkmesh triangle
                    script:request( Script.ENTITY, "Cloud", "climb_ladder_1", 6 )
                    FFVII.Data.moving_linear = false
                end
                if ( FFVII.Data.moving_jump == false ) and ( triangle_id == 39 ) then
                    -- The entities have to be slightly "inside" the walkmesh for the above statement to be true
                    FFVII.Data.moving_jump = true -- This prevents this area from being executed constantly while player is in the walkmesh triangle
                    script:request( Script.ENTITY, "Cloud", "jumpto_right", 6 )
                    FFVII.Data.moving_jump = false
                end
                if ( FFVII.Data.moving_jump == false ) and ( triangle_id == 42 ) then
                    -- The entities have to be slightly "inside" the walkmesh for the above statement to be true
                    FFVII.Data.moving_jump = true -- This prevents this area from being executed constantly while player is in the walkmesh triangle
                    script:request( Script.ENTITY, "Cloud", "jumpto_left", 6 )
                    FFVII.Data.moving_jump = false
                end
            end

            script:wait( 0 )
        end
        return 0
    end,
}



EntityContainer[ "Cloud" ] = {
    cloud = nil,

    on_start = function( self )
        set_entity_to_character( "Cloud", "Cloud" )
        self.cloud = entity_manager:get_entity( "Cloud" )
        entity_manager:set_player_entity( "Cloud" )
        self.cloud:set_solid( true )
        self.cloud:set_visible( true )

        return 0
    end,
    
    --[[ Main script ]]
    main = function( self )
    
        return 0
    end,

    --[[ Climb up ladder at top of reactor. ]]
    climb_ladder_1 = function( self )
        --05af (end 05af): temp5_12 = 1;
        player_lock( true )
        --entity_manager:unset_player_entity()
        self.cloud:set_solid( false )
        --05b5 (end 05b5): field:menu_lock(true);
        --05b7 (end 05b7): music:execute_akao( 20, 40, 15 ); -- play sound
        --self.cloud:set_solid_radius( 1 );
        --self.cloud:move_to_position( -1.30469, 14.1875 ) -- REIMPLEMENT
        --self.cloud:move_to_position( -1.30469, 14.1875 ) --Bad movement (gets stuck on the walkmesh)
        --self.cloud:move_sync() -- REIMPLEMENT
        --self.cloud:set_solid_radius( 30 );
        player_lock( false )
        --05ca (end 05ca): field:menu_lock(false);
        self.cloud:set_rotation( 90 )
        self.cloud:linear_to_position( 1.78125, 14.625, 17.2969, Entity.DOWN_TO_UP, "Climb" ); -- direction needs to be seted to "90" and animation during climbing is "5" with speed "1"
        self.cloud:linear_sync()
        player_lock( true )
        self.cloud:set_rotation( 90 )
        self.cloud:move_auto_animation( false )
        self.cloud:play_animation_stop( "Jump" )
        script:wait( 0.5 ) -- wait to start of jump in animation
        self.cloud:jump_to_position( 1.54, 14.87, 17.2969, 0.567 ) -- triangle 24
        self.cloud:jump_sync()
        script:wait( 0.667 ) -- wait to end of animation
        self.cloud:move_auto_animation( true )
        --05dd (end 05dd): field:menu_lock(true);
        --05df (end 05df): temp6_07 = cl:get_x();
        --temp6_09 = cl:get_y();
        --temp6_0b = cl:get_z();
        --temp6_00 = cl:get_move_triangle_id();
        --05e7 (end 05e7): if ( temp6_0b > 1792 ) then continue else jumpto(060e);
        --05ef (end 060e): field:pc_lock(true);
        --05f1 (end 060e): field:menu_lock(true);
        --05f3 (end 060e): cl:set_direction( 180 )
        --05f6 (end 060e): cl:play_animation( "4", 0.5, 2.3, 1, true );
        --05fb (end 060e): music:execute_akao( 20, 40, 3e ); -- play sound
        --0600 (end 060e): cl:jump_to_position( 194, 1925, 7, 0.566667 );
        --060b (end 060e): script:wait( 0.666667 )
        --060e (end 060e): cl:set_default_animation( "0" ) -- speed=1
        --                 cl:play_animation( "0" )
        --0611 (end 0611): field:pc_lock(false);
        --0613 (end 0613): field:menu_lock(false);
        --0615 (end 0615): return
        self.cloud:set_solid( true )
        player_lock( false )
        return 0
    end,
    
    jumpto_right = function( self )
        player_lock( true )
        self.cloud:set_solid( false )
        self.cloud:set_rotation( 90 )
        self.cloud:move_auto_animation( false )
        self.cloud:play_animation_stop( "Jump" )
        script:wait( 0.5 ) -- wait to start of jump in animation
        self.cloud:jump_to_position( 0.386, 14.94, 17.2969, 0.567 ) -- triangle 24
        self.cloud:jump_sync()
        script:wait( 0.667 ) -- wait to end of animation
        self.cloud:move_auto_animation( true )
        self.cloud:set_solid( true )
        player_lock( false )
        return 0
    end,
    
    jumpto_left = function( self )
        player_lock( true )
        self.cloud:set_solid( false )
        self.cloud:set_rotation( 90 )
        self.cloud:move_auto_animation( false )
        self.cloud:play_animation_stop( "Jump" )
        script:wait( 0.5 ) -- wait to start of jump in animation
        self.cloud:jump_to_position( -2.89844, 15.3047, 16.5391, 0.567 ) -- triangle 24
        self.cloud:jump_sync()
        script:wait( 0.667 ) -- wait to end of animation
        self.cloud:move_auto_animation( true )
        self.cloud:set_solid( true )
        player_lock( false )
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

    --[[ Stop breaking gate for a while and talk. ]]
    main = function( self )
        if FFVII.Data.progress_game < 13 then
            self.jessie:set_move_auto_speed( 2.8125 )
            self.jessie:move_to_position( 2.97656, -0.257813 )
            self.jessie:move_sync()
            self.jessie:move_to_position( 5.17969, 6.03906 )
            self.jessie:move_sync()
            self.jessie:move_to_position( 6.14063, 6.39063 )
            self.jessie:move_sync()
            self.jessie:set_move_auto_speed( 3.75 )
            FFVII.Data.progress_game = 13
            self.jessie:move_to_position( 6.20313, -1 )
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
            --load_field_map_request( "ffvii_elevtr1", "Spawn_elevtr1_1f" )
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
