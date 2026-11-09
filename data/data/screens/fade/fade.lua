if UiContainer == nil then UiContainer = {} end



UiContainer.Fade = {
    fade_in = function( self )
        local fade = ui_manager:get_widget( "Fade.Screen" )
        fade:play_animation_stop( "FadeIn" )
        fade:animation_sync()

        return 0;
    end,



    --- Transition from the field or the world map to the battle in Battle.pending_id.
    --
    -- The screen flashes white and the battle starts under the flash, which BattleUi.show reveals.
    battle_transition = function( self )
        -- The player can't move during the transition. It's unlocked when the battle ends, unless
        -- the player was already locked.
        Battle.locked_by_transition = not entity_manager:is_player_locked()
        player_lock( true )
        local fade = ui_manager:get_widget( "Fade.Screen" )
        fade:set_colour( 1, 1, 1 )
        fade:play_animation_stop( "BattleFlash" )
        fade:animation_sync()
        battle_manager:start_battle( Battle.pending_id )
        -- The battle takes over the screen, so the cover of the field goes away.
        fade:set_alpha( 0 )
        fade:set_colour( 0, 0, 0 )
        UiContainer.BattleUi.show( UiContainer.BattleUi )
        return 0
    end,

    fade_out = function( self )
        local fade = ui_manager:get_widget( "Fade.Screen" )
        fade:play_animation_stop( "FadeOut" )
        fade:animation_sync()

        return 0;
    end,
}
