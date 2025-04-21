if UiContainer == nil then UiContainer = {} end

--- The pause menu
UiContainer.PauseMenu = {

    --- Cursor position.
    position = 1,

    --- Maximum cursor position
    position_total = 2,

    --- Run on creation, does nothing.
    on_start = function( self )
        return 0
    end,

    --- Handles button events.
    --
    -- If the game is paused, it handles the navigation in the pause menu. If the game is not
    -- paused, it only handles the pause event
    on_button = function( self, button, event )
        if System:is_paused() == true then
            local menu_cursor = ui_manager:get_widget( "PauseMenu.Menu.Cursor" )
            if button == Config.CONTROLS.PAUSE and event == "Press" then
                System:unpause()
            elseif button == "Enter" and event == "Press" then
                audio_manager:play_sound("Cursor")
                script:request_end_sync( Script.UI, "PauseMenu", "hide", 0 )
                if self.position == 2 then
                    script:request_end_sync( Script.UI, "BeginMenu", "show", 0 )
                    map("empty")
                    MenuSettings.pause_available = false
                else
                    MenuSettings.available = true
                end
            elseif button == "Down" then
                audio_manager:play_sound("Cursor")
                self.position = self.position + 1
                if self.position > self.position_total then
                    self.position = 1;
                end
                menu_cursor:set_default_animation( "Position" .. self.position )
            elseif button == "Up" then
                audio_manager:play_sound("Cursor")
                self.position = self.position - 1
                if self.position <= 0 then
                    self.position = self.position_total;
                end
                menu_cursor:set_default_animation( "Position" .. self.position )
            end
        elseif System:is_paused() == false then
            if button == Config.CONTROLS.PAUSE and event == "Press" then
                System:pause()
            end
        end

        return 0
    end,

    --- Shows the pause menu.
    show = function( self )
        ui_manager:get_widget("PauseMenu"):set_visible(true)
    end,

    --- Hides the pause menu
    hide = function(self)
        ui_manager:get_widget("PauseMenu"):set_visible(false)
    end,
}
