if UiContainer == nil then UiContainer = {} end

--- The main menu.
UiContainer.ItemMenu = {
    bar_position = 1,
    bar_position_total = 3,
    item_cursor_position = 1,
    item_cursor_position_total = 10,
    item_cursor_item_selected = 0,
    char_position = 1,
    char_position_total = 3,
    first_item_in_window = 0,

    -- Item ID of the currently selected (not highlighted) item.
    item_in_use = nil,

    -- Indicates it the selected item affect the entire party.
    item_in_use_party = false,

    on_start = function(self)
        return 0
    end,

    on_button = function(self, button, event)
        if UiContainer.current_menu == "item" then
            if UiContainer.current_submenu == "bar_selection" then
                if button == "Escape" and event == "Press" then
                    UiContainer.current_menu = "main"
                    script:request_end_sync(Script.UI, "ItemMenu", "hide", 0)
                elseif button == "Right" then
                    self.bar_position = self.bar_position + 1
                    if self.bar_position > self.bar_position_total then
                        self.bar_position = 1
                    end
                    ui_manager:get_widget("ItemMenu.Container.ItemMenuTopBar.Cursor"):set_default_animation("Position" .. self.bar_position)
                elseif button == "Left" then
                    self.bar_position = self.bar_position - 1
                    if self.bar_position < 1 then
                        self.bar_position = self.bar_position_total
                    end
                    ui_manager:get_widget("ItemMenu.Container.ItemMenuTopBar.Cursor"):set_default_animation("Position" .. self.bar_position)
                elseif button == "Enter" then
                    if self.bar_position == 1 then
                        self.submenu_items(self)
                    elseif self.bar_position == 2 then
                        print("Unimplemented: Order")
                    elseif self.bar_position == 3 then
                        print("Unimplemented: Key Items")
                    else
                        return 0
                    end
                else
                    return 0
                end
            elseif UiContainer.current_submenu == "item_selection" then
                local cursor = ui_manager:get_widget("ItemMenu.Container.ItemList.Cursor")
                local desc_label = ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText")
                if button == "Down" then
                    if self.item_cursor_item_selected >= 320 then
                        -- At the very end, do nothing
                        return 0
                    end
                    self.item_cursor_item_selected = self.item_cursor_item_selected + 1
                    if self.item_cursor_position == self.item_cursor_position_total then
                        -- The cursor is at the end. Scroll page, but dont change cursor.
                        self.first_item_in_window = self.first_item_in_window + 1
                        self.populate_items(self)
                    else
                        -- The cursor is not at the end, move cursor down.
                        self.item_cursor_position = self.item_cursor_position + 1
                    end
                    cursor:set_default_animation("Position" .. self.item_cursor_position)
                    -- If an item is selected, show description
                    if (Inventory[self.item_cursor_item_selected] ~= nil) then
                        desc_label:set_text(Game.Items[Inventory[self.item_cursor_item_selected].item].description)
                    else
                        desc_label:set_text("")
                    end
                elseif button == "Up" then
                    if self.item_cursor_item_selected == 0 then
                        -- At the very begining, do nothing
                        return 0
                    end
                    self.item_cursor_item_selected = self.item_cursor_item_selected - 1
                    if self.item_cursor_position == 1 then
                        -- The cursor is at the end. Scroll page, but dont change cursor.
                        self.first_item_in_window = self.first_item_in_window - 1
                        self.populate_items(self)
                    else
                        -- The cursor is not at the end, move cursor down.
                        self.item_cursor_position = self.item_cursor_position - 1
                    end
                    cursor:set_default_animation("Position" .. self.item_cursor_position)
                    -- If an item is selected, show description
                    if (Inventory[self.item_cursor_item_selected] ~= nil) then
                        desc_label:set_text(Game.Items[Inventory[self.item_cursor_item_selected].item].description)
                    else
                        desc_label:set_text("")
                    end
                elseif button == "Escape" and event == "Press" then
                    self.submenu_bar(self)
                    UiContainer.current_submenu = "bar_selection"
                    ui_manager:get_widget("ItemMenu.Container.ItemList.Cursor"):set_visible(false)
                elseif button == "Enter" then
                    if Game.Items[Inventory[self.item_cursor_item_selected].item].menu == 1 then
                        self.item_in_use = Inventory[self.item_cursor_item_selected].item
                        if (Game.Items[Inventory[self.item_cursor_item_selected].item].target.default_multiple) == 1 then
                            self.item_in_use_party = true
                            self.submenu_chars(self, true)
                        else
                            self.item_in_use_party = false
                            self.submenu_chars(self, false)
                        end
                    else
                        print("BEEP")
                    end
                    --self.submenu_bar(self)
                    --UiContainer.current_submenu = "bar_selection"
                    --ui_manager:get_widget("ItemMenu.Container.ItemList.Cursor"):set_visible(false)
                end
            elseif UiContainer.current_submenu == "char_selection" then
                if button == "Down" then
                    if self.item_in_use_party == true then
                        return 0
                    else
                        self.char_position = self.char_position + 1
                        if self.char_position > self.char_position_total then
                            self.char_position = 1
                        end
                        ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.Cursor"):set_default_animation("Position" .. self.char_position)
                    end
                elseif button == "Up" then
                    if self.item_in_use_party == true then
                        return 0
                    else
                        self.char_position = self.char_position - 1
                        if self.char_position < 1 then
                            self.char_position = self.char_position_total
                        end
                        ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.Cursor"):set_default_animation("Position" .. self.char_position)
                    end
                elseif button == "Escape" then
                    self.submenu_items(self)
                elseif button == "Enter" then
                    local use_result = false
                    if self.item_in_use_party == true then
                        use_result = self.use_item(self, self.item_in_use, nil)
                    else
                        use_result = self.use_item(self, self.item_in_use, self.char_position)
                    end
                    if use_result == false then
                        print("USE BEEP")
                    end
                end
            elseif UiContainer.current_submenu == "key_items" then
            end
        end
        return 0
    end,

    --- Opens the main menu.
    --
    -- Populates and updates displayed data before opening.
    show = function(self)
        ui_manager:get_widget("ItemMenu"):set_visible(true)
        UiContainer.current_menu = "item"
        Inventory.add_item(0, 2)
        Inventory.add_item(1, 4)
        Inventory.add_item(42, 3)
        Inventory.add_item(1, 4)
        Inventory.add_item(6, 5)
        Inventory.add_item(13, 2)
        Inventory.add_item(14, 4)
        Characters[0].stats.hp.current = 10
        self.char_position_total = 0
        for c = 1, 3 do
            if FFVII.Party[c] ~= nil then
                self.char_position_total = self.char_position_total + 1
                UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(c), Characters[FFVII.Party[c]])
            else
                UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(c), nil)
            end
        end
        self.populate_items(self)
        self.submenu_items(self)
        return 0;
    end,

    submenu_bar = function(self)
        UiContainer.current_submenu = "bar_selection"
        ui_manager:get_widget("ItemMenu.Container.ItemList.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.CursorMultiple"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuTopBar.Cursor"):set_visible(true)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuTopBar.Cursor"):set_default_animation("Position" .. self.bar_position)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text("")
    end,

    submenu_items = function(self)
        UiContainer.current_submenu = "item_selection"
        ui_manager:get_widget("ItemMenu.Container.ItemList.Cursor"):set_visible(true)
        ui_manager:get_widget("ItemMenu.Container.ItemList.Cursor"):set_default_animation("Position" .. self.item_cursor_position)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.CursorMultiple"):set_visible(false)
        --ui_manager:get_widget("ItemMenu.Container.ItemMenuTopBar.Cursor"):set_visible(false)
        -- Show description of the very first item, if any.
        if (Inventory[0] ~= nil) then
            ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text(Game.Items[Inventory[0].item].description)
        else
            ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text("")
        end
    end,

    submenu_chars = function(self, multiple)
        UiContainer.current_submenu = "char_selection"
        --ui_manager:get_widget("ItemMenu.Container.ItemList.Cursor"):set_visible(false)
        --ui_manager:get_widget("ItemMenu.Container.ItemMenuTopBar.Cursor"):set_visible(false)
        if (multiple == true) then
            ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.Cursor"):set_visible(false)
            ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.CursorMultiple"):set_visible(true)
        else
            ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.CursorMultiple"):set_visible(false)
            ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.Cursor"):set_visible(true)
            ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.Cursor"):set_default_animation("Position" .. self.char_position)
        end
    end,

    --- Uses an item from the menu.
    --
    -- Checks the item conditions. If they are valid, uses the item and discounts 1 from the
    -- inventory.
    -- TODO: If after using a item, the quantity becames 0, go back to the item selection menu.
    --
    -- @param item The ID of the item being used.
    -- @param character_pos Position in the party of the character on which to use the item (1-3). If
    -- nil, the item affects the whole party.
    use_item = function(self, item, character_pos)
        char_id = -1
        if character_pos then
            char_id = FFVII.Party[character_pos]
            -- Normal item, use in one character.
        else
            -- Party item, use in all characters.
        end

        -- TODO: Refactor this. Use damage formulas, power and restore types to make it less
        -- hardcoded
        -- TODO: When HP MAX is fixed, use it instead of base.
        if Game.Items[item].dmg_formula == 2 then
            -- HP or MP heal: Potion, Hi-Potion, X-Potion, Ether, Turbo Ether, Phoenix Down
            if Game.Items[item].dmg_modifier == 6 then
                -- Partial restore: Potion, Hi-Potion, Ether
                if Game.Items[item].restore == 0 then
                    -- Partial HP restore: Potion, Hi potion. Based on power.
                    if char_id == -1 then
                        return false
                    end
                    if Characters[char_id].stats.hp.current < Characters[char_id].stats.hp.base then
                        Characters[char_id].stats.hp.current = Characters[char_id].stats.hp.current + (25 * Game.Items[item].power)
                        if Characters[char_id].stats.hp.current > Characters[char_id].stats.hp.base then
                            Characters[char_id].stats.hp.current = Characters[char_id].stats.hp.base
                        end
                        UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(character_pos), Characters[char_id])
                        UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(character_pos), Characters[char_id])
                    else
                        return false
                    end
                elseif Game.Items[item].restore == 1 then
                    -- Partial MP restore: Ether. Based on power.
                    if Characters[char_id].stats.mp.current < Characters[char_id].stats.mp.base then
                        Characters[char_id].stats.mp.current = Characters[char_id].stats.mp.current + (25 * Game.Items[item].power)
                        if Characters[char_id].stats.mp.current > Characters[char_id].stats.mp.base then
                            Characters[char_id].stats.mp.current = Characters[char_id].stats.mp.base
                        end
                        UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(character_pos), Characters[char_id])
                        UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(character_pos), Characters[char_id])
                    else
                        return false
                    end
                end
            elseif Game.Items[item].dmg_modifier == 4 then
                -- Full restore: X-Potion, Turbo Ether or partial resurrect: Phoenix Down.
                if Game.Items[item].restore == 0 then
                    -- Full HP restore: X-Potion. Power ignored.
                    if char_id == -1 then
                        return false
                    end
                    if Characters[char_id].stats.hp.current < Characters[char_id].stats.hp.base then
                        Characters[char_id].stats.hp.current = Characters[char_id].stats.hp.base
                        UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(character_pos), Characters[char_id])
                        UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(character_pos), Characters[char_id])
                    else
                        return false
                    end
                elseif Game.Items[item].restore == 1 then
                    -- Full MP restore: Turbo ether. Power ignored.
                    if Characters[char_id].stats.mp.current < Characters[char_id].stats.mp.base then
                        Characters[char_id].stats.mp.current = Characters[char_id].stats.mp.base
                        UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(character_pos), Characters[char_id])
                        UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(character_pos), Characters[char_id])
                    else
                        return false
                    end
                else
                    -- Resurrect: Phoenix down. Power / 32 is the percent of max HP.
                    if Characters[char_id].stats.hp.current == 0 then
                        Characters[char_id].stats.hp.current = math.floor(Game.Items[item].power * Characters[char_id].stats.hp.current / 32)
                        if Characters[char_id].stats.hp.current > Characters[char_id].stats.hp.base then
                            Characters[char_id].stats.hp.current = Characters[char_id].stats.hp.base
                        end
                        UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(character_pos), Characters[char_id])
                        UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(character_pos), Characters[char_id])
                    else
                        return false
                    end
                end
            end
        elseif Game.Items[item].dmg_formula == 0 and Game.Items[item].dmg_modifier == 0 then
            -- Full HP and MP restore: Elixir, Megalixir
            if Game.Items[item].target.default_multiple == 0 then
                --Full HP and MP restore for single target: Elixir.
                if Characters[char_id].stats.hp.current < Characters[char_id].stats.hp.base or Characters[char_id].stats.mp.current < Characters[char_id].stats.mp.base then
                    Characters[char_id].stats.hp.current = Characters[char_id].stats.hp.base
                    Characters[char_id].stats.mp.current = Characters[char_id].stats.mp.base
                    UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(character_pos), Characters[char_id])
                    UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(character_pos), Characters[char_id])
                else
                    return false
                end
            else
                --Full HP and MP restore for party: MegaElixir.
                local useable = false;
                for c = 1, 3 do
                    if Characters[c] ~= nil then
                        if Characters[c].stats.hp.current < Characters[c].stats.hp.base then
                            useable = true
                        elseif Characters[c].stats.mp.current < Characters[c].stats.mp.base then
                            useable = true
                        end
                    end
                end
                if useable == true then
                    for pos = 1, 3 do
                        if Characters[Party[pos]] ~= nil then
                            Characters[Party[pos]].stats.hp.current = Characters[Party[pos]].stats.hp.base
                            Characters[Party[pos]].stats.mp.current = Characters[Party[pos]].stats.mp.base
                            UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(pos), Characters[Party[pos]])
                            UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(pos), Characters[Party[pos]])
                        end
                    end
                else
                    return false
                end
            end
        elseif Game.Items[item].dmg_formula == 4 then
            -- Status change: Hyper, Tranquilizer
            if #(Game.Items[item].status) > 0 then
                for _, status in ipairs(Game.Items[item].status) do
                    if status.status == 5 then
                        -- Fury. Cure saddness or induce fury.
                        local sadness_key = -1
                        local fury_key = -1
                        for st_key, st_val in ipairs(Characters[char_id].status) do
                            if st_val == 4 then
                                sadness_key = st_key
                            elseif st_val == 5 then
                                fury_key = st_key
                            end
                        end
                        -- If fury already, donothing. If sad, cure. Else, apply fury.
                        if fury_key ~= -1 then
                            return false
                        elseif sadness_key ~= -1 then
                            table.remove(Characters[char_id].status, sadness_key)
                            UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(character_pos), Characters[char_id])
                            UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(character_pos), Characters[char_id])
                        else
                            table.insert(Characters[char_id].status, 5)
                            UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(character_pos), Characters[char_id])
                            UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(character_pos), Characters[char_id])
                        end
                    elseif status.status == 4 then
                        -- Saddness. Cure fury or induce sadness.
                        local sadness_key = -1
                        local fury_key = -1
                        for st_key, st_val in ipairs(Characters[char_id].status) do
                            if st_val == 4 then
                                sadness_key = st_key
                            elseif st_val == 5 then
                                fury_key = st_key
                            end
                        end
                        -- If fury already, donothing. If sad, cure. Else, apply fury.
                        if sadness_key ~= -1 then
                            return false
                        elseif fury_key ~= -1 then
                            table.remove(Characters[char_id].status, fury_key)
                            UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(character_pos), Characters[char_id])
                            UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(character_pos), Characters[char_id])
                        else
                            table.insert(Characters[char_id].status, 4)
                            UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(character_pos), Characters[char_id])
                            UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(character_pos), Characters[char_id])
                        end
                    end
                end

            end
        elseif Game.Items[item].id == 70 then
            -- Tent. Can't be differenciated by other means other than ID. Full HP and MP restore for party.
                local useable = false;
                for c = 1, 3 do
                    if Characters[c] ~= nil then
                        if Characters[c].stats.hp.current < Characters[c].stats.hp.base then
                            useable = true
                        elseif Characters[c].stats.mp.current < Characters[c].stats.mp.base then
                            useable = true
                        end
                    end
                end
                if useable == true then
                    for pos = 1, 3 do
                        if Characters[Party[pos]] ~= nil then
                            Characters[Party[pos]].stats.hp.current = Characters[Party[pos]].stats.hp.base
                            Characters[Party[pos]].stats.mp.current = Characters[Party[pos]].stats.mp.base
                            UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(pos), Characters[Party[pos]])
                            UiContainer.populate_character_data("MainMenu.Container.Characters.Character" .. tostring(pos), Characters[Party[pos]])
                        end
                    end
                else
                    return false
                end
        elseif Game.Items[item].id == 71 then
            -- Power source. Can't be differenciated by other means other than ID. STR bonus + 1.
            -- Original game caps bonus at 255. V-Gears doesn't.
            Characters[char_id].stats.str.bonus = Characters[char_id].stats.str.bonus + 1
        elseif Game.Items[item].id == 72 then
            -- Guard source. Can't be differenciated by other means other than ID. VIT bonus + 1.
            -- Original game caps bonus at 255. V-Gears doesn't.
            Characters[char_id].stats.vit.bonus = Characters[char_id].stats.vit.bonus + 1
        elseif Game.Items[item].id == 73 then
            -- Magic source. Can't be differenciated by other means other than ID. MAG bonus + 1.
            -- Original game caps bonus at 255. V-Gears doesn't.
            Characters[char_id].stats.mag.bonus = Characters[char_id].stats.mag.bonus + 1
        elseif Game.Items[item].id == 74 then
            -- Mind source. Can't be differenciated by other means other than ID. SPR bonus + 1.
            -- Original game caps bonus at 255. V-Gears doesn't.
            Characters[char_id].stats.spr.bonus = Characters[char_id].stats.spr.bonus + 1
        elseif Game.Items[item].id == 75 then
            -- Speed source. Can't be differenciated by other means other than ID. DEX bonus + 1.
            -- Original game caps bonus at 255. V-Gears doesn't.
            Characters[char_id].stats.dex.bonus = Characters[char_id].stats.dex.bonus + 1
        elseif Game.Items[item].id == 76 then
            -- Luck source. Can't be differenciated by other means other than ID. LCK bonus + 1.
            -- Original game caps bonus at 255. V-Gears doesn't.
            Characters[char_id].stats.lck.bonus = Characters[char_id].stats.lck.bonus + 1
        elseif Game.Items[item].id == 87 then
            -- Omnislash. Can't be differenciated by other means other than ID.
            -- Functionality hardcoded here.
            if (char_id ~= 0) then
                -- TODO: Show some message
                return false
            end
            for l = 1, 3 do
                if Characters[0].limit.learned[l][1] == 0 or Characters[0].limit.learned[l][2] == 0 then
                    -- Some limit not learned yet. Show some message.
                    return false
                end
            end
            if Characters[0].limit.learned[4][1] == 1 then
                -- Already learned
                return false
            end
            Characters[0].limit.learned[4][1] = 1
        elseif Game.Items[item].id == 88 then
            -- Catastrophe. Can't be differenciated by other means other than ID.
            -- Functionality hardcoded here.
            if (char_id ~= 1) then
                -- TODO: Show some message
                return false
            end
            for l = 1, 3 do
                if Characters[1].limit.learned[l][1] == 0 or Characters[1].limit.learned[l][2] == 0 then
                    -- Some limit not learned yet. Show some message.
                    return false
                end
            end
            if Characters[1].limit.learned[4][1] == 1 then
                -- Already learned
                return false
            end
            Characters[1].limit.learned[4][1] = 1
        elseif Game.Items[item].id == 89 then
            -- Final Heaven. Can't be differenciated by other means other than ID.
            -- Functionality hardcoded here.
            if (char_id ~= 2) then
                -- TODO: Show some message
                return false
            end
            for l = 1, 3 do
                if Characters[2].limit.learned[l][1] == 2 or Characters[2].limit.learned[l][2] == 0 then
                    -- Some limit not learned yet. Show some message.
                    return false
                end
            end
            if Characters[2].limit.learned[4][1] == 1 then
                -- Already learned
                return false
            end
            Characters[2].limit.learned[4][1] = 1
        elseif Game.Items[item].id == 90 then
            -- Great Gospel. Can't be differenciated by other means other than ID.
            -- Functionality hardcoded here.
            if (char_id ~= 3) then
                -- TODO: Show some message
                return false
            end
            for l = 1, 3 do
                if Characters[3].limit.learned[l][1] == 0 or Characters[3].limit.learned[l][2] == 0 then
                    -- Some limit not learned yet. Show some message.
                    return false
                end
            end
            if Characters[3].limit.learned[4][1] == 1 then
                -- Already learned
                return false
            end
            Characters[3].limit.learned[4][1] = 1
        elseif Game.Items[item].id == 91 then
            -- Cosmo Memory. Can't be differenciated by other means other than ID.
            -- Functionality hardcoded here.
            if (char_id ~= 4) then
                -- TODO: Show some message
                return false
            end
            for l = 1, 3 do
                if Characters[4].limit.learned[l][1] == 0 or Characters[4].limit.learned[l][2] == 0 then
                    -- Some limit not learned yet. Show some message.
                    return false
                end
            end
            if Characters[4].limit.learned[4][1] == 1 then
                -- Already learned
                return false
            end
            Characters[4].limit.learned[4][1] = 1
        elseif Game.Items[item].id == 92 then
            -- All Creation. Can't be differenciated by other means other than ID.
            -- Functionality hardcoded here.
            if (char_id ~= 5) then
                -- TODO: Show some message
                return false
            end
            for l = 1, 3 do
                if Characters[5].limit.learned[l][1] == 0 or Characters[5].limit.learned[l][2] == 0 then
                    -- Some limit not learned yet. Show some message.
                    return false
                end
            end
            if Characters[5].limit.learned[4][1] == 1 then
                -- Already learned
                return false
            end
            Characters[5].limit.learned[4][1] = 1
        elseif Game.Items[item].id == 93 then
            -- Chaos. Can't be differenciated by other means other than ID.
            -- Functionality hardcoded here.
            if (char_id ~= 7) then
                -- TODO: Show some message
                return false
            end
            for l = 1, 3 do
                if Characters[7].limit.learned[l][1] == 0 then
                    -- Some limit not learned yet. Show some message.
                    return false
                end
            end
            if Characters[7].limit.learned[4][1] == 1 then
                -- Already learned
                return false
            end
            Characters[0].limit.learned[4][1] = 1
        elseif Game.Items[item].id == 94 then
            -- Highwind. Can't be differenciated by other means other than ID.
            -- Functionality hardcoded here.
            if (char_id ~= 8) then
                -- TODO: Show some message
                return false
            end
            for l = 1, 3 do
                if Characters[8].limit.learned[l][1] == 0 or Characters[8].limit.learned[l][2] == 0 then
                    -- Some limit not learned yet. Show some message.
                    return false
                end
            end
            if Characters[8].limit.learned[4][1] == 1 then
                -- Already learned
                return false
            end
            Characters[8].limit.learned[4][1] = 1
        elseif Game.Items[item].id == 98 then
            -- Save Crystal. Can't be differenciated by other means other than ID.
            -- TODO: Implement.
            return false
        end

        -- If this point has been reached, an item has been succesfully used.
        -- Discount it from the inventory.
        Inventory.remove_item(item, 1)
        self.populate_items(self)

        -- If it was the last item, go back to the list
        if Inventory.get_item_quantity(item) == 0 then
            self.submenu_items(self)
        end
        return true
    end,


    --- Populates the visible inventory entries.
    --
    -- 10 entries visible at a time.
    populate_items = function(self)
        local first_item_in_window = self.first_item_in_window
        for i = 0, 9 do
            local w_icon = ui_manager:get_widget("ItemMenu.Container.ItemList.Icon" .. tostring(i))
            local w_name = ui_manager:get_widget("ItemMenu.Container.ItemList.Name" .. tostring(i))
            local w_colon = ui_manager:get_widget("ItemMenu.Container.ItemList.Colon" .. tostring(i))
            local w_qty = ui_manager:get_widget("ItemMenu.Container.ItemList.Qty" .. tostring(i))
            local inventory_index = i + first_item_in_window
            if Inventory[inventory_index] == nil then
                w_icon:set_visible(false)
                w_name:set_visible(false)
                w_colon:set_visible(false)
                w_qty:set_visible(false)
            else
                local useable = Game.Items[Inventory[inventory_index].item].menu
                -- TODO: Get icon.
                w_icon:set_visible(true)
                w_name:set_visible(true)
                w_colon:set_visible(true)
                w_qty:set_visible(true)
                -- TODO: Gray for unuseable
                if (useable == 1) then
                    w_name:set_colour(1.0, 1.0, 1.0)
                    w_colon:set_colour(1.0, 1.0, 1.0)
                    w_qty:set_colour(1.0, 1.0, 1.0)
                else
                    w_name:set_colour(0.6, 0.6, 0.6)
                    w_colon:set_colour(0.6, 0.6, 0.6)
                    w_qty:set_colour(0.6, 0.6, 0.6)
                end
                w_name:set_text(Game.Items[Inventory[inventory_index].item].name)
                w_qty:set_text(tostring(Inventory[inventory_index].quantity))
                w_colon:set_text(":") -- Refresh text to apply colour.
            end
        end
        return
    end,


    hide = function(self)
        --[[local characters  = ui_manager:get_widget("MainMenu.Container.Characters")
        local menu = ui_manager:get_widget("MainMenu.Container.Menu")
        local timegil = ui_manager:get_widget("MainMenu.Container.TimeGil")
        local location = ui_manager:get_widget("MainMenu.Container.Location")

        characters:play_animation_stop("Disappear")
        menu:play_animation_stop("Disappear")
        timegil:play_animation_stop("Disappear")
        location:play_animation_stop("Disappear")
        location:animation_sync()]]

        ui_manager:get_widget("ItemMenu"):set_visible(false)
        UiContainer.current_menu = "main"
        UiContainer.current_submenu = ""

        --FFVII.MenuSettings.pause_available = true
        --entity_manager:set_paused(false)

        return 0;
    end,
}
