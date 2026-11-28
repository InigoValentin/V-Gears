if UiContainer == nil then UiContainer = {} end

--- The shop menu.
--
-- Opened by the field scripts (open_shop). The shop to open is set in `shop_id` before showing the
-- menu. The inventory of each shop is in Game.Shops (game/kernel/shops.lua). Prices are the ones of
-- the items and materia, the same in every shop. Items are sold for half of their price, materia for its AP (see materia_sell_price).
--
-- States:
--  * "" (command): Buy, Sell or Exit.
--  * "buy": list of the things the shop sells.
--  * "buy_quantity": how many of the highlighted item to buy.
--  * "sell": list of the things of the party that can be sold.
--  * "sell_quantity": how many of the highlighted item to sell.
--  * "sell_materia": stars, AP and price of the highlighted materia, to confirm the sale.
UiContainer.ShopMenu = UiContainer.Menu.new{

    id = "shop",

    --- ID of the shop to open (index of Game.Shops).
    shop_id = 0,

    --- Visible rows of the list.
    ROWS = 8,

    --- Characters shown in the party window (the 9 that can be enabled).
    MAX_CHARACTERS = 9,

    --- Most items of a kind that the party can carry.
    MAX_QUANTITY = 99,

    --- Names of the kinds of shop (the type of the shop in Game.Shops).
    TITLES = {
        [1] = "Weapon Shop",
        [2] = "Item Shop",
        [3] = "Materia Shop",
        [4] = "Drug Store",
        [5] = "Tool Shop",
        [6] = "Accessory Shop",
        [7] = "Item Shop",
        [8] = "Item Shop",
    },

    --- Entries of the list being shown. Each one is {materia = bool, id = ID, price = gil, ...}.
    entries = {},

    --- Quantity chosen in the quantity states.
    quantity = 1,

    --- Most that can be chosen in the quantity states.
    max_quantity = 1,

    --- Cursor over Buy, Sell and Exit.
    command_cursor = UiContainer.Cursor.new{
        widget = "ShopMenu.Container.TopBar.Commands.Cursor",
        total = 3,
    },

    --- Cursor over the visible rows of the list, scrolling over the entries.
    list_cursor = UiContainer.ListCursor.new{
        widget = "ShopMenu.Container.List.Cursor",
        rows = 8,
        on_scroll = function() UiContainer.ShopMenu:populate_list() end,
        on_move = function() UiContainer.ShopMenu:populate_details() end,
    },

    states = {
        -- Buy, Sell, Exit.
        [""] = {
            cursor = function(self) return self.command_cursor end,
            axis = "horizontal",
            confirm_sound = false,
            confirm = function(self, cursor)
                local command = cursor:index()
                if command == 1 then
                    UiContainer.play_sound("Cursor")
                    self:enter_list("buy")
                elseif command == 2 then
                    if not self:enter_list("sell") then return false end
                    UiContainer.play_sound("Cursor")
                else
                    UiContainer.play_sound("Back")
                    script:request_end_sync(Script.UI, "ShopMenu", "hide", 0)
                end
            end,
            cancel = function(self)
                script:request_end_sync(Script.UI, "ShopMenu", "hide", 0)
            end,
        },

        buy = {
            cursor = function(self) return self.list_cursor end,
            axis = "vertical",
            confirm_sound = false,
            confirm = function(self, cursor)
                return self:start_buy(self.entries[cursor:index()])
            end,
            cancel = function(self) self:enter_command() end,
        },

        buy_quantity = {
            confirm_sound = false,
            confirm = function(self) return self:buy(self.entries[self.list_cursor:index()]) end,
            cancel = function(self) self:set_state("buy") self:update_prompt() end,
        },

        sell = {
            cursor = function(self) return self.list_cursor end,
            axis = "vertical",
            confirm_sound = false,
            confirm = function(self, cursor)
                return self:start_sell(self.entries[cursor:index()])
            end,
            cancel = function(self) self:enter_command() end,
        },

        -- Details of a materia before selling it: confirm sells it, cancel goes back to the list.
        sell_materia = {
            confirm_sound = false,
            confirm = function(self) return self:sell(self.entries[self.list_cursor:index()]) end,
            cancel = function(self) self:set_state("sell") self:update_prompt() end,
        },

        sell_quantity = {
            confirm_sound = false,
            confirm = function(self) return self:sell(self.entries[self.list_cursor:index()]) end,
            cancel = function(self) self:set_state("sell") self:update_prompt() end,
        },
    },

    -----------------------------------------------------------------------------------------------
    -- Opening and closing
    -----------------------------------------------------------------------------------------------

    --- Opens the shop set in `shop_id`.
    show = function(self)
        local shop = Game.Shops ~= nil and Game.Shops[self.shop_id] or nil
        if shop == nil then
            print("Tried to open invalid shop " .. tostring(self.shop_id))
            return 0
        end
        self.shop = shop
        self.materia_shop = (shop.type == 3)
        self:activate()
        -- Like the other menus opened by the scenes: the game is paused and the main menu is not
        -- available meanwhile.
        entity_manager:set_paused(true)
        MenuSettings.pause_available = false
        self.menu_was_available = MenuSettings.available
        MenuSettings.available = false
        ui_manager:get_widget("ShopMenu"):set_visible(true)
        ui_manager:get_widget("ShopMenu.Background"):set_visible(true)
        ui_manager:get_widget("ShopMenu.Container.Title.Title"):set_text(self.TITLES[shop.type] or "Shop")
        self:populate_party(nil)
        self.command_cursor:set(1)
        self:enter_command()
        return 0
    end,

    --- Closes the shop.
    hide = function(self)
        ui_manager:get_widget("ShopMenu.Background"):set_visible(false)
        ui_manager:get_widget("ShopMenu"):set_visible(false)
        self:deactivate()
        MenuSettings.available = self.menu_was_available
        MenuSettings.pause_available = true
        entity_manager:set_paused(false)
        return 0
    end,

    -----------------------------------------------------------------------------------------------
    -- States
    -----------------------------------------------------------------------------------------------

    --- Goes back to the Buy / Sell / Exit command.
    enter_command = function(self)
        self:set_state("")
        self.entries = {}
        self.list_cursor.count = 0
        self:update_prompt()
        self:populate_list()
        self:populate_details()
    end,

    --- Enters the list of things to buy or sell.
    --
    -- @param mode "buy" or "sell".
    -- @return False if there is nothing to list.
    enter_list = function(self, mode)
        local entries
        if mode == "buy" then
            entries = self:buy_entries()
        else
            entries = self:sell_entries()
        end
        if #entries == 0 then return false end
        self.entries = entries
        self.list_cursor.count = #entries
        self.list_cursor.first = 1
        self.list_cursor:set(1)
        self:set_state(mode)
        self:update_prompt()
        self:populate_list()
        self:populate_details()
        return true
    end,

    --- Shows the prompt (or the commands) that match the current state.
    update_prompt = function(self)
        local state = self:get_state()
        local bar = "ShopMenu.Container.TopBar."
        ui_manager:get_widget(bar .. "Commands"):set_visible(state == "")
        ui_manager:get_widget(bar .. "PromptBuy"):set_visible(state == "buy")
        ui_manager:get_widget(bar .. "PromptSell"):set_visible(state == "sell")
        ui_manager:get_widget(bar .. "PromptQuantity"):set_visible(
            state == "buy_quantity" or state == "sell_quantity")
        ui_manager:get_widget("ShopMenu.Container.MateriaSale"):set_visible(state == "sell_materia")
        ui_manager:get_widget("ShopMenu.Container.Quantity"):set_visible(
            state == "buy_quantity" or state == "sell_quantity")
        ui_manager:get_widget("ShopMenu.Container.List.Cursor"):set_visible(state ~= "")
    end,

    -----------------------------------------------------------------------------------------------
    -- Entries
    -----------------------------------------------------------------------------------------------

    --- Gets the data of the thing an entry refers to.
    entry_data = function(self, entry)
        if entry.materia then return Game.Materia[entry.id] end
        return Game.Items[entry.id]
    end,

    --- Entries of the things that the shop sells.
    buy_entries = function(self)
        local entries = {}
        for _, listed in ipairs(self.shop.items) do
            local entry = {materia = listed[1], id = listed[2]}
            local data = self:entry_data(entry)
            if data ~= nil then
                entry.price = data.price
                table.insert(entries, entry)
            end
        end
        return entries
    end,

    --- Entries of the things that the party can sell.
    --
    -- Materia shops buy materia. The others buy items, weapons, armors and accessories.
    sell_entries = function(self)
        local entries = {}
        if self.materia_shop then
            for slot = 1, MAX_MATERIA_SLOTS do
                local owned = Materia[slot]
                if owned ~= nil and Game.Materia[owned.id] ~= nil then
                    table.insert(entries, {
                        materia = true, id = owned.id, slot = slot, ap = owned.ap or 0,
                        price = self:materia_sell_price(owned.id, owned.ap or 0),
                        sellable = true,
                    })
                end
            end
        else
            for slot = 1, MAX_INVENTORY_SLOTS do
                local owned = Inventory[slot]
                if owned ~= nil and Game.Items[owned.item] ~= nil then
                    local data = Game.Items[owned.item]
                    table.insert(entries, {
                        materia = false, id = owned.item, quantity = owned.quantity,
                        price = math.floor(data.price / 2),
                        sellable = (data.sell ~= 0),
                    })
                end
            end
        end
        return entries
    end,

    --- Gil that a materia is sold for.
    --
    -- As in ff7.exe: a materia sells for the AP it has. A mastered one sells for 70 times its
    -- price, and the ones priced 1 (the master materia, which have no levels to master) for 1.
    --
    -- @param id ID of the materia.
    -- @param ap AP of the materia.
    materia_sell_price = function(self, id, ap)
        local data = Game.Materia[id]
        if data.price == 1 then return 1 end
        local levels = data.levels_ap or {}
        if #levels > 1 and ap >= levels[#levels] then return data.price * 70 end
        return ap
    end,

    --- Icon of an entry.
    entry_icon = function(self, entry)
        if entry.materia then
            return "icons/materia_" .. tostring(Game.Materia[entry.id].type) .. ".png"
        end
        local data = Game.Items[entry.id]
        if data.type == Inventory.ITEM_TYPE.ARMOR then
            return "icons/item_armor.png"
        elseif data.type == Inventory.ITEM_TYPE.ACCESSORY then
            return "icons/item_accessory.png"
        elseif data.type == Inventory.ITEM_TYPE.WEAPON then
            for _, user in ipairs(data.users or {}) do
                if user >= 0 and user <= 8 then
                    return "icons/item_weapon_" .. tostring(user) .. ".png"
                end
            end
        end
        return "icons/item_item.png"
    end,

    -----------------------------------------------------------------------------------------------
    -- Counting
    -----------------------------------------------------------------------------------------------

    --- How many of a thing does the party carry (not equipped)?
    owned_count = function(self, entry)
        if not entry.materia then return Inventory.get_item_quantity(entry.id) end
        local count = 0
        for slot = 1, MAX_MATERIA_SLOTS do
            if Materia[slot] ~= nil and Materia[slot].id == entry.id then count = count + 1 end
        end
        return count
    end,

    --- How many of a thing are equipped by the characters?
    equipped_count = function(self, entry)
        local count = 0
        for char_id = 0, 8 do
            local character = Characters[char_id]
            if type(character) == "table" and character.weapon ~= nil then
                if entry.materia then
                    for _, equipment in ipairs({character.weapon, character.armor}) do
                        for slot = 0, 7 do
                            local materia = equipment.materia ~= nil and equipment.materia[slot] or nil
                            if materia ~= nil and materia.id == entry.id then count = count + 1 end
                        end
                    end
                else
                    if character.weapon.id == entry.id then count = count + 1 end
                    if character.armor ~= nil and character.armor.id == entry.id then count = count + 1 end
                    if character.accessory == entry.id then count = count + 1 end
                end
            end
        end
        return count
    end,

    --- Is there room in the inventory for one more of a thing?
    has_room = function(self, entry)
        if entry.materia then
            for slot = 1, MAX_MATERIA_SLOTS do
                if Materia[slot] == nil then return true end
            end
            return false
        end
        if Inventory.get_item_quantity(entry.id) > 0 then
            return Inventory.get_item_quantity(entry.id) < self.MAX_QUANTITY
        end
        for slot = 1, MAX_INVENTORY_SLOTS do
            if Inventory[slot] == nil then return true end
        end
        return false
    end,

    -----------------------------------------------------------------------------------------------
    -- Buying
    -----------------------------------------------------------------------------------------------

    --- Starts buying the highlighted entry.
    --
    -- Materia is bought straight away. For anything else the quantity is asked.
    -- @return False if it can't be bought.
    start_buy = function(self, entry)
        if entry == nil or Inventory.money < entry.price or not self:has_room(entry) then
            return false
        end
        if entry.materia then
            return self:buy(entry)
        end
        local room = self.MAX_QUANTITY - Inventory.get_item_quantity(entry.id)
        local affordable = (entry.price > 0) and math.floor(Inventory.money / entry.price) or room
        self.max_quantity = math.max(1, math.min(room, affordable))
        self:enter_quantity("buy_quantity")
        UiContainer.play_sound("Cursor")
    end,

    --- Buys the chosen quantity of an entry.
    buy = function(self, entry)
        if entry == nil then return false end
        local quantity = entry.materia and 1 or self.quantity
        local total = entry.price * quantity
        if Inventory.money < total or not self:has_room(entry) then
            return false
        end
        Inventory.remove_money(total)
        if entry.materia then
            Materia.add(entry.id, 0)
        else
            Inventory.add_item(entry.id, quantity)
        end
        UiContainer.play_sound("Buy")
        self:set_state("buy")
        self:update_prompt()
        self:populate_details()
    end,

    -----------------------------------------------------------------------------------------------
    -- Selling
    -----------------------------------------------------------------------------------------------

    --- Starts selling the highlighted entry.
    --
    -- Materia is sold straight away. For anything else the quantity is asked.
    -- @return False if it can't be sold.
    start_sell = function(self, entry)
        -- Materia can be sold for 0 gil (it has no AP).
        if entry == nil or not entry.sellable then return false end
        if not entry.materia and entry.price <= 0 then return false end
        if entry.materia then
            self:set_state("sell_materia")
            self:update_prompt()
            self:populate_materia_sale(entry)
            UiContainer.play_sound("Cursor")
            return
        end
        self.max_quantity = entry.quantity
        self:enter_quantity("sell_quantity")
        UiContainer.play_sound("Cursor")
    end,

    --- Sells the chosen quantity of an entry.
    sell = function(self, entry)
        if entry == nil or not entry.sellable then return false end
        if entry.materia then
            Materia[entry.slot] = nil
            Inventory.add_money(entry.price)
        else
            Inventory.remove_item(entry.id, self.quantity)
            Inventory.add_money(entry.price * self.quantity)
        end
        UiContainer.play_sound("Buy")
        -- The list changes: build it again and keep the cursor where it was if it can.
        local first, position = self.list_cursor.first, self.list_cursor.position
        local entries = self:sell_entries()
        if #entries == 0 then
            self:enter_command()
            return
        end
        self.entries = entries
        self.list_cursor.count = #entries
        local index = math.min(first + position - 1, #entries)
        first = math.max(1, math.min(first, #entries - self.ROWS + 1))
        self.list_cursor.first = first
        self.list_cursor:set(index - first + 1)
        self:set_state("sell")
        self:update_prompt()
        self:populate_list()
        self:populate_details()
    end,

    -----------------------------------------------------------------------------------------------
    -- Quantity
    -----------------------------------------------------------------------------------------------

    --- Opens the window to choose how many to buy or sell.
    enter_quantity = function(self, state)
        self.quantity = 1
        self:set_state(state)
        self:update_prompt()
        self:populate_quantity()
    end,

    --- Changes the quantity.
    --
    -- @param delta Amount to add. The quantity is kept between 1 and the maximum.
    change_quantity = function(self, delta)
        local quantity = math.max(1, math.min(self.max_quantity, self.quantity + delta))
        if quantity == self.quantity then
            UiContainer.play_sound("Error")
            return
        end
        UiContainer.play_sound("Cursor")
        self.quantity = quantity
        self:populate_quantity()
    end,

    -----------------------------------------------------------------------------------------------
    -- Widgets
    -----------------------------------------------------------------------------------------------

    --- Fills the quantity window.
    populate_quantity = function(self)
        local entry = self.entries[self.list_cursor:index()]
        local window = "ShopMenu.Container.Quantity."
        ui_manager:get_widget(window .. "Name"):set_text(self:entry_data(entry).name)
        ui_manager:get_widget(window .. "Count"):set_text(UiContainer.pad_value(self.quantity, 2))
        ui_manager:get_widget(window .. "Total"):set_text(
            UiContainer.pad_value(entry.price * self.quantity, 8))
        self:populate_slots(entry)
    end,

    --- Shows the materia slots and links of a weapon or armor in the quantity window, as the
    -- equipment menu does. Other things have no slots.
    populate_slots = function(self, entry)
        local data = (not entry.materia) and Game.Items[entry.id] or nil
        local slots = {}
        local growth = 1
        if data ~= nil and (data.type == Inventory.ITEM_TYPE.WEAPON or data.type == Inventory.ITEM_TYPE.ARMOR) then
            slots = data.slots or {}
            growth = data.growth
        end
        local window = "ShopMenu.Container.Quantity."
        for s = 1, 8 do
            local slot = ui_manager:get_widget(window .. "Slot" .. s)
            slot:set_visible(#slots >= s)
            if growth == 0 then
                slot:set_image("icons/materia_slot_no_growth.png")
            else
                slot:set_image("icons/materia_slot.png")
            end
            if s % 2 ~= 0 then
                -- A link joins an odd slot with the next one.
                ui_manager:get_widget(window .. "Link" .. (math.floor(s / 2) + 1)):set_visible(
                    #slots > s and slots[s] == 1)
            end
        end
    end,

    --- Fills the window that shows a materia before it is sold.
    populate_materia_sale = function(self, entry)
        local data = Game.Materia[entry.id]
        local window = "ShopMenu.Container.MateriaSale."
        local levels = data.levels_ap or {}
        local level = 0
        for _, level_ap in ipairs(levels) do
            if level_ap > entry.ap then break end
            level = level + 1
        end
        ui_manager:get_widget(window .. "Icon"):set_image("icons/materia_" .. tostring(data.type) .. ".png")
        ui_manager:get_widget(window .. "Name"):set_text(data.name)
        for star = 1, 5 do
            local widget = ui_manager:get_widget(window .. "Star" .. star)
            if star > #levels then
                widget:set_visible(false)
            else
                widget:set_image((star <= level and "icons/materia_star_" or "icons/materia_star_empty_")
                    .. tostring(data.type) .. ".png")
                widget:set_visible(true)
            end
        end
        ui_manager:get_widget(window .. "Ap"):set_text(UiContainer.pad_value(entry.ap, 8))
        ui_manager:get_widget(window .. "Price"):set_text(UiContainer.pad_value(entry.price, 8))
    end,

    --- Fills the rows of the list.
    populate_list = function(self)
        local list = "ShopMenu.Container.List."
        for row = 1, self.ROWS do
            local entry = self.entries[self.list_cursor.first + row - 1]
            local icon = ui_manager:get_widget(list .. "Icon" .. row)
            local name = ui_manager:get_widget(list .. "Name" .. row)
            local price = ui_manager:get_widget(list .. "Price" .. row)
            if entry == nil then
                icon:set_visible(false)
                name:set_text("")
                price:set_text("")
            else
                icon:set_image(self:entry_icon(entry))
                icon:set_visible(true)
                name:set_text(self:entry_data(entry).name)
                price:set_text(UiContainer.pad_value(entry.price, 7))
                -- Things that can't be sold are shown dimmed.
                if entry.sellable == false or (self:get_state() == "sell" and not entry.materia and entry.price <= 0) then
                    name:set_colour(0.6, 0.6, 0.6)
                else
                    name:set_colour(1, 1, 1)
                end
            end
        end
    end,

    --- Fills the description, the gil, the amounts and the party for the highlighted entry.
    populate_details = function(self)
        local info = "ShopMenu.Container.Info."
        ui_manager:get_widget(info .. "Gil"):set_text(UiContainer.pad_value(Inventory.money, 8))
        local entry = self.entries[self.list_cursor:index()]
        if self:get_state() == "" or entry == nil then
            ui_manager:get_widget("ShopMenu.Container.Help.Help"):set_text("")
            ui_manager:get_widget(info .. "Owned"):set_text("")
            ui_manager:get_widget(info .. "Equipped"):set_text("")
            self:populate_party(nil)
            return
        end
        ui_manager:get_widget("ShopMenu.Container.Help.Help"):set_text(
            self:entry_data(entry).description or "")
        ui_manager:get_widget(info .. "Owned"):set_text(UiContainer.pad_value(self:owned_count(entry), 8))
        ui_manager:get_widget(info .. "Equipped"):set_text(UiContainer.pad_value(self:equipped_count(entry), 8))
        self:populate_party(entry)
    end,

    --- Fills the party window: portraits of every character enabled in the PHS, and how the
    -- stats change if the entry is equipped.
    --
    -- @param entry Highlighted entry, or nil to show only the portraits.
    populate_party = function(self, entry)
        local data = (entry ~= nil and not entry.materia) and Game.Items[entry.id] or nil
        local is_weapon = data ~= nil and data.type == Inventory.ITEM_TYPE.WEAPON
        local is_armor = data ~= nil and data.type == Inventory.ITEM_TYPE.ARMOR
        -- Each character has the spot of its ID, even if that leaves empty spots between them.
        for char_id = 0, self.MAX_CHARACTERS - 1 do
            local slot = char_id + 1
            local character = Characters[char_id]
            if not (type(character) == "table" and character.enabled == true) then
                ui_manager:get_widget("ShopMenu.Container.Party.Slot" .. slot):set_visible(false)
            else
                local widget = "ShopMenu.Container.Party.Slot" .. slot
                ui_manager:get_widget(widget):set_visible(true)
                ui_manager:get_widget(widget .. ".Portrait"):set_image("characters/" .. tostring(char_id) .. ".png")
                local can_use = false
                if is_weapon or is_armor then
                    for _, user in ipairs(data.users or {}) do
                        if user == char_id then can_use = true end
                    end
                end
                local stats = character.stats or {}
                local attack, defense = stats.atk or 0, stats.def or 0
                local new_attack, new_defense = attack, defense
                if can_use and is_weapon then
                    new_attack = attack - Game.Items[character.weapon.id].power + data.power
                elseif can_use and is_armor then
                    new_defense = defense - Game.Items[character.armor.id].defense.physical
                        + data.defense.physical
                end
                self:populate_stat(widget .. ".Atk", can_use, attack, new_attack)
                self:populate_stat(widget .. ".Def", can_use, defense, new_defense)
            end
        end
    end,

    --- Fills a stat row of a party member.
    --
    -- @param prefix Path of the row widgets, without the Lbl/Old/Arw/New suffix.
    -- @param visible Show the row?
    -- @param old Value now.
    -- @param new Value if equipped.
    populate_stat = function(self, prefix, visible, old, new)
        for _, part in ipairs({"Lbl", "Old", "Arw", "New"}) do
            ui_manager:get_widget(prefix .. part):set_visible(visible)
        end
        if not visible then return end
        ui_manager:get_widget(prefix .. "Old"):set_text(UiContainer.pad_value(old, 3))
        local value = ui_manager:get_widget(prefix .. "New")
        value:set_text(UiContainer.pad_value(new, 3))
        if new > old then
            value:set_colour(1, 1, 0)
        elseif new < old then
            value:set_colour(1, 0, 0)
        else
            value:set_colour(1, 1, 1)
        end
    end,
}

--- Directional buttons of the quantity states.
--
-- Up and down change the quantity by one, left and right by ten.
local function quantity_buttons()
    local deltas = {
        [Config.CONTROLS.UP] = 1,
        [Config.CONTROLS.DOWN] = -1,
        [Config.CONTROLS.RIGHT] = 10,
        [Config.CONTROLS.LEFT] = -10,
    }
    local buttons = {}
    for button, delta in pairs(deltas) do
        buttons[button] = function(self, event)
            if event == "Press" or event == "Repeat" then self:change_quantity(delta) end
        end
    end
    return buttons
end
UiContainer.ShopMenu.states.buy_quantity.buttons = quantity_buttons()
UiContainer.ShopMenu.states.sell_quantity.buttons = quantity_buttons()
