if UiContainer == nil then UiContainer = {} end

--- The item menu.
UiContainer.ItemMenu = {

    --- Current cursor position in the top bar (1-3).
    bar_position = 1,

    --- Max cursor position in the top bar.
    bar_position_total = 3,

    --- Current cursor position in the item list (1-10).
    --
    -- It doesn't represent the item id or the inventory position, but the highlighted row among
    -- the ten displayed.
    item_cursor_position = 1,

    --- Max cursor position in the item list (1-10).
    item_cursor_position_total = 10,

    --- Highlighted inventory position (1-320).
    item_cursor_item_selected = 1,

    --- Current cursor position in the character section (1-3).
    char_position = 1,

    --- Max cursor position in the character section.
    char_position_total = 3,

    --- First inventory entry currently visible in the item list.
    first_item_in_window = 0,

    --- Key items cursor vertical position (0-9)
    --
    -- It doesn't represent the item id or the inventory position, but the highlighted row among
    -- the ten displayed.
    key_items_cursor_x_position = 0,

    --- Key items cursor horizontal position (0-1)
    --
    -- It doesn't represent the item id or the inventory position, but the highlighted row among
    -- the ten displayed. 0 if the cursor is in the left column, 1 if it's on the right.
    key_items_cursor_y_position = 0,

    --- First visible row of key items (0-16)
    --
    -- Changes as the key item list scrols vertically.
    first_key_item_row_in_window = 0,

    --- Current cursor position in the order section (1-8).
    order_cursor_position = 1,

    --- Max cursor position in the order section.
    order_cursor_position_total = 8,

    --- Item ID of the currently selected (not highlighted) item.
    item_in_use = nil,

    --- Indicates it the selected item affect the entire party.
    item_in_use_party = false,

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
                    -- If highlighted, show key items.
                    if self.bar_position == 3 then
                        ui_manager:get_widget("ItemMenu.Container.KeyItems"):set_visible(true)
                    else
                        ui_manager:get_widget("ItemMenu.Container.KeyItems"):set_visible(false)
                    end
                elseif button == "Left" then
                    self.bar_position = self.bar_position - 1
                    if self.bar_position < 1 then
                        self.bar_position = self.bar_position_total
                    end
                    ui_manager:get_widget("ItemMenu.Container.ItemMenuTopBar.Cursor"):set_default_animation("Position" .. self.bar_position)
                    -- If highlighted, show key items.
                    if self.bar_position == 3 then
                        ui_manager:get_widget("ItemMenu.Container.KeyItems"):set_visible(true)
                    else
                        ui_manager:get_widget("ItemMenu.Container.KeyItems"):set_visible(false)
                    end
                elseif button == "Enter" then
                    if self.bar_position == 1 then
                        self.submenu_items(self)
                    elseif self.bar_position == 2 then
                        self.submenu_order(self)
                    elseif self.bar_position == 3 then
                        self.submenu_key_items(self)
                    else
                        return 0
                    end
                else
                    return 0
                end
            elseif UiContainer.current_submenu == "order_selection" then
                if button == "Escape" and event == "Press" then
                    self.submenu_bar(self)
                elseif button == "Down" then
                    self.order_cursor_position = self.order_cursor_position + 1
                    if self.order_cursor_position > self.order_cursor_position_total then
                        self.order_cursor_position = 1
                    end
                    ui_manager:get_widget("ItemMenu.Container.ItemOrder.Cursor"):set_default_animation("Position" .. self.order_cursor_position)
                elseif button == "Up" then
                    self.order_cursor_position = self.order_cursor_position - 1
                    if self.order_cursor_position <= 0 then
                        self.order_cursor_position = self.order_cursor_position_total
                    end
                    ui_manager:get_widget("ItemMenu.Container.ItemOrder.Cursor"):set_default_animation("Position" .. self.order_cursor_position)
                elseif button == "Enter" then
                    if self.order_cursor_position == 1 then
                        Inventory.sort(Inventory.ORDER.CUSTOM)
                    elseif self.order_cursor_position == 2 then
                        Inventory.sort(Inventory.ORDER.FIELD)
                    elseif self.order_cursor_position == 3 then
                        Inventory.sort(Inventory.ORDER.BATTLE)
                    elseif self.order_cursor_position == 4 then
                        Inventory.sort(Inventory.ORDER.THROW)
                    elseif self.order_cursor_position == 5 then
                        Inventory.sort(Inventory.ORDER.TYPE)
                    elseif self.order_cursor_position == 6 then
                        Inventory.sort(Inventory.ORDER.NAME)
                    elseif self.order_cursor_position == 7 then
                        Inventory.sort(Inventory.ORDER.MOST)
                    elseif self.order_cursor_position == 8 then
                        Inventory.sort(Inventory.ORDER.LEAST)
                    end
                    self.populate_items(self)
                    self.submenu_bar(self)
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
            elseif UiContainer.current_submenu == "key_item_selection" then
                if button == "Down" then
                    if self.key_items_cursor_y_position == 9 and self.first_key_item_row_in_window == 16 then
                        -- Last key item selected, do nothing
                        return 0
                    end
                    if self.key_items_cursor_y_position == 9 then
                        -- Scroll down by 1, don't move cursor
                        self.first_key_item_row_in_window = self.first_key_item_row_in_window + 1
                        self.populate_key_items(self)
                    else
                        -- Move cursor down by 1.
                        self.key_items_cursor_y_position = self.key_items_cursor_y_position + 1
                    end
                    ui_manager:get_widget("ItemMenu.Container.KeyItems.Cursor"):set_default_animation("Position" .. self.key_items_cursor_y_position .. "-" .. self.key_items_cursor_x_position)
                    -- Show description of the selected item, if any.
                    local k_selected_id = (self.first_key_item_row_in_window + self.key_items_cursor_y_position) * 2 + self.key_items_cursor_x_position
                    if (Inventory.key[k_selected_id] == true) then
                        ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text(Game.Key_Items[k_selected_id].description)
                    else
                        ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text("")
                    end
                elseif button == "Up" then
                    if self.key_items_cursor_y_position == 0 and self.first_key_item_row_in_window == 0 then -- TODO: Don't hardcode.
                        -- First item, do nothing.
                        return 0
                    end
                    if self.key_items_cursor_y_position == 0 then
                        -- Scroll down by 1, don't move cursor
                        self.first_key_item_row_in_window = self.first_key_item_row_in_window - 1
                        self.populate_key_items(self)
                    else
                        -- Move cursor up by 1.
                        self.key_items_cursor_y_position = self.key_items_cursor_y_position - 1
                    end
                    ui_manager:get_widget("ItemMenu.Container.KeyItems.Cursor"):set_default_animation("Position" .. self.key_items_cursor_y_position .. "-" .. self.key_items_cursor_x_position)
                    -- Show description of the selected item, if any.
                    local k_selected_id = (self.first_key_item_row_in_window + self.key_items_cursor_y_position) * 2 + self.key_items_cursor_x_position
                    if (Inventory.key[k_selected_id] == true) then
                        ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text(Game.Key_Items[k_selected_id].description)
                    else
                        ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text("")
                    end
                elseif button == "Right" then
                    local next_selected_index = (self.first_key_item_row_in_window + self.key_items_cursor_y_position) * 2 + self.key_items_cursor_x_position + 1
                    if next_selected_index >= 50 then
                        -- Already in last item, do nothing
                        return 0
                    end
                    if self.key_items_cursor_x_position == 0 then
                        -- If on the left, move to the right. No scrolling.
                        self.key_items_cursor_x_position = 1
                    else
                        -- If on the right, move to left and one row down...
                        self.key_items_cursor_x_position = 0
                        if self.key_items_cursor_y_position == 9 then
                            -- ... by scrolling down by 1, not moving cursor.
                            self.first_key_item_row_in_window = self.first_key_item_row_in_window + 1
                            self.populate_key_items(self)
                        else
                            --  ... by moving the cursor down.
                            self.key_items_cursor_y_position = self.key_items_cursor_y_position + 1
                        end
                    end
                    ui_manager:get_widget("ItemMenu.Container.KeyItems.Cursor"):set_default_animation("Position" .. self.key_items_cursor_y_position .. "-" .. self.key_items_cursor_x_position)
                    -- Show description of the selected item, if any.
                    local k_selected_id = (self.first_key_item_row_in_window + self.key_items_cursor_y_position) * 2 + self.key_items_cursor_x_position
                    if (Inventory.key[k_selected_id] == true) then
                        ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text(Game.Key_Items[k_selected_id].description)
                    else
                        ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text("")
                    end
                elseif button == "Left" then
                    local next_selected_index = (self.first_key_item_row_in_window + self.key_items_cursor_y_position) * 2 + self.key_items_cursor_x_position - 1
                    if next_selected_index < 0 then
                        -- Already in first item, do nothing
                        return 0
                    end
                    if self.key_items_cursor_x_position == 1 then
                        -- If on the right, move to the left. No scrolling.
                        self.key_items_cursor_x_position = 0
                    else
                        -- If on the left, move to right and one row up...
                        self.key_items_cursor_x_position = 1
                        if self.key_items_cursor_y_position == 0 then
                            -- ... by scrolling up by 1, not moving cursor.
                            self.first_key_item_row_in_window = self.first_key_item_row_in_window - 1
                            self.populate_key_items(self)
                        else
                            --  ... by moving the cursor up.
                            self.key_items_cursor_y_position = self.key_items_cursor_y_position - 1
                        end
                    end
                    ui_manager:get_widget("ItemMenu.Container.KeyItems.Cursor"):set_default_animation("Position" .. self.key_items_cursor_y_position .. "-" .. self.key_items_cursor_x_position)
                    -- Show description of the selected item, if any.
                    local k_selected_id = (self.first_key_item_row_in_window + self.key_items_cursor_y_position) * 2 + self.key_items_cursor_x_position
                    if (Inventory.key[k_selected_id] == true) then
                        ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text(Game.Key_Items[k_selected_id].description)
                    else
                        ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text("")
                    end
                end

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
        self.char_position_total = 0
        for c = 1, 3 do
            if FFVII.Party[c] ~= nil then
                self.char_position_total = self.char_position_total + 1
                UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(c), Characters[FFVII.Party[c]])
                ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(c) .. ".Portrait"):set_image("images/characters/" .. tostring(FFVII.Party[c]) .. ".png")
            else
                UiContainer.populate_character_data("ItemMenu.Container.ItemMenuCharacters.Character" .. tostring(c), nil)
            end
        end
        self.populate_items(self)
        self.populate_key_items(self)
        self.submenu_items(self)
        return 0;
    end,

    --- Initializes the selection of the menu section in the top bar.
    submenu_bar = function(self)
        UiContainer.current_submenu = "bar_selection"
        ui_manager:get_widget("ItemMenu.Container.ItemList.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.CursorMultiple"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuTopBar.Cursor"):set_visible(true)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuTopBar.Cursor"):set_default_animation("Position" .. self.bar_position)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text("")
        ui_manager:get_widget("ItemMenu.Container.KeyItems.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemOrder.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemOrder"):set_visible(false)
    end,

    --- Initializes the sorting mode window.
    submenu_order = function(self)
        UiContainer.current_submenu = "order_selection"
        self.order_cursor_position = 1
        ui_manager:get_widget("ItemMenu.Container.ItemList.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.CursorMultiple"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuTopBar.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text("")
        ui_manager:get_widget("ItemMenu.Container.KeyItems.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemOrder"):set_visible(true)
        ui_manager:get_widget("ItemMenu.Container.ItemOrder.Cursor"):set_visible(true)
        ui_manager:get_widget("ItemMenu.Container.ItemOrder.Cursor"):set_default_animation("Position" .. self.order_cursor_position)
    end,

    --- Initializes the item selection selection mode.
    submenu_items = function(self)
        UiContainer.current_submenu = "item_selection"
        ui_manager:get_widget("ItemMenu.Container.ItemList.Cursor"):set_visible(true)
        ui_manager:get_widget("ItemMenu.Container.ItemList.Cursor"):set_default_animation("Position" .. self.item_cursor_position)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.CursorMultiple"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.KeyItems.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemOrder.Cursor"):set_visible(false)
        --ui_manager:get_widget("ItemMenu.Container.ItemMenuTopBar.Cursor"):set_visible(false)
        -- Show description of the very first item, if any.
        -- TODO: Selected, not first
        -- Show description of the selected item, if any.
        local k_selected_id = (self.first_key_item_row_in_window + self.key_items_cursor_y_position) * 2 + self.key_items_cursor_x_position
        if (Inventory.key[k_selected_id] == true) then
            ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text(Game.Key_Items[k_selected_id].description)
        else
            ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text("")
        end


        if (Inventory[0] ~= nil) then
            ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text(Game.Items[Inventory[0].item].description)
        else
            ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text("")
        end
    end,

    --- Initializes the character selection mode.
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
        ui_manager:get_widget("ItemMenu.Container.KeyItems.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemOrder.Cursor"):set_visible(false)
    end,

    --- Initializes the key item selection mode.
    submenu_key_items = function(self)
        UiContainer.current_submenu = "key_item_selection"
        ui_manager:get_widget("ItemMenu.Container.ItemList.Cursor"):set_visible(true)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemMenuCharacters.CursorMultiple"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.ItemOrder.Cursor"):set_visible(false)
        ui_manager:get_widget("ItemMenu.Container.KeyItems.Cursor"):set_visible(true)
        ui_manager:get_widget("ItemMenu.Container.KeyItems.Cursor"):set_default_animation("Position" .. self.key_items_cursor_y_position .. "-" .. self.key_items_cursor_x_position)
        -- Show description of the selected item, if any.
        local k_selected_id = (self.first_key_item_row_in_window + self.key_items_cursor_y_position) * 2 + self.key_items_cursor_x_position
        if (Inventory.key[k_selected_id] == true) then
            ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text(Game.Key_Items[k_selected_id].description)
        else
            ui_manager:get_widget("ItemMenu.Container.ItemMenuSecondBar.ItemText"):set_text("")
        end
    end,

    --- Uses an item from the menu.
    --
    -- Checks the item conditions. If they are valid, uses the item and discounts 1 from the
    -- inventory.
    --
    -- @param item The ID of the item being used.
    -- @param character_pos Position in the party of the character on which to use the item (1-3).
    -- If nil, the item affects the whole party.
    use_item = function(self, item, character_pos)
        char_id = -1
        if character_pos then
            -- Normal item, use in one character.
            char_id = FFVII.Party[character_pos]
        end

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
            local inventory_index = i + first_item_in_window + 1
            if Inventory[inventory_index] == nil then
                w_icon:set_visible(false)
                w_name:set_visible(false)
                w_colon:set_visible(false)
                w_qty:set_visible(false)
            else
                local useable = Game.Items[Inventory[inventory_index].item].menu
                local type = Game.Items[Inventory[inventory_index].item].type
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
                -- Item icon
                if type == Inventory.ITEM_TYPE.ITEM then
                    w_icon:set_image("images/icons/item_item.png")
                elseif type == Inventory.ITEM_TYPE.ARMOR then
                    w_icon:set_image("images/icons/item_armor.png")
                elseif type == Inventory.ITEM_TYPE.ACCESSORY then
                    w_icon:set_image("images/icons/item_accessory.png")
                elseif type == Inventory.ITEM_TYPE.WEAPON then
                    local users = Game.Items[Inventory[inventory_index].item].users
                    for _, v in ipairs(users) do
                        print(Game.Items[Inventory[inventory_index].item].name .. " Users " .. tostring(v))
                        if v >= 0 and v <= 8 then
                            w_icon:set_image("images/icons/item_weapon_" .. v .. ".png")
                            break;
                        end
                    end

                end

                w_name:set_text(Game.Items[Inventory[inventory_index].item].name)
                w_qty:set_text(tostring(Inventory[inventory_index].quantity))
                w_colon:set_text(":") -- Refresh text to apply colour.
            end
        end
        return
    end,

    --- Populates the visible key items entries.
    --
    -- 20 entries visible at a time (10 rows).
    populate_key_items = function(self)
        local first_key_item_row_in_window = self.first_key_item_row_in_window
        for i = 0, 9 do
            local label_left = ui_manager:get_widget("ItemMenu.Container.KeyItems.KeyItem" .. tostring(i) .. "-0")
            local label_right = ui_manager:get_widget("ItemMenu.Container.KeyItems.KeyItem" .. tostring(i) .. "-1")
            local left_item_index = (first_key_item_row_in_window + i) * 2
            local right_item_index = left_item_index + 1
            if Inventory.has_key_item(left_item_index) == true then -- Left column
                label_left:set_text(Game.Key_Items[left_item_index].name)
            else
                label_left:set_text("")
            end
            if Inventory.has_key_item(right_item_index) == true then -- Right column
                label_right:set_text(Game.Key_Items[right_item_index].name)
            else
                label_right:set_text("")
            end
        end
        return
    end,

    --- Hides the item menu and goes back to the main menu.
    hide = function(self)
        ui_manager:get_widget("ItemMenu"):set_visible(false)
        UiContainer.current_menu = "main"
        UiContainer.current_submenu = ""
        return 0;
    end,
}
