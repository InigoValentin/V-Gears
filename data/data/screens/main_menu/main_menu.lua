if UiContainer == nil then UiContainer = {} end

--- The main menu.
UiContainer.MainMenu = {

    --- Current cursor position in the main menu (1-10).
    position = 1,

    -- Max cursor position in the main menu.
    position_total = 10,

    --- When selecting a character, the menu in this position will be opened.
    --
    -- If 0, not selecting character.
    select_character_for_menu = 0,

    --- Current cursorposition in the charaster list (1-3).
    character_position = 1,

    --- Max cursor position in the characetr list.
    character_position_total = 3,

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
                elseif button == "Enter" and event == "Press" then
                    if self.position == 1 then -- Item menu
                        script:request_end_sync(Script.UI, "ItemMenu", "show", 0)
                    elseif self.position == 8 then -- Config menu
                        print("TODO: Open config menu")
                    elseif self.position == 9 then -- PHS menu
                        print("TODO: Open PHS menu")
                    elseif self.position == 10 then -- Save menu
                        print("TODO: Open save menu")
                    else -- Any other menu that needs a character
                        self.select_character_for_menu = self.position
                        UiContainer.current_submenu = "main_character"
                        ui_manager:get_widget("MainMenu.Container.Characters.Cursor"):set_visible(true)
                    end
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
            elseif UiContainer.current_submenu == "main_character" then
                if button == "Escape" and event == "Press" then
                    UiContainer.current_submenu = ""
                    ui_manager:get_widget("MainMenu.Container.Characters.Cursor"):set_visible(false)
                elseif button == "Down" then
                    -- TODO: Skip empty character slots
                    self.character_position = self.character_position + 1
                    if self.character_position > self.character_position_total then
                        self.character_position = 1
                    end
                    ui_manager:get_widget("MainMenu.Container.Characters.Cursor"):set_default_animation("Position" .. self.character_position)
                elseif button == "Up" then
                    -- TODO: Skip empty character slots
                    self.character_position = self.character_position - 1
                    if self.character_position < 1 then
                        self.character_position = self.character_position_total
                    end
                    ui_manager:get_widget("MainMenu.Container.Characters.Cursor"):set_default_animation("Position" .. self.character_position)
                elseif button == "Enter" then
                    if self.select_character_for_menu == 2 then -- Magic menu
                        print("Open magic menu for char in slot " .. self.character_position)
                    elseif self.select_character_for_menu == 3 then -- Materia menu
                        print("Open materia menu for char in slot " .. self.character_position)
                    elseif self.select_character_for_menu == 4 then -- Equip menu
                        print("Open equip menu for char in slot " .. self.character_position)
                    elseif self.select_character_for_menu == 5 then -- Status menu
                        print("Open status menu for char in slot " .. self.character_position)
                    elseif self.select_character_for_menu == 6 then -- Order menu
                        print("Open order menu for char in slot " .. self.character_position)
                    elseif self.select_character_for_menu == 7 then -- Limit menu
                        print("Open limit menu for char in slot " .. self.character_position)
                    end
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
                if Characters[FFVII.Party[c]].back_row == 1 then
                    ui_manager:get_widget("MainMenu.Container.Characters.Character" .. tostring(c) .. ".Portrait"):set_default_animation("RowBack")
                else
                    ui_manager:get_widget("MainMenu.Container.Characters.Character" .. tostring(c) .. ".Portrait"):set_default_animation("RowFront")
                end
            else
                UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(c), nil)
            end
        end

        -- TODO: Set portraits of the correct char

        --ui_manager:get_widget("MainMenu.Container.Menu.PHSText"):set_visible(false)
        ui_manager:get_widget("MainMenu.Container.Menu.SaveText"):set_colour(0.4, 0.4, 0.4)

        -- Set money
        local money_str = tostring(Inventory.money)
        while #(money_str) < 8 do
            money_str = " " .. money_str
        end
        ui_manager:get_widget("MainMenu.Container.TimeGil.Gil"):set_text(money_str)

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
