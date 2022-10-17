if UiContainer == nil then UiContainer = {} end

--- The main menu.
UiContainer.MainMenu = {

    --- Current cursor position in the main menu (1-10).
    position = 1,

    -- Max cursor position in the main menu.
    position_total = 10,

    --- Run when the menu is creaded.
    --
    -- It does nothing.
    on_start = function(self)
        return 0
    end,

    --- Handles button events.
    --
    -- For the current submenu, handles directional keys, enter and escape events.
    -- @param button Pressed button string key. "Up", "Left", "Enter" and "Escape" are handled.
    -- @param event Trigger event. Normally, "Press".
    on_button = function(self, button, event)
        local characters  = ui_manager:get_widget("MainMenu.Container.Characters")
        local menu  = ui_manager:get_widget("MainMenu.Container.Menu")
        local menu_cursor = ui_manager:get_widget("MainMenu.Container.Menu.Cursor")
        local timegil = ui_manager:get_widget("MainMenu.Container.TimeGil")
        local location = ui_manager:get_widget("MainMenu.Container.Location")
        if UiContainer.current_menu == "main" then
            if UiContainer.current_submenu == "" then
                if button == "Escape" and event == "Press" then
                    script:request_end_sync(Script.UI, "MainMenu", "hide", 0)
                elseif button == "Enter" and event == "Press" and self.position == 1 then
                    script:request_end_sync(Script.UI, "ItemMenu", "show", 0)
                elseif button == "Down" then
                    self.position = self.position + 1
                    if self.position > self.position_total then
                        self.position = 1;
                    end
                    menu_cursor:set_default_animation("Position" .. self.position)
                elseif button == "Up" then
                    self.position = self.position - 1
                    if self.position <= 0 then
                        self.position = self.position_total;
                    end
                    menu_cursor:set_default_animation("Position" .. self.position)
                end
            end
        elseif ui_manager:get_widget("MainMenu"):is_visible() == false and FFVII.MenuSettings.available == true then
            if button == "Escape" and event == "Press" then
                script:request_end_sync(Script.UI, "MainMenu", "show", 0)
            end
        else
            return 0
        end

        return 0
    end,

    --- Opens the main menu.
    --
    -- Populates and updates displayed data before opening. It also pauses the game.
    show = function(self)
        UiContainer.current_menu = "main"
        UiContainer.current_submenu = ""
        entity_manager:set_paused(true)
        FFVII.MenuSettings.pause_available = false
        ui_manager:get_widget("MainMenu"):set_visible(true)
        local characters  = ui_manager:get_widget("MainMenu.Container.Characters")
        local menu = ui_manager:get_widget("MainMenu.Container.Menu")
        local menu_cursor = ui_manager:get_widget("MainMenu.Container.Menu.Cursor")
        local timegil = ui_manager:get_widget("MainMenu.Container.TimeGil")
        local location = ui_manager:get_widget("MainMenu.Container.Location")

        for c = 1, 3 do
            if FFVII.Party[c] ~= nil then
                UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(c), Characters[FFVII.Party[c]])
            else
                UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(c), nil)
            end
        end

        --ui_manager:get_widget("MainMenu.Container.Menu.PHSText"):set_visible(false)
        ui_manager:get_widget("MainMenu.Container.Menu.SaveText"):set_colour(0.4, 0.4, 0.4)

        characters:play_animation_stop("Appear")
        menu:play_animation_stop("Appear")
        menu_cursor:set_default_animation("Position" .. self.position)
        timegil:play_animation_stop("Appear")
        location:play_animation_stop("Appear")
        location:animation_sync()
        return 0;
    end,

    --- Hides the item menu and goes back to the game.
    --
    -- It also unpauses the game.
    hide = function(self)
        UiContainer.current_menu = ""
        UiContainer.current_submenu = ""
        local characters  = ui_manager:get_widget("MainMenu.Container.Characters")
        local menu = ui_manager:get_widget("MainMenu.Container.Menu")
        local timegil = ui_manager:get_widget("MainMenu.Container.TimeGil")
        local location = ui_manager:get_widget("MainMenu.Container.Location")

        characters:play_animation_stop("Disappear")
        menu:play_animation_stop("Disappear")
        timegil:play_animation_stop("Disappear")
        location:play_animation_stop("Disappear")
        location:animation_sync()

        ui_manager:get_widget("MainMenu"):set_visible(false)

        FFVII.MenuSettings.pause_available = true
        entity_manager:set_paused(false)

        return 0;
    end,
}
