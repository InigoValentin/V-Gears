if UiContainer == nil then UiContainer = {} end

--- The pause menu.
--
-- Unlike the other menus it isn't driven by UiContainer.current_menu: it reacts to the START
-- button while the game is paused.
UiContainer.PauseMenu = UiContainer.Menu.new{

    id = "pause",

    --- Cursor over the pause options. 1: Continue, 2: Exit to the begin menu.
    cursor = UiContainer.Cursor.new{
        widget = "PauseMenu.Menu.Cursor",
        total = 2,
    },

    --- Handles button events.
    --
    -- If the game is paused, it handles the navigation in the pause menu. If the game is not
    -- paused, it only handles the pause event.
    on_button = function(self, button, event)
        if System:is_paused() == false then
            if button == Config.ACTIONS.START and event == "Press" then
                System:pause()
            end
            return 0
        end

        if button == Config.ACTIONS.START and event == "Press" then
            System:unpause()
        elseif button == Config.ACTIONS.CONFIRM and event == "Press" then
            UiContainer.play_sound("Cursor")
            script:request_end_sync(Script.UI, "PauseMenu", "hide", 0)
            if self.cursor:index() == 2 then
                script:request_end_sync(Script.UI, "BeginMenu", "show", 0)
                map("empty")
                MenuSettings.pause_available = false
            else
                MenuSettings.available = true
            end
        else
            self.cursor:handle(button, "vertical")
        end
        return 0
    end,

    --- Shows the pause menu.
    show = function(self)
        ui_manager:get_widget("PauseMenu"):set_visible(true)
    end,

    --- Hides the pause menu
    hide = function(self)
        ui_manager:get_widget("PauseMenu"):set_visible(false)
    end,
}
