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
    -- Inventory.ITEM_TYPE.WEAPON, Inventory.ITEM_TYPE.ARMOR, or Inventory.ITEM_TYPE.ACCESSORY.
    selecting_slot = Inventory.ITEM_TYPE.WEAPON,

    --- List of owned weapons equipable by the current characters.
    --
    -- Only one of each, nevermind the owned quantity.
    avail_weapons = {},

    --- List of owned armors equipable by the current characters.
    --
    -- Only one of each, nevermind the owned quantity.
    avail_armors = {},

    --- List of owned accessories equipable by the current characters.
    --
    -- Only one of each, nevermind the owned quantity.
    avail_accessories = {},

    --- First visible position in the item list.
    --
    -- I.e. How many position the list is scrolled down + 1
    list_first_visible = 1,

    --- Cursor position in the list.
    list_position = 1,

    --- Maximum slots in the item list.
    list_position_total = 10,

    --- highlighted position in the current item list.
    list_item_selected = 1,

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
                -- TODO: Handle L1/R1 (change character)
                -- TODO: Handle square: Materia menu
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
                        self.selecting_slot = Inventory.ITEM_TYPE.WEAPON
                    elseif self.equip_position == 2 then
                        self.selecting_slot = Inventory.ITEM_TYPE.ARMOR
                    elseif self.equip_position == 3 then
                        self.selecting_slot = Inventory.ITEM_TYPE.ACCESSORY
                    end
                    self.submenu_select(self)
                else
                    return 0
                end
            elseif UiContainer.current_submenu == "item_select" then
                local list
                if self.selecting_slot == Inventory.ITEM_TYPE.WEAPON then
                    list = self.avail_weapons
                elseif self.selecting_slot == Inventory.ITEM_TYPE.ARMOR then
                    list = self.avail_armors
                elseif self.selecting_slot == Inventory.ITEM_TYPE.ACCESSORY then
                    list = self.avail_accessories
                end
                if button == "Escape" and event == "Press" then
                    self.submenu_none(self)
                elseif button == "Down" then
                    -- Move one position down only if there is a next item
                    if #(list) <= self.list_item_selected + 1 then
                        return 0
                    end
                    self.list_item_selected = self.list_item_selected + 1
                    -- Move cursor...
                    if self.list_position < self.list_position_total then
                        -- ... by moving it one position down.
                        self.list_position = self.list_position + 1
                        ui_manager:get_widget("EquipMenu.Container.List.Cursor"):set_default_animation("Position" .. self.list_position)
                    else
                        -- ... by scrolling the list.
                        self.list_first_visible = self.list_first_visible + 1
                        self.populate_item_list(self)
                    end
                    self.populate_details(self, true)
                    self.calculate_stat_diffs(self)
                elseif button == "Up" then
                    -- Move one position up only if not in the first.
                    if self.list_item_selected <= 1 then
                        return 0
                    end
                    self.list_item_selected = self.list_item_selected - 1
                    -- Move cursor...
                    if self.list_position > 1 then
                        -- ... by moving it one position up
                        self.list_position = self.list_position - 1
                        ui_manager:get_widget("EquipMenu.Container.List.Cursor"):set_default_animation("Position" .. self.list_position)
                    else
                        -- ... by scrolling the list.
                        self.list_first_visible = self.list_first_visible - 1
                        self.populate_item_list(self)
                    end
                    self.populate_details(self, true)
                    self.calculate_stat_diffs(self)
                elseif button == "Enter" and event == "Press" then
                    -- Equip the seleced item
                    local item_id = nil
                    if self.selecting_slot == Inventory.ITEM_TYPE.WEAPON then
                        item_id = self.avail_weapons[self.list_item_selected + 1]
                    elseif self.selecting_slot == Inventory.ITEM_TYPE.ARMOR then
                        item_id = self.avail_armors[self.list_item_selected + 1]
                    elseif self.selecting_slot == Inventory.ITEM_TYPE.ACCESSORY then
                        item_id = self.avail_accessories[self.list_item_selected + 1]
                    end
                    if Characters.equip(self.char_id, item_id) == false then
                        print("BEEP (error)")
                        return 0
                    end
                    self.submenu_none(self)
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
        self.submenu_none(self)
        return 0;
    end,

    --- Enters or returns to the main equip menu.
    submenu_none = function(self)
        UiContainer.current_submenu = ""
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
        self.load_availables(self)
        self.populate_details(self, false)
        self.populate_current_stats(self)
        self.calculate_stat_diffs(self)
        -- Hide stat arrows and difs
        ui_manager:get_widget("EquipMenu.Container.Stats.AtkDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.AccDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.DefDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.EvaDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MAtkDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MDefDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MEvaDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.AtkArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.AccArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.DefArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.EvaArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MAtkArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MDefArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MEvaArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.List.Cursor"):set_visible(false)
        for i = 1, self.list_position_total do
            ui_manager:get_widget("EquipMenu.Container.List.Item" .. tostring(i)):set_text("")
        end
    end,

    --- Initializzes the item selection submenu
    submenu_select = function(self)
        -- Populate the list
        self.list_first_visible = 1
        self.list_position = 1
        self.list_item_selected = 1
        if self.populate_item_list(self) == false then
            print("BEEP")
            return
        end
        UiContainer.current_submenu = "item_select"
        ui_manager:get_widget("EquipMenu.Container.List.Cursor"):set_visible(true)
        ui_manager:get_widget("EquipMenu.Container.List.Cursor"):set_default_animation("Position" .. self.list_position)
        -- TODO: Description and details of first item
        self.populate_details(self, true)
        self.calculate_stat_diffs(self)
    end,

    populate_item_list = function(self)
        local list
        if self.selecting_slot == Inventory.ITEM_TYPE.WEAPON then
            list = self.avail_weapons
        elseif self.selecting_slot == Inventory.ITEM_TYPE.ARMOR then
            list = self.avail_armors
        elseif self.selecting_slot == Inventory.ITEM_TYPE.ACCESSORY then
            list = self.avail_accessories
        else
            for i = 1, self.list_position_total do
                ui_manager:get_widget("EquipMenu.Container.List.Item" .. tostring(i)):set_text("")
            end
            return false
        end
        if list == nil or #(list) == 0 then
            -- No items for the list
            return false
        end
        for i = 1, self.list_position_total do
            if #(list) >= i + self.list_first_visible then
                local name = Game.Items[list[i + self.list_first_visible]].name
                ui_manager:get_widget("EquipMenu.Container.List.Item" .. tostring(i)):set_text(name)
            else
                ui_manager:get_widget("EquipMenu.Container.List.Item" .. tostring(i)):set_text("")
            end
        end
    end,

    --- Loads the list of equipment available for the current character.
    --
    -- Currently equiped items are not added, even if more of them are in the inventory.
    load_availables = function(self)
        self.avail_weapons = {}
        self.avail_armors = {}
        self.avail_accessories = {}
        for _, item in ipairs(Inventory) do
            if item.item ~= nil then
                local item_type = Game.Items[item.item].type
                if item_type == Inventory.ITEM_TYPE.WEAPON or item_type == Inventory.ITEM_TYPE.ARMOR or item_type == Inventory.ITEM_TYPE.ACCESSORY then
                    local equip = Game.Items[item.item].users
                    for _, ch in ipairs(equip) do
                        if ch == self.char_id then
                            if item_type == Inventory.ITEM_TYPE.WEAPON then
                                if item.item ~= Characters[self.char_id].weapon.id then
                                    table.insert(self.avail_weapons, item.item)
                                end
                            elseif item_type == Inventory.ITEM_TYPE.ARMOR then
                                if item.item ~= Characters[self.char_id].armor.id then
                                    table.insert(self.avail_armors, item.item)
                                end
                            elseif item_type == Inventory.ITEM_TYPE.ACCESSORY then
                                if item.item ~= Characters[self.char_id].accessory then
                                    table.insert(self.avail_accessories, item.item)
                                end
                            end
                        end
                    end
                end
            end
        end
    end,

    populate_current_stats = function(self)
        -- TODO: Cap at 255?
        local stats = Characters[self.char_id].stats
        ui_manager:get_widget("EquipMenu.Container.Stats.Atk"):set_text(UiContainer.pad_value(stats.atk, 4))
        ui_manager:get_widget("EquipMenu.Container.Stats.Acc"):set_text(UiContainer.pad_value(stats.acc, 4))
        ui_manager:get_widget("EquipMenu.Container.Stats.Def"):set_text(UiContainer.pad_value(stats.def, 4))
        ui_manager:get_widget("EquipMenu.Container.Stats.Eva"):set_text(UiContainer.pad_value(stats.eva, 4))
        ui_manager:get_widget("EquipMenu.Container.Stats.MAtk"):set_text(UiContainer.pad_value(stats.matk, 4))
        ui_manager:get_widget("EquipMenu.Container.Stats.MDef"):set_text(UiContainer.pad_value(stats.mdef, 4))
        ui_manager:get_widget("EquipMenu.Container.Stats.MEva"):set_text(UiContainer.pad_value(stats.meva, 4))
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
            --elseif self.equip_position == 3 and Characters[self.char_id].accessory ~= nil then
            --    ui_manager:get_widget("EquipMenu.Container.Help.Help"):set_text(Game.Items[Characters[self.char_id].accessory].description)
            end
        else
            -- Selecting from the list
            if self.selecting_slot == Inventory.ITEM_TYPE.WEAPON then
                item = Game.Items[self.avail_weapons[self.list_item_selected + 1]]
                type = Inventory.ITEM_TYPE.WEAPON
            elseif self.selecting_slot == Inventory.ITEM_TYPE.ARMOR then
                item = Game.Items[self.avail_armors[self.list_item_selected + 1]]
                type = Inventory.ITEM_TYPE.ARMOR
            elseif self.selecting_slot == Inventory.ITEM_TYPE.ACCESSORY then
                item = Game.Items[self.avail_accessories[self.list_item_selected + 1]]
                type = Inventory.ITEM_TYPE.ACCESSORY
            end
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
        ui_manager:get_widget("EquipMenu.Container.Slots.Growth0"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Slots.Growth1"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Slots.Growth2"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Slots.Growth3"):set_visible(false)
        if item.growth >= 0 and item.growth <= 3 then
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
    end,

    --- Populates the stat difference panel.
    calculate_stat_diffs = function(self)
        local item
        local cur_item
        if self.selecting_slot == Inventory.ITEM_TYPE.WEAPON then
            item = Game.Items[self.avail_weapons[self.list_item_selected + 1]]
            cur_item = Game.Items[Characters[self.char_id].weapon.id]
        elseif self.selecting_slot == Inventory.ITEM_TYPE.ARMOR then
            item = Game.Items[self.avail_armors[self.list_item_selected + 1]]
            cur_item = Game.Items[Characters[self.char_id].armor.id]
        end
        ui_manager:get_widget("EquipMenu.Container.Stats.AtkDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.AccDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.DefDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.EvaDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MAtkDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MDefDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MEvaDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.AtkArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.AccArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.DefArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.EvaArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MAtkArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MDefArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MEvaArw"):set_visible(false)
        if item == nil or cur_item == nil then
            return 0
        end
        if item.type == Inventory.ITEM_TYPE.WEAPON then
            if item.power > cur_item.power then
                ui_manager:get_widget("EquipMenu.Container.Stats.AtkArw"):set_visible(true)
                ui_manager:get_widget("EquipMenu.Container.Stats.AtkDif"):set_colour(1, 1, 0)
                local val = tostring(Characters[self.char_id].stats.atk - cur_item.power + item.power)
                ui_manager:get_widget("EquipMenu.Container.Stats.AtkDif"):set_text(val)
                ui_manager:get_widget("EquipMenu.Container.Stats.AtkDif"):set_visible(true)
            elseif item.power < cur_item.power then
                ui_manager:get_widget("EquipMenu.Container.Stats.AtkArw"):set_visible(true)
                ui_manager:get_widget("EquipMenu.Container.Stats.AtkDif"):set_colour(1, 0, 0)
                local val = tostring(Characters[self.char_id].stats.atk - cur_item.power + item.power)
                ui_manager:get_widget("EquipMenu.Container.Stats.AtkDif"):set_text(val)
                ui_manager:get_widget("EquipMenu.Container.Stats.AtkDif"):set_visible(true)
            end
            if item.accuracy > cur_item.accuracy then
                ui_manager:get_widget("EquipMenu.Container.Stats.AccArw"):set_visible(true)
                ui_manager:get_widget("EquipMenu.Container.Stats.AccDif"):set_colour(1, 1, 0)
                local val = tostring(Characters[self.char_id].stats.acc - cur_item.accuracy + item.accuracy)
                ui_manager:get_widget("EquipMenu.Container.Stats.AccDif"):set_text(val)
                ui_manager:get_widget("EquipMenu.Container.Stats.AccDif"):set_visible(true)
            elseif item.accuracy < cur_item.accuracy then
                ui_manager:get_widget("EquipMenu.Container.Stats.AccArw"):set_visible(true)
                ui_manager:get_widget("EquipMenu.Container.Stats.AccDif"):set_colour(1, 0, 0)
                local val = tostring(Characters[self.char_id].stats.acc - cur_item.accuracy + item.accuracy)
                ui_manager:get_widget("EquipMenu.Container.Stats.AccDif"):set_text(val)
                ui_manager:get_widget("EquipMenu.Container.Stats.AccDif"):set_visible(true)
            end
        elseif item.type == Inventory.ITEM_TYPE.ARMOR then
            if item.defense.physical > cur_item.defense.physical then
                ui_manager:get_widget("EquipMenu.Container.Stats.DefArw"):set_visible(true)
                ui_manager:get_widget("EquipMenu.Container.Stats.DefDif"):set_colour(1, 1, 0)
                local val = tostring(Characters[self.char_id].stats.def - cur_item.defense.physical + item.defense.physical)
                ui_manager:get_widget("EquipMenu.Container.Stats.DefDif"):set_text(val)
                ui_manager:get_widget("EquipMenu.Container.Stats.DefDif"):set_visible(true)
            elseif item.defense.physical < cur_item.defense.physical then
                ui_manager:get_widget("EquipMenu.Container.Stats.DefArw"):set_visible(true)
                ui_manager:get_widget("EquipMenu.Container.Stats.DefDif"):set_colour(1, 0, 0)
                local val = tostring(Characters[self.char_id].stats.def - cur_item.defense.physical + item.defense.physical)
                ui_manager:get_widget("EquipMenu.Container.Stats.DefDif"):set_text(val)
                ui_manager:get_widget("EquipMenu.Container.Stats.DefDif"):set_visible(true)
            end
            if item.defense.magical > cur_item.defense.magical then
                ui_manager:get_widget("EquipMenu.Container.Stats.MDefArw"):set_visible(true)
                ui_manager:get_widget("EquipMenu.Container.Stats.MDefDif"):set_colour(1, 1, 0)
                local val = tostring(Characters[self.char_id].stats.mdef - cur_item.defense.magical + item.defense.magical)
                ui_manager:get_widget("EquipMenu.Container.Stats.MDefDif"):set_text(val)
                ui_manager:get_widget("EquipMenu.Container.Stats.MDefDif"):set_visible(true)
            elseif item.defense.magical < cur_item.defense.magical then
                ui_manager:get_widget("EquipMenu.Container.Stats.MDefArw"):set_visible(true)
                ui_manager:get_widget("EquipMenu.Container.Stats.MDefDif"):set_colour(1, 0, 0)
                local val = tostring(Characters[self.char_id].stats.mdef - cur_item.defense.magical + item.defense.magical)
                ui_manager:get_widget("EquipMenu.Container.Stats.MDefDif"):set_text(val)
                ui_manager:get_widget("EquipMenu.Container.Stats.MDefDif"):set_visible(true)
            end
            if item.evasion.physical > cur_item.evasion.physical then
                ui_manager:get_widget("EquipMenu.Container.Stats.EvaArw"):set_visible(true)
                ui_manager:get_widget("EquipMenu.Container.Stats.EvaDif"):set_colour(1, 1, 0)
                local val = tostring(Characters[self.char_id].stats.eva - cur_item.evasion.physical + item.evasion.physical)
                ui_manager:get_widget("EquipMenu.Container.Stats.EvaDif"):set_text(val)
                ui_manager:get_widget("EquipMenu.Container.Stats.EvaDif"):set_visible(true)
            elseif item.evasion.physical < cur_item.evasion.physical then
                ui_manager:get_widget("EquipMenu.Container.Stats.EvaArw"):set_visible(true)
                ui_manager:get_widget("EquipMenu.Container.Stats.EvaDif"):set_colour(1, 0, 0)
                local val = tostring(Characters[self.char_id].stats.eva - cur_item.evasion.physical + item.evasion.physical)
                ui_manager:get_widget("EquipMenu.Container.Stats.EvaDif"):set_text(val)
                ui_manager:get_widget("EquipMenu.Container.Stats.EvaDif"):set_visible(true)
            end
            if item.evasion.magical > cur_item.evasion.magical then
                ui_manager:get_widget("EquipMenu.Container.Stats.MEvaArw"):set_visible(true)
                ui_manager:get_widget("EquipMenu.Container.Stats.MEvaDif"):set_colour(1, 1, 0)
                local val = tostring(Characters[self.char_id].stats.mdef - cur_item.evasion.magical + item.evasion.magical)
                ui_manager:get_widget("EquipMenu.Container.Stats.MEvaDif"):set_text(val)
                ui_manager:get_widget("EquipMenu.Container.Stats.MEvaDif"):set_visible(true)
            elseif item.evasion.magical < cur_item.evasion.magical then
                ui_manager:get_widget("EquipMenu.Container.Stats.MEvaArw"):set_visible(true)
                ui_manager:get_widget("EquipMenu.Container.Stats.MEvaDif"):set_colour(1, 0, 0)
                local val = tostring(Characters[self.char_id].stats.mdef - cur_item.evasion.magical + item.evasion.magical)
                ui_manager:get_widget("EquipMenu.Container.Stats.MEvaDif"):set_text(val)
                ui_manager:get_widget("EquipMenu.Container.Stats.MEvaDif"):set_visible(true)
            end
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
