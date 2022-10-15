if UiContainer == nil then UiContainer = {} end

--- The main menu.
UiContainer.MainMenu = {
    position = 1,
    position_total = 10,

    on_start = function(self)
        return 0
    end,

    --- Populates a character data.
    --
    -- @param row The row the character is in (1-3).
    -- @param character The character to get the data from. If nill, the character details will be
    -- hidden.
    populate_character = function(row, character)
        local widget = "MainMenu.Container.Characters.Character" .. tostring(row)
        if character == nil then
            ui_manager:get_widget(widget):set_visible(false)
            return 0
        end

        -- TODO: When MAX HP and MP values are fixed, use them instead of base.

        local char_id = character.char_id
        ui_manager:get_widget(widget):set_visible(true)

        -- Critical?
        if character.stats.hp.current == 0 then
            ui_manager:get_widget(widget .. ".Data.HpCurrent"):set_colour(1.0, 0, 0)
        elseif character.stats.hp.current <= character.stats.hp.base / 4 then
            ui_manager:get_widget(widget .. ".Data.HpCurrent"):set_colour(1.0, 1.0, 0)
        else
            ui_manager:get_widget(widget .. ".Data.HpCurrent"):set_colour(0.9, 0.9, 0.9)
        end

        -- Text data
        ui_manager:get_widget(widget .. ".Data.Name"):set_text(character.name)
        ui_manager:get_widget(widget .. ".Data.LvNumber"):set_text(tostring(character.level))
        ui_manager:get_widget(widget .. ".Data.HpCurrent"):set_text(tostring(character.stats.hp.current))
        ui_manager:get_widget(widget .. ".Data.HpMax"):set_text(tostring(character.stats.hp.base))
        ui_manager:get_widget(widget .. ".Data.MpCurrent"):set_text(tostring(character.stats.mp.current))
        ui_manager:get_widget(widget .. ".Data.MpMax"):set_text(tostring(character.stats.mp.base))

        -- Calculate the HP and MP bar.
        local max_hp_width= ui_manager:get_widget(widget .. ".Data.HpLine"):get_width()
        local hp_width = character.stats.hp.current * max_hp_width / character.stats.hp.base
        ui_manager:get_widget(widget .. ".Data.HpLineCurrent"):set_width(hp_width)
        local max_mp_width= ui_manager:get_widget(widget .. ".Data.MpLine"):get_width()
        local mp_width = character.stats.mp.current * max_mp_width / character.stats.mp.base
        ui_manager:get_widget(widget .. ".Data.MpLineCurrent"):set_width(mp_width)

        return 0
    end,



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
    -- Populates and updates displayed data before opening.
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
                self.populate_character(c, Characters[FFVII.Party[c]])
            else
                self.populate_character(c, nil)
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
