if UiContainer == nil then UiContainer = {} end

--- The main menu.
UiContainer.ItemMenu = {
    bar_position = 1,
    bar_position_total = 3,
    item_cursor_position = 1,
    item_cursor_position_total = 10,
    item_cursor_item_selected = 0,
    char_position = 1,
    char_position_total = 10,
    first_item_in_window = 0,

    on_start = function(self)
        return 0
    end,

    on_button = function(self, button, event)
        if UiContainer.current_menu == "item" then
            if UiContainer.current_submenu == "" then
                if button == "Escape" and event == "Press" then
                    script:request_end_sync(Script.UI, "ItemMenu", "hide", 0)
                elseif button == "Right" then

                elseif button == "Left" then

                elseif button == "Enter" then
                    if bar_position == 1 then
                        print("Select item")
                    elseif bar_position == 2 then
                        print("Unimplemented: Order")
                    elseif bar_position == 3 then
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
                        self.populate_items()
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
                    if self.item_cursor_position == self.item_cursor_position_total then
                        -- The cursor is at the end. Scroll page, but dont change cursor.
                        self.first_item_in_window = self.first_item_in_window - 1
                        self.populate_items()
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
                end
            elseif UiContainer.current_submenu == "char_selection" then
            elseif UiContainer.current_submenu == "key_items" then
            end
        end
        --[[if ui_manager:get_widget("ItemMenu"):is_visible() == true then
            local characters  = ui_manager:get_widget("MainMenu.Container.Characters")
            local menu        = ui_manager:get_widget("MainMenu.Container.Menu")
            local menu_cursor = ui_manager:get_widget("MainMenu.Container.Menu.Cursor")
            local timegil     = ui_manager:get_widget("MainMenu.Container.TimeGil")
            local location    = ui_manager:get_widget("MainMenu.Container.Location")

            if button == "Escape" and event == "Press" then
                script:request_end_sync(Script.UI, "ItemMenu", "hide", 0)
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
        elseif FFVII.MenuSettings.available == true then
            if button == "Escape" and event == "Press" then
                script:request_end_sync(Script.UI, "ItemMenu", "show", 0)
            end
        end
        ]]
        return 0
    end,

    --- Opens the main menu.
    --
    -- Populates and updates displayed data before opening.
    show = function(self)
        ui_manager:get_widget("ItemMenu"):set_visible(true)
        UiContainer.current_menu = "item"
        UiContainer.current_submenu = "item_selection"
        Inventory.add_item(0, 7)
        Inventory.add_item(1, 4)
        Inventory.add_item(42, 3)
        Inventory.add_item(1, 4)
        self.populate_items(self)
        return 0;
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
