if UiContainer == nil then UiContainer = {} end

--- The main menu.
UiContainer.BattleUi = {

    --- Current cursor position in the main menu (1-10).
    position = 1,

    -- Max cursor position in the main menu.
    position_total = 11,

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
        print("BattleUi start")
        return 0
    end,

    --- Handles button events.
    --
    -- For the current submenu, handles directional keys, enter and escape events.
    -- @param button Pressed button string key. "Up", "Left", "Enter" and "Escape" are handled.
    -- @param event Trigger event. Normally, "Press".
    on_button = function(self, button, event)
        return 0
    end,

    --- Shows the battle UI.
    --
    -- Populates and updates displayed data.
    show = function(self)
        self:populate()
        print("POPULATED")
        ui_manager:get_widget("BattleUi"):set_visible(true)
        --local characters  = ui_manager:get_widget("MainMenu.Container.Characters")
        --local menu = ui_manager:get_widget("MainMenu.Container.Menu")
        --local menu_cursor = ui_manager:get_widget("MainMenu.Container.Menu.Cursor")
        --local timegil = ui_manager:get_widget("MainMenu.Container.TimeGil")
        --local location = ui_manager:get_widget("MainMenu.Container.Location")


        --for c = 1, 3 do
            --if Party[c] ~= nil then
                --UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(c), Characters[Party[c]])
                --ui_manager:get_widget("MainMenu.Container.Characters.Character" .. tostring(c) .. ".Portrait"):set_image("characters/" .. tostring(Party[c]) .. ".png")
                --if Characters[Party[c]].back_row == 1 then
                    --ui_manager:get_widget("MainMenu.Container.Characters.Character" .. tostring(c) .. ".Portrait"):set_default_animation("RowBack")
                --else
                   --ui_manager:get_widget("MainMenu.Container.Characters.Character" .. tostring(c) .. ".Portrait"):set_default_animation("RowFront")
                --end
                --ui_manager:get_widget("MainMenu.Container.Characters.Character" .. tostring(c) .. ".LimitLevel"):set_text(tostring(Characters[Party[c]].limit.current))
            --else
                --UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(c), nil)
            --end
        --end

        -- TODO: Set portraits of the correct char

        --ui_manager:get_widget("MainMenu.Container.Menu.PHSText"):set_visible(false)
        --ui_manager:get_widget("MainMenu.Container.Menu.SaveText"):set_colour(0.4, 0.4, 0.4)

        -- Set location
        --ui_manager:get_widget("MainMenu.Container.Location.Text"):set_text(System["MapChanger"].location_name)

        -- Set money
        --local money_str = tostring(Inventory.money)
        --while #(money_str) < 8 do
        --    money_str = " " .. money_str
        --end
        --ui_manager:get_widget("MainMenu.Container.TimeGil.Gil"):set_text(money_str)

        --characters:play_animation_stop("Appear")
        --menu:play_animation_stop("Appear")
        --menu_cursor:set_default_animation("Position" .. self.position)
        --timegil:play_animation_stop("Appear")
        --location:play_animation_stop("Appear")
        --location:animation_sync()
        return 0;
    end,

    --- Hides the battle UI.
    --
    -- It also unpauses the game.
    hide = function(self)
        --local characters  = ui_manager:get_widget("MainMenu.Container.Characters")
        --local menu = ui_manager:get_widget("MainMenu.Container.Menu")
        --local timegil = ui_manager:get_widget("MainMenu.Container.TimeGil")
        --local location = ui_manager:get_widget("MainMenu.Container.Location")

        --characters:play_animation_stop("Disappear")
        --menu:play_animation_stop("Disappear")
        --timegil:play_animation_stop("Disappear")
        --location:play_animation_stop("Disappear")
        --location:animation_sync()

        ui_manager:get_widget("BattleUi"):set_visible(false)

        --MenuSettings.pause_available = true
        --entity_manager:set_paused(false)

        return 0;
    end,


    populate = function(self)

        space_pad = function(val, length)
            str = tostring(val)
            while #(str) < length do
                str = " " .. str
            end
            return str
        end

        print("POPULATING")

        for c = 1, 3 do
            print("POPULATING " .. tostring(c))
            if Party[c] ~= nil then
                print("POPULATING " .. tostring(c) .. " NOT NULL")
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c)):set_visible(true)
                print("POPULATING " .. tostring(c) .. " NAME")
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Left.Name"):set_text(Characters[Party[c]].name)
                print("POPULATING " .. tostring(c) .. " HP")
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Right.Hp.HpCurrent"):set_text(space_pad(Characters[Party[c]].stats.hp.current, 5))
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Right.Hp.HpMax"):set_text(space_pad(Characters[Party[c]].stats.hp.base, 5))
                print("POPULATING " .. tostring(c) .. " MP")
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Right.Mp.MpCurrent"):set_text(space_pad(Characters[Party[c]].stats.mp.current, 5))
            else
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c)):set_visible(false)
            end
        end
        print("POPULATING END")
        return 0
    end
}
