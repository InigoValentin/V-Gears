if UiContainer == nil then UiContainer = {} end

--- The item menu.
UiContainer.EquipMenu = {

    --- Current cursor position in the equip slots (1-3).
    equip_position = 1,

    --- Max cursor position in the equip slots.
    equip_position_total = 3,

    --- ID of the character the menu is open for.
    char_id = -1,

    --- The type of item currently being selected.
    --
    -- Inventory.TYPE.WEAPON, Inventory.TYPE.ARMOR, or Inventory.TYPE.ACCESSORY.
    selecting_slot = 0,

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
        if UiContainer.current_menu == "equip" then
            if UiContainer.current_submenu == "" then
                if button == "Escape" and event == "Press" then
                    UiContainer.current_menu = "main"
                    script:request_end_sync(Script.UI, "EquipMenu", "hide", 0)
                elseif button == "Down" then
                    self.equip_position = self.equip_position + 1
                    if self.equip_position > self.equip_position_total then
                        self.equip_position = 1
                    end
                    ui_manager:get_widget("EquipMenu.Container.Character.Cursor"):set_default_animation("Position" .. self.equip_position)
                    self.populate_details(self, false)
                elseif button == "Up" then
                    self.equip_position = self.equip_position - 1
                    if self.equip_position < 1 then
                        self.equip_position = self.equip_position_total
                    end
                    ui_manager:get_widget("EquipMenu.Container.Character.Cursor"):set_default_animation("Position" .. self.equip_position)
                    self.populate_details(self, false)
                elseif button == "Enter" then
                    if self.equip_position == 1 then
                        self.selecting_slot = Inventory.TYPE.WEAPON
                    elseif self.bar_position == 2 then
                        self.selecting_slot = Inventory.TYPE.ARMOR
                    elseif self.bar_position == 3 then
                        self.selecting_slot = Inventory.TYPE.ACCESSORY
                    end
                    self.submenu_select(self)
                else
                    return 0
                end
            end
        end
        return 0
    end,

    --- Opens the equip menu.
    --
    -- Populates and updates displayed data before opening.
    show = function(self)
        self.char_id = UiContainer.current_character_id
        ui_manager:get_widget("EquipMenu"):set_visible(true)
        UiContainer.current_menu = "equip"
        UiContainer.current_submenu = ""
        self.equip_position = 1
        UiContainer.populate_character_data("EquipMenu.Container.Character", Characters[self.char_id])
        ui_manager:get_widget("EquipMenu.Container.Character.Portrait"):set_image("images/characters/" .. tostring(self.char_id) .. ".png")
        -- TODO: Do something for chars 9 and 10
        ui_manager:get_widget("EquipMenu.Container.Character.WpnLbl"):set_text(Game.Items[Characters[self.char_id].weapon.id].name)
        ui_manager:get_widget("EquipMenu.Container.Character.ArmLbl"):set_text(Game.Items[Characters[self.char_id].armor.id].name)
        if Characters[self.char_id].accessory == nil then
            ui_manager:get_widget("EquipMenu.Container.Character.AccLbl"):set_text("")
        else
            ui_manager:get_widget("EquipMenu.Container.Character.AccLbl"):set_text(Game.Items[Characters[self.char_id].accessory].name)
        end
        self.populate_details(self, false)

        --self.populate_stats(self)
        return 0;
    end,

    --- Populated the details of the highlighted item.
    --
    -- @param from_list If true, data will be read from the item higlighted in the list on the
    -- right. If false, data wil be red from the currently equipped items.
    populate_details = function(self, from_list)
        local type
        local item
        if (from_list == false) then
            if self.equip_position == 1 then
                type = Inventory.ITEM_TYPE.WEAPON
                item = Game.Items[Characters[self.char_id].weapon.id]
                ui_manager:get_widget("EquipMenu.Container.Help.Help"):set_text(Game.Items[Characters[self.char_id].weapon.id].description)
            elseif self.equip_position == 2 then
                type = Inventory.ITEM_TYPE.ARMOR
                item = Game.Items[Characters[self.char_id].armor.id]
                ui_manager:get_widget("EquipMenu.Container.Help.Help"):set_text(Game.Items[Characters[self.char_id].armor.id].description)
            elseif self.equip_position == 3 then
                type = Inventory.ITEM_TYPE.ACCESSORY
                item = Game.Items[Characters[self.char_id].accessory]
            elseif self.equip_position == 3 and Characters[self.char_id].accessory ~= nil then
                ui_manager:get_widget("EquipMenu.Container.Help.Help"):set_text(Game.Items[Characters[self.char_id].accessory].description)
            end
            -- Show description
            if item ~= nil then
                ui_manager:get_widget("EquipMenu.Container.Help.Help"):set_text(item.description)
            else
                ui_manager:get_widget("EquipMenu.Container.Help.Help"):set_text("")
            end
            -- If accessory, hide all details and return
            if type == Inventory.ITEM_TYPE.ACCESSORY then
                ui_manager:get_widget("EquipMenu.Container.SlotsEmpty"):set_visible(true)
                return 0
            end
            ui_manager:get_widget("EquipMenu.Container.SlotsEmpty"):set_visible(false)
            ui_manager:get_widget("EquipMenu.Container.Slots.Growth1"):set_visible(false)
            ui_manager:get_widget("EquipMenu.Container.Slots.Growth2"):set_visible(false)
            ui_manager:get_widget("EquipMenu.Container.Slots.Growth3"):set_visible(false)
            if item.growth >= 1 and item.growth <= 3 then
                ui_manager:get_widget("EquipMenu.Container.Slots.Growth" .. item.growth):set_visible(true)
            else
                ui_manager:get_widget("EquipMenu.Container.Slots.Growth1"):set_visible(true)
            end
            for s = 1, 8 do
                if #(item.slots) < s then
                    ui_manager:get_widget("EquipMenu.Container.Slots.Slot" .. tostring(s)):set_visible(false)
                else
                    ui_manager:get_widget("EquipMenu.Container.Slots.Slot" .. tostring(s)):set_visible(true)
                    if item.growth == 0 then
                        ui_manager:get_widget("EquipMenu.Container.Slots.Slot" .. tostring(s)):set_image("images/icons/materia_slot_no_growth.png")
                    else
                        ui_manager:get_widget("EquipMenu.Container.Slots.Slot" .. tostring(s)):set_image("images/icons/materia_slot.png")
                    end
                end
                if s % 2 ~= 0 then
                    -- At every odd slot, if there is a next slot and its linked, show linker.
                    if #(item.slots) > s  and item.slots[s] == 1 then
                        ui_manager:get_widget("EquipMenu.Container.Slots.Link" .. tostring(math.floor(s / 2) + 1)):set_visible(true)
                    else
                        ui_manager:get_widget("EquipMenu.Container.Slots.Link" .. tostring(math.floor(s / 2) + 1)):set_visible(false)
                    end
                end
            end
        elseif from_list == true then
            --TODO: Implement
        end
    end,

    --- Hides the item menu and goes back to the main menu.
    hide = function(self)
        ui_manager:get_widget("EquipMenu"):set_visible(false)
        UiContainer.current_menu = "main"
        UiContainer.current_submenu = ""
        return 0;
    end,
}
