if UiContainer == nil then UiContainer = {} end

--- The save/load menu.
UiContainer.SaveMenu = UiContainer.Menu.new{

    id = "save",

    --- Current operation ("save/load")
    operation = "save",

    --- Maximum number of save slots.
    max_slots = 15,

    --- Cursor over the three visible slots, scrolling over all of them.
    cursor = UiContainer.ListCursor.new{
        widget = "SaveMenu.Container.Cursor",
        rows = 3,
        count = 15,
        on_scroll = function() UiContainer.SaveMenu:populate_slots() end,
    },

    states = {
        [""] = {
            cursor = function(self) return self.cursor end,
            axis = "vertical",
            cancel = function(self)
                script:request_end_sync(Script.UI, "SaveMenu", "hide", 0)
            end,
            -- The action plays its own sound.
            confirm_sound = false,
            confirm = function(self, cursor)
                self:action(cursor:index())
            end,
        },
    },

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
        self:activate()
        local loading = (self.operation == "load")
        local container = "SaveMenu.Container."
        ui_manager:get_widget(container .. "TopBar.DescSave"):set_visible(not loading)
        ui_manager:get_widget(container .. "TopBar.DescLoad"):set_visible(loading)
        ui_manager:get_widget(container .. "Title.TitleSave"):set_visible(not loading)
        ui_manager:get_widget(container .. "Title.TitleLoad"):set_visible(loading)
        ui_manager:get_widget("SaveMenu"):set_visible(true)
        self.cursor.count = self.max_slots
        self.cursor:reset()
        self:populate_slots()
        return 0;
    end,

    populate_slots = function(self)
        for s = 1, 3 do
            local slot = self.cursor.first + s - 2 -- 0-index
            if savemap_manager:is_slot_empty(slot) == true then
                ui_manager:get_widget("SaveMenu.Container.Slot" .. tostring(s) .. ".Slot"):set_visible(false)
                ui_manager:get_widget("SaveMenu.Container.Slot" .. tostring(s) .. ".Empty"):set_visible(true)
            else
                local slot_id = "SaveMenu.Container.Slot" .. tostring(s) .. ".Slot."
                ui_manager:get_widget(slot_id .. "TimeMoney.Money"):set_text(UiContainer.format_money(savemap_manager:get_slot_money(slot), 7))
                local seconds = savemap_manager:get_slot_game_time(slot)
                local time = UiContainer.format_time(seconds)
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
                        ui_manager:get_widget(slot_id .. "Party.Portrait" .. tostring(c)):set_image("characters/" .. tostring(save_party[c]) .. ".png")
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
            end
            save_game(slot - 1, true)
            audio_manager:play_sound("Cursor")
            self:populate_slots()
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
        self:deactivate(self.operation == "load" and "begin" or "main")
        savemap_manager:release()
        return 0;
    end,
}
