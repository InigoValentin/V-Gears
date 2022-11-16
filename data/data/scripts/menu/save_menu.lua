if UiContainer == nil then UiContainer = {} end

--- The item menu.
UiContainer.SaveMenu = {

    --- Current operation ("save/load")
    operation = "save",

    --- Current cursor position in the top bar (1-3).
    position = 1,

    --- Max cursor position in the top bar.
    position_total = 3,

    --- Maximum scroll position.
    max_slots = 15,

    --- First visible save slot.
    first_visible = 1,

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
        if UiContainer.current_menu == "save" then
            if UiContainer.current_submenu == "" then
                if button == "Escape" and event == "Press" then
                    audio_manager:play_sound("Back")
                    script:request_end_sync(Script.UI, "SaveMenu", "hide", 0)
                elseif button == "Enter" then
                    self.action(self, self.first_visible - 1 + self.position)
                elseif button == "Down" then
                    if self.position < self.position_total and self.first_visible + self.position < self.max_slots then
                        -- Move cursor down
                        self.position = self.position + 1
                        ui_manager:get_widget("SaveMenu.Container.Cursor"):set_default_animation("Position" .. self.position)
                        audio_manager:play_sound("Cursor")
                    elseif self.position == self.position_total and self.first_visible + self.position < self.max_slots then
                        self.first_visible = self.first_visible + 1
                        self.populate_slots(self)
                        audio_manager:play_sound("Cursor")
                    else
                        audio_manager:play_sound("Error")
                    end
                elseif button == "Up" then
                    if self.position > 1 then
                        -- Move cursor up
                        self.position = self.position - 1
                        ui_manager:get_widget("SaveMenu.Container.Cursor"):set_default_animation("Position" .. self.position)
                        audio_manager:play_sound("Cursor")
                    elseif self.position == 1 and self.first_visible > 1 then
                        self.first_visible = self.first_visible - 1
                        self.populate_slots(self)
                        audio_manager:play_sound("Cursor")
                    else
                        audio_manager:play_sound("Error")
                    end
                end
            elseif UiContainer.current_submenu == "confirm" then
            end
        end
        return 0
    end,

    --- Opens the save menu.
    --
    -- Populates and updates displayed data before opening.
    show = function(self)
        if self.operation ~= "save" and self.operation ~= "load" then
            print("Error: Tried to open the save menu without setting an operation.")
            print("Do:")
            print("    UiContainer.SaveMenu.operation = \"save\"")
            print("or")
            print("    UiContainer.SaveMenu.operation = \"load\"")
            print("before calling show()")
            return 0
        end
        UiContainer.current_menu = "save"
        UiContainer.current_submenu = ""
        if self.operation == "load" then
            ui_manager:get_widget("SaveMenu.Container.TopBar.DescSave"):set_visible(false)
            ui_manager:get_widget("SaveMenu.Container.TopBar.DescLoad"):set_visible(true)
            ui_manager:get_widget("SaveMenu.Container.Title.TitleSave"):set_visible(false)
            ui_manager:get_widget("SaveMenu.Container.Title.TitleLoad"):set_visible(true)
        else
            ui_manager:get_widget("SaveMenu.Container.TopBar.DescSave"):set_visible(true)
            ui_manager:get_widget("SaveMenu.Container.TopBar.DescLoad"):set_visible(false)
            ui_manager:get_widget("SaveMenu.Container.Title.TitleSave"):set_visible(true)
            ui_manager:get_widget("SaveMenu.Container.Title.TitleLoad"):set_visible(false)
        end
        ui_manager:get_widget("SaveMenu"):set_visible(true)
        UiContainer.current_menu = "save"
        self.position = 1
        self.first_visible = 1
        ui_manager:get_widget("SaveMenu.Container.Cursor"):set_default_animation("Position" .. self.position)
        self.populate_slots(self)
        return 0;
    end,

    populate_slots = function(self)
        for s = 1, 3 do
            local slot = self.first_visible + s - 2 -- 0-index
            if savemap_manager:is_slot_empty(slot) == true then
                ui_manager:get_widget("SaveMenu.Container.Slot" .. tostring(s) .. ".Slot"):set_visible(false)
                ui_manager:get_widget("SaveMenu.Container.Slot" .. tostring(s) .. ".Empty"):set_visible(true)
            else
                local slot_id = "SaveMenu.Container.Slot" .. tostring(s) .. ".Slot."
                ui_manager:get_widget(slot_id .. "TimeMoney.Money"):set_text(UiContainer.pad_value(savemap_manager:get_slot_money(slot), 7))
                local seconds = savemap_manager:get_slot_game_time(slot)
                local time = UiContainer.pad_value(math.floor(seconds / 3600), 3) .. ":" .. UiContainer.zero_pad_value(math.floor((seconds % 3600) / 60), 2)
                ui_manager:get_widget(slot_id .. "TimeMoney.Time"):set_text(time)
                ui_manager:get_widget(slot_id .. "Location.Name"):set_text(savemap_manager:get_slot_location_name(slot))
                local save_party = {
                    [1] = savemap_manager:get_slot_party_member(slot, 0),
                    [2] = savemap_manager:get_slot_party_member(slot, 1),
                    [3] = savemap_manager:get_slot_party_member(slot, 2)
                }
                ui_manager:get_widget(slot_id .. "Name"):set_text(savemap_manager:get_slot_character_name(slot, save_party[1]))
                ui_manager:get_widget(slot_id .. "LvNumber"):set_text(UiContainer.pad_value(savemap_manager:get_slot_character_level(slot, save_party[1]), 3))
                for c = 1, 3 do
                    if save_party[c] ~= -1 and Characters[save_party[c] + 1] ~= nil then
                        ui_manager:get_widget(slot_id .. "Party.Portrait" .. tostring(c)):set_image("images/characters/" .. tostring(save_party[c]) .. ".png")
                        ui_manager:get_widget(slot_id .. "Party.Portrait" .. tostring(c)):set_visible(true)
                    else
                        ui_manager:get_widget(slot_id .. "Party.Portrait" .. tostring(c)):set_visible(false)
                    end
                end
                ui_manager:get_widget("SaveMenu.Container.Slot" .. tostring(s) .. ".Slot"):set_visible(true)
                ui_manager:get_widget("SaveMenu.Container.Slot" .. tostring(s) .. ".Empty"):set_visible(false)
            end
        end
    end,

    action = function(self, slot)
        if self.operation == "save" then
            if savemap_manager:is_slot_empty(slot - 1) == false and ControlKey ~= savemap_manager:get_slot_control_key(slot - 1) then
                -- TODO: Confirmation dialog
                print("OVERRIDE ANOTHER SAVE")
                save_game(slot - 1, true)
                audio_manager:play_sound("Cursor")
                self.populate_slots(self)
            else
                save_game(slot - 1, true)
                audio_manager:play_sound("Cursor")
                self.populate_slots(self)
            end
        elseif self.operation == "load" then
            audio_manager:play_sound("Window")
            script:request_end_sync(Script.UI, "SaveMenu", "hide", 0)
            script:request_end_sync(Script.UI, "BeginMenu", "hide", 0)
            load_game(slot - 1)
        end
    end,

    --- Hides the item menu and goes back to the main menu.
    hide = function(self)
        ui_manager:get_widget("SaveMenu"):set_visible(false)
        if self.operation == "load" then
            UiContainer.current_menu = "begin"
        else
            UiContainer.current_menu = "main"
        end
        UiContainer.current_submenu = ""
        savemap_manager:release()
        return 0;
    end,
}
