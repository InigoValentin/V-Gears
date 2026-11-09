if UiContainer == nil then UiContainer = {} end

--- The game over screen.
--
-- Shown when the whole party dies in battle. It fades to black, plays the game over music, and
-- when the player confirms, goes back to the begin menu.
UiContainer.GameOver = {

    --- Music to play. Name as in the musics file (57 is the game over music of the original game).
    MUSIC = "57",

    --- Indicates that the screen is being shown.
    active = false,

    --- Indicates that the screen is fully shown and can be dismissed.
    ready = false,

    --- Shows the game over screen.
    show = function(self)
        self.active = true
        self.ready = false
        UiContainer.current_menu = "game_over"
        ui_manager:get_widget("GameOver.Container.Text"):set_visible(false)
        ui_manager:get_widget("GameOver"):set_visible(true)
        audio_manager:play_music(self.MUSIC)
        local background = ui_manager:get_widget("GameOver.Background")
        background:play_animation_stop("FadeIn")
        background:animation_sync()
        ui_manager:get_widget("GameOver.Container.Text"):set_visible(true)
        self.ready = true
        return 0
    end,

    --- Handles button events.
    --
    -- Confirming, once the screen is shown, leaves it.
    on_button = function(self, button, event)
        if self.active == false or self.ready == false then do return 0 end end
        if button == Config.ACTIONS.CONFIRM then self:leave() end
        return 0
    end,

    --- Leaves the game over screen and goes back to the begin menu.
    --
    -- TODO: Reload the last save, as the original game does. For now the party is healed, so the
    -- debug battles of the begin menu can be started again.
    leave = function(self)
        self.active = false
        self.ready = false
        ui_manager:get_widget("GameOver.Container.Text"):set_visible(false)
        ui_manager:get_widget("GameOver"):set_visible(false)
        for i = 1, Party.MAX do
            if Party[i] ~= nil then
                local stats = Characters[Party[i]].stats
                stats.hp.current = stats.hp.max or stats.hp.base
            end
        end
        Battle.finish()
        MenuSettings.pause_available = false
        UiContainer.current_menu = ""
        script:request_end_sync(Script.UI, "BeginMenu", "show", 0)
        return 0
    end,
}
