if UiContainer == nil then UiContainer = {} end

--- The materia exchange menu (Materia > Order > Exchange).
--
-- The left window lists every character enabled in the PHS, with the slots of their weapon and
-- armor. The right window is the materia of the inventory. The cursor goes through all the slots
-- and all the positions of the inventory.
--
-- Confirm on a position with a materia picks it up: its cursor stays blinking while the real one
-- moves. Confirm on another position swaps the two (the destination can be empty). Confirm on the
-- picked position, or cancel, drops it. Cancel with nothing picked closes the menu.
--
-- A position is a table: {kind = "equip", char = index in `chars`, part = "weapon" or "armor",
-- slot = 0-7} or {kind = "inv", index = slot of Materia}.
UiContainer.ExchangeMenu = UiContainer.Menu.new{

    id = "exchange",

    --- Menu that opened this one.
    parent = "materia",

    --- Character blocks shown at once in the left window.
    BLOCKS = 4,

    --- Rows shown at once in the inventory window.
    INV_ROWS = 14,

    --- Position and size of the elements of the left window.
    BLOCK_Y = 0,
    BLOCK_HEIGHT = 58,
    ROW_Y = {weapon = 22, armor = 39},
    SLOT_X = 70,
    SLOT_PITCH = 16,

    --- Position of the rows of the inventory window.
    INV_Y = 6,
    INV_PITCH = 16,

    --- IDs of the characters in the list (enabled in the PHS).
    chars = {},

    --- First character shown in the left window.
    first = 1,

    --- First inventory slot shown in the right window.
    inv_first = 1,

    --- Position of the cursor.
    pos = nil,

    --- Position of the picked materia, or nil.
    held = nil,

    --- Last position of the left window and of the inventory the cursor was on, to come back to.
    last_equip = nil,
    last_inv = 1,

    states = {
        [""] = {
            confirm_sound = false,
            cancel_sound = false,
            confirm = function(self) return self:confirm() end,
            cancel = function(self) return self:cancel() end,
            buttons = {},
        },
    },

    -----------------------------------------------------------------------------------------------
    -- Opening and closing
    -----------------------------------------------------------------------------------------------

    --- Opens the menu.
    show = function(self)
        self.chars = {}
        for char_id = 0, 8 do
            local character = Characters[char_id]
            if type(character) == "table" and character.enabled == true then
                self.chars[#self.chars + 1] = char_id
            end
        end
        self.first = 1
        self.inv_first = 1
        self.held = nil
        self.last_inv = 1
        self.last_equip = nil
        local rows = self:rows()
        if #rows > 0 then
            self.pos = {kind = "equip", char = rows[1].char, part = rows[1].part, slot = 0}
        else
            self.pos = {kind = "inv", index = 1}
        end
        self:activate()
        ui_manager:get_widget("MateriaMenu"):set_visible(false)
        ui_manager:get_widget("ExchangeMenu"):set_visible(true)
        self:refresh()
        return 0
    end,

    --- Closes the menu and goes back to the materia menu.
    hide = function(self)
        ui_manager:get_widget("ExchangeMenu"):set_visible(false)
        ui_manager:get_widget("MateriaMenu"):set_visible(true)
        UiContainer.current_menu = "materia"
        UiContainer.current_submenu = "order"
        -- The materia menu shows the equipment, so it has to know what changed.
        UiContainer.MateriaMenu:on_exchange_closed()
        return 0
    end,

    -----------------------------------------------------------------------------------------------
    -- Positions
    -----------------------------------------------------------------------------------------------

    --- Number of materia slots of the weapon or armor of a character of the list.
    slot_count = function(self, char, part)
        local equipment = Characters[self.chars[char]][part]
        return #(Game.Items[equipment.id].slots)
    end,

    --- Rows of the left window that have slots, in order.
    --
    -- @return List of {char = index in `chars`, part = "weapon" or "armor", slots = count}.
    rows = function(self)
        local rows = {}
        for char = 1, #self.chars do
            for _, part in ipairs({"weapon", "armor"}) do
                local count = self:slot_count(char, part)
                if count > 0 then rows[#rows + 1] = {char = char, part = part, slots = count} end
            end
        end
        return rows
    end,

    --- Gets the materia in a position (nil if empty).
    get = function(self, pos)
        if pos.kind == "inv" then return Materia[pos.index] end
        return Characters[self.chars[pos.char]][pos.part].materia[pos.slot]
    end,

    --- Puts a materia (or nil) in a position.
    set = function(self, pos, materia)
        if pos.kind == "inv" then
            Materia[pos.index] = materia
        else
            Characters[self.chars[pos.char]][pos.part].materia[pos.slot] = materia
        end
    end,

    --- Are two positions the same?
    same = function(self, a, b)
        if a == nil or b == nil or a.kind ~= b.kind then return false end
        if a.kind == "inv" then return a.index == b.index end
        return a.char == b.char and a.part == b.part and a.slot == b.slot
    end,

    --- Moves the cursor up or down.
    --
    -- In the left window it goes through the rows that have slots (wrapping around), keeping the
    -- column or going to the last slot of a shorter row. In the inventory it goes through all the
    -- slots, without wrapping.
    move_vertical = function(self, delta)
        local pos = self.pos
        if pos.kind == "inv" then
            local index = pos.index + delta
            if index < 1 or index > MAX_MATERIA_SLOTS then
                UiContainer.play_sound("Error")
                return
            end
            pos.index = index
        else
            local rows = self:rows()
            local current = 1
            for i, row in ipairs(rows) do
                if row.char == pos.char and row.part == pos.part then current = i end
            end
            local target = rows[(current - 1 + delta) % #rows + 1]
            pos.char, pos.part = target.char, target.part
            pos.slot = math.min(pos.slot, target.slots - 1)
        end
        UiContainer.play_sound("Cursor")
        self:refresh()
    end,

    --- Moves the cursor left or right.
    --
    -- Right from the last slot of a row goes to the inventory, and left from the inventory goes
    -- back to the slot it came from.
    move_horizontal = function(self, delta)
        local pos = self.pos
        if pos.kind == "inv" then
            if delta > 0 then
                UiContainer.play_sound("Error")
                return
            end
            local rows = self:rows()
            if #rows == 0 then
                UiContainer.play_sound("Error")
                return
            end
            self.last_inv = pos.index
            local back = self.last_equip or {char = rows[1].char, part = rows[1].part, slot = 0}
            self.pos = {kind = "equip", char = back.char, part = back.part, slot = back.slot}
        else
            local slot = pos.slot + delta
            if slot < 0 then
                UiContainer.play_sound("Error")
                return
            elseif slot >= self:slot_count(pos.char, pos.part) then
                self.last_equip = {char = pos.char, part = pos.part, slot = pos.slot}
                self.pos = {kind = "inv", index = self.last_inv}
            else
                pos.slot = slot
            end
        end
        UiContainer.play_sound("Cursor")
        self:refresh()
    end,

    --- Moves the cursor in the inventory by a page.
    move_page = function(self, delta)
        if self.pos.kind ~= "inv" then return end
        local index = math.max(1, math.min(MAX_MATERIA_SLOTS, self.pos.index + delta * self.INV_ROWS))
        if index == self.pos.index then
            UiContainer.play_sound("Error")
            return
        end
        UiContainer.play_sound("Cursor")
        self.pos.index = index
        self:refresh()
    end,

    -----------------------------------------------------------------------------------------------
    -- Actions
    -----------------------------------------------------------------------------------------------

    --- Confirm: picks up the materia of the cursor, or swaps it with the picked one.
    confirm = function(self)
        if self.held == nil then
            if self:get(self.pos) == nil then return false end
            UiContainer.play_sound("Cursor")
            self.held = {}
            for key, value in pairs(self.pos) do self.held[key] = value end
            self:refresh()
            return
        end
        if self:same(self.held, self.pos) then
            UiContainer.play_sound("Back")
            self.held = nil
            self:refresh()
            return
        end
        local from, to = self.held, self.pos
        local moved, replaced = self:get(from), self:get(to)
        self:set(from, replaced)
        self:set(to, moved)
        self.held = nil
        -- The materia change the stats of the characters involved, and the current HP and MP can't
        -- be over the new maximums.
        local recalculated = {}
        for _, pos in ipairs({from, to}) do
            if pos.kind == "equip" and not recalculated[pos.char] then
                recalculated[pos.char] = true
                Characters.recalculate_stats(self.chars[pos.char])
            end
        end
        UiContainer.play_sound("Cursor")
        self:refresh()
    end,

    --- Cancel: drops the picked materia, or closes the menu.
    cancel = function(self)
        if self.held ~= nil then
            self.held = nil
            self:refresh()
            UiContainer.play_sound("Back")
            return
        end
        UiContainer.play_sound("Back")
        script:request_end_sync(Script.UI, "ExchangeMenu", "hide", 0)
    end,

    -----------------------------------------------------------------------------------------------
    -- Widgets
    -----------------------------------------------------------------------------------------------

    --- Scrolls the windows so that the cursor is in them, and redraws everything.
    refresh = function(self)
        local pos = self.pos
        if pos.kind == "equip" then
            if pos.char < self.first then
                self.first = pos.char
            elseif pos.char > self.first + self.BLOCKS - 1 then
                self.first = pos.char - self.BLOCKS + 1
            end
        else
            if pos.index < self.inv_first then
                self.inv_first = pos.index
            elseif pos.index > self.inv_first + self.INV_ROWS - 1 then
                self.inv_first = pos.index - self.INV_ROWS + 1
            end
        end
        self:populate_equip()
        self:populate_inventory()
        self:place_cursors()
        local materia = self:get(pos)
        self:populate_info(materia)
        ui_manager:get_widget("ExchangeMenu.Container.Help.Help"):set_text(
            materia ~= nil and Game.Materia[materia.id].description or "")
    end,

    --- Fills the top bar with the icon, name and stars of a materia (nothing if it is nil).
    populate_info = function(self, materia)
        local bar = "ExchangeMenu.Container.Info."
        ui_manager:get_widget(bar .. "Icon"):set_visible(materia ~= nil)
        ui_manager:get_widget(bar .. "Name"):set_visible(materia ~= nil)
        local data = materia ~= nil and Game.Materia[materia.id] or nil
        local levels = data ~= nil and data.levels_ap or {}
        local level = 0
        for _, level_ap in ipairs(levels) do
            if level_ap > (materia.ap or 0) then break end
            level = level + 1
        end
        if data ~= nil then
            ui_manager:get_widget(bar .. "Icon"):set_image("icons/materia_" .. tostring(data.type) .. ".png")
            ui_manager:get_widget(bar .. "Name"):set_text(data.name)
        end
        for star = 1, 5 do
            local widget = ui_manager:get_widget(bar .. "Stars.Star" .. star)
            if data == nil or star > #levels then
                widget:set_visible(false)
            else
                widget:set_image((star <= level and "icons/materia_star_" or "icons/materia_star_empty_")
                    .. tostring(data.type) .. ".png")
                widget:set_visible(true)
            end
        end
    end,

    --- Fills the blocks of the left window.
    populate_equip = function(self)
        local window = "ExchangeMenu.Container.Characters."
        for b = 1, self.BLOCKS do
            local block = window .. "Block" .. b
            local char_id = self.chars[self.first + b - 1]
            ui_manager:get_widget(block):set_visible(char_id ~= nil)
            if char_id ~= nil then
                local character = Characters[char_id]
                ui_manager:get_widget(block .. ".Portrait"):set_image("characters/" .. tostring(char_id) .. ".png")
                ui_manager:get_widget(block .. ".Name"):set_text(character.name)
                ui_manager:get_widget(block .. ".WIcon"):set_image("icons/item_weapon_" .. tostring(char_id) .. ".png")
                self:populate_slots(block, "W", character.weapon)
                self:populate_slots(block, "A", character.armor)
            end
        end
        -- Scroll bar.
        local scroll = ui_manager:get_widget(window .. "Scroll")
        local hidden = math.max(0, #self.chars - self.BLOCKS)
        local fraction = hidden > 0 and (self.first - 1) / hidden or 0
        scroll:set_y(fraction * 100, 3 - fraction * 24)
        scroll:set_visible(hidden > 0)
        ui_manager:get_widget(window .. "ScrollBg"):set_visible(hidden > 0)
    end,

    --- Fills the slots of the weapon or armor of a block.
    --
    -- @param block Path of the block.
    -- @param prefix "W" or "A".
    -- @param equipment Weapon or armor of the character (an entry of Characters).
    populate_slots = function(self, block, prefix, equipment)
        local slots = Game.Items[equipment.id].slots
        for s = 1, 8 do
            local slot = ui_manager:get_widget(block .. "." .. prefix .. "Slot" .. s)
            local mat = ui_manager:get_widget(block .. "." .. prefix .. "Mat" .. s)
            slot:set_visible(#slots >= s)
            local materia = equipment.materia[s - 1]
            if #slots >= s and materia ~= nil then
                mat:set_image("icons/materia_" .. tostring(Game.Materia[materia.id].type) .. ".png")
                mat:set_visible(true)
            else
                mat:set_visible(false)
            end
            -- A link joins an odd slot with the next one.
            if s % 2 ~= 0 then
                ui_manager:get_widget(block .. "." .. prefix .. "Link" .. (math.floor(s / 2) + 1)):set_visible(
                    #slots > s and slots[s] == 1)
            end
        end
    end,

    --- Fills the rows of the inventory window.
    populate_inventory = function(self)
        local window = "ExchangeMenu.Container.Inventory."
        for r = 1, self.INV_ROWS do
            local materia = Materia[self.inv_first + r - 1]
            local icon = ui_manager:get_widget(window .. "Icon" .. r)
            local name = ui_manager:get_widget(window .. "Name" .. r)
            if materia ~= nil then
                local data = Game.Materia[materia.id]
                icon:set_image("icons/materia_" .. tostring(data.type) .. ".png")
                icon:set_visible(true)
                name:set_text(data.name)
                name:set_visible(true)
            else
                icon:set_visible(false)
                name:set_visible(false)
            end
        end
        local scroll = ui_manager:get_widget(window .. "Scroll")
        local fraction = (self.inv_first - 1) / (MAX_MATERIA_SLOTS - self.INV_ROWS)
        scroll:set_y(fraction * 100, 3 - fraction * 24)
    end,

    --- Coordinates of a position inside its window, and the window.
    --
    -- @return Window ("Characters" or "Inventory"), x, y, or nil if the position is not visible.
    cursor_place = function(self, pos)
        if pos.kind == "inv" then
            local row = pos.index - self.inv_first
            if row < 0 or row >= self.INV_ROWS then return nil end
            return "Inventory", -8, self.INV_Y + self.INV_PITCH * row - 1
        end
        local block = pos.char - self.first
        if block < 0 or block >= self.BLOCKS then return nil end
        return "Characters", self.SLOT_X + self.SLOT_PITCH * pos.slot - 22,
            self.BLOCK_Y + self.BLOCK_HEIGHT * block + self.ROW_Y[pos.part] - 1
    end,

    --- Places the cursor and the blinking cursor of the picked materia.
    place_cursors = function(self)
        local base = "ExchangeMenu.Container."
        for _, window in ipairs({"Characters", "Inventory"}) do
            ui_manager:get_widget(base .. window .. ".Cursor"):set_visible(false)
            ui_manager:get_widget(base .. window .. ".Held"):set_visible(false)
        end
        local function place(pos, name)
            if pos == nil then return end
            local window, x, y = self:cursor_place(pos)
            if window == nil then return end
            local cursor = ui_manager:get_widget(base .. window .. "." .. name)
            cursor:set_x(0, x)
            cursor:set_y(0, y)
            cursor:set_visible(true)
            if name == "Held" then cursor:set_default_animation("Blink") end
        end
        place(self.pos, "Cursor")
        place(self.held, "Held")
    end,
}

--- Directional buttons and pages.
local function direction(handler)
    return function(self, event)
        if event == "Press" or event == "Repeat" then handler(self) end
    end
end

local buttons = UiContainer.ExchangeMenu.states[""].buttons
buttons[Config.CONTROLS.UP] = direction(function(self) self:move_vertical(-1) end)
buttons[Config.CONTROLS.DOWN] = direction(function(self) self:move_vertical(1) end)
buttons[Config.CONTROLS.LEFT] = direction(function(self) self:move_horizontal(-1) end)
buttons[Config.CONTROLS.RIGHT] = direction(function(self) self:move_horizontal(1) end)
buttons[Config.ACTIONS.L1] = direction(function(self) self:move_page(-1) end)
buttons[Config.ACTIONS.R1] = direction(function(self) self:move_page(1) end)
