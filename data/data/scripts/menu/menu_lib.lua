--- Shared building blocks for the menu scripts.
--
-- Provides:
--  * UiContainer.Cursor: a cursor over a fixed list of positions.
--  * UiContainer.ListCursor: a cursor over a scrollable list.
--  * UiContainer.Menu: a small state machine for menus (replaces the nested
--    "if current_menu == ... if current_submenu == ..." ladders).
--  * Formatting helpers and constants.
--
-- Menus still publish UiContainer.current_menu / UiContainer.current_submenu, so menus that haven't
-- been migrated yet keep working next to the migrated ones.

if UiContainer == nil then UiContainer = {} end

---------------------------------------------------------------------------------------------------
-- Constants and formatting
---------------------------------------------------------------------------------------------------

--- Status effect ids shown in menus.
UiContainer.Status = {
    SADNESS = 4,
    FURY = 5,
}

--- Formats a money amount, right aligned.
-- @param value The amount of gil.
-- @param length Width of the field. Defaults to 8.
UiContainer.format_money = function(value, length)
    return UiContainer.pad_value(value, length or 8)
end

--- Formats a number of seconds as HHH:MM.
UiContainer.format_time = function(seconds)
    return UiContainer.pad_value(math.floor(seconds / 3600), 3) .. ":"
        .. UiContainer.zero_pad_value(math.floor((seconds % 3600) / 60), 2)
end

--- Calculates how much of the way to the next level a character has done.
--
-- Uses the EXP curve of the character (see Game.Growth.curves): the EXP needed to attain a level
-- L is Mod * (L - 1)^2 / 10, where Mod is the gradient of the curve segment for L. The character
-- stores the EXP still missing for the next level (exp_to_next).
--
-- @param character Entry of the Characters table.
-- @return A number from 0 to 1. 0 if it can't be calculated (e.g. level 99).
UiContainer.exp_progress = function(character)
    local definition = Game.Characters[character.char_id]
    if definition == nil or definition.curves == nil or definition.curves.exp == nil then
        return 0
    end
    local curve = Game.Growth.curves[definition.curves.exp]
    local level = character.level
    local target = level + 1
    if curve == nil or target > 99 then return 0 end
    local segment
    if target <= 11 then segment = 0
    elseif target <= 21 then segment = 1
    elseif target <= 31 then segment = 2
    elseif target <= 41 then segment = 3
    elseif target <= 51 then segment = 4
    elseif target <= 61 then segment = 5
    elseif target <= 81 then segment = 6
    else segment = 7 end
    local span = math.floor(curve[segment].gradient * level * level / 10)
    if span <= 0 or character.exp_to_next == nil then return 0 end
    return math.max(0, math.min(1, 1 - character.exp_to_next / span))
end

--- Fills a two-line bar (widgets "<prefix>1" and "<prefix>2").
--
-- @param prefix Path of the bar widgets, without the final 1/2.
-- @param fraction Fill amount, 0 to 1.
-- @param width Width in pixels of the full bar.
UiContainer.set_bar = function(prefix, fraction, width)
    fraction = math.max(0, math.min(1, fraction))
    local pixels = math.floor(fraction * width)
    for line = 1, 2 do
        local widget = ui_manager:get_widget(prefix .. tostring(line))
        widget:set_visible(pixels > 0)
        widget:set_width(pixels)
    end
end

--- Is the cursor memory on? (Cursor: Memory in the config menu.)
--
-- If it is, the cursors of the menus and of the battle commands start where they were left. If not
-- (Initial), they start at the first position.
UiContainer.cursor_memory = function()
    return MenuSettings ~= nil and MenuSettings.config ~= nil and MenuSettings.config.cursor == 2
end

--- Maximum value of the limit bar of a character.
UiContainer.LIMIT_BAR_MAX = 255

--- Fills the limit bar of a character (see set_bar).
--
-- @param prefix Path of the bar widgets, without the final 1/2.
-- @param character Entry of the Characters table.
-- @param width Width in pixels of the full bar.
UiContainer.set_limit_bar = function(prefix, character, width)
    UiContainer.set_bar(prefix, character.limit.bar / UiContainer.LIMIT_BAR_MAX, width)
end

--- Plays a menu sound.
-- @param name One of "Cursor", "Back", "Error", "Window"...
UiContainer.play_sound = function(name)
    audio_manager:play_sound(name)
end

--- Resolves a widget given its full path, or returns the argument if it is already a widget.
local function resolve_widget(widget)
    if type(widget) == "string" then
        return ui_manager:get_widget(widget)
    end
    return widget
end

---------------------------------------------------------------------------------------------------
-- Cursor
---------------------------------------------------------------------------------------------------

--- A cursor over a fixed list of positions (1..total).
--
-- The cursor widget must define animations "Position1".."PositionN".
--
--     self.cursor = UiContainer.Cursor.new{
--         widget = "ItemMenu.Container.ItemOrder.Cursor",
--         total = 8,
--         on_move = function(position) ... end,  -- optional
--     }
--
-- Options:
--  * widget: Path of the cursor widget (resolved lazily), or a widget. Optional.
--  * total: Number of positions.
--  * wrap: Wrap around the ends. Defaults to true.
--  * sound: Sound played on move. Defaults to "Cursor". false for none.
--  * on_move: Called with the new position after every move.
UiContainer.Cursor = {}
UiContainer.Cursor.__index = UiContainer.Cursor

UiContainer.Cursor.new = function(options)
    local cursor = setmetatable({}, UiContainer.Cursor)
    cursor.widget = options.widget
    cursor.total = options.total or 1
    cursor.wrap = options.wrap ~= false
    cursor.sound = options.sound
    if cursor.sound == nil then cursor.sound = "Cursor" end
    cursor.on_move = options.on_move
    cursor.position = options.position or 1
    return cursor
end

--- Moves the cursor to a position and updates the widget. Plays no sound.
function UiContainer.Cursor:set(position)
    self.position = position
    if self.widget ~= nil then
        resolve_widget(self.widget):set_default_animation("Position" .. self.position)
    end
    if self.on_move ~= nil then self.on_move(self.position) end
end

--- Refreshes the widget with the current position.
function UiContainer.Cursor:refresh()
    self:set(self.position)
end

--- Moves by a delta. Returns true if the cursor actually moved.
function UiContainer.Cursor:move(delta)
    local target = self.position + delta
    if target > self.total then
        if not self.wrap then
            if self.sound then UiContainer.play_sound("Error") end
            return false
        end
        target = 1
    elseif target < 1 then
        if not self.wrap then
            if self.sound then UiContainer.play_sound("Error") end
            return false
        end
        target = self.total
    end
    if self.sound then UiContainer.play_sound(self.sound) end
    self:set(target)
    return true
end

function UiContainer.Cursor:next() return self:move(1) end
function UiContainer.Cursor:prev() return self:move(-1) end

--- Currently highlighted position.
function UiContainer.Cursor:index() return self.position end

--- Handles a directional button.
--
-- @param button Button name.
-- @param axis "vertical" (Up/Down) or "horizontal" (Left/Right).
-- @return true if the button was a direction of the axis (even if the cursor didn't move).
function UiContainer.Cursor:handle(button, axis)
    local forward, backward
    if axis == "horizontal" then
        forward, backward = Config.CONTROLS.RIGHT, Config.CONTROLS.LEFT
    else
        forward, backward = Config.CONTROLS.DOWN, Config.CONTROLS.UP
    end
    if button == forward then
        self:next()
        return true
    elseif button == backward then
        self:prev()
        return true
    end
    return false
end

--- Handlers for the direction buttons that move a cursor over a grid.
--
-- The cursor goes through the cells row by row, and wraps around at the ends of rows and columns.
-- Up and down keep the column, or go to the last cell of a shorter row. Use them as the `buttons` of
-- a state of a menu.
--
--     buttons = UiContainer.grid_buttons("level_cursor", 2),
--
-- @param cursor_name Name of the field of the menu that holds the cursor (a Cursor).
-- @param columns Number of columns of the grid.
UiContainer.grid_buttons = function(cursor_name, columns)
    local buttons = {}
    for _, button in ipairs({
        Config.CONTROLS.UP, Config.CONTROLS.DOWN, Config.CONTROLS.LEFT, Config.CONTROLS.RIGHT
    }) do
        buttons[button] = function(self, event)
            local cursor = self[cursor_name]
            local index = cursor:index() - 1
            local column = index % columns
            local row = math.floor(index / columns)
            local rows = math.ceil(cursor.total / columns)
            local function row_length(r) return math.min(columns, cursor.total - r * columns) end
            if button == Config.CONTROLS.LEFT then
                column = (column - 1) % row_length(row)
            elseif button == Config.CONTROLS.RIGHT then
                column = (column + 1) % row_length(row)
            else
                row = (row + ((button == Config.CONTROLS.DOWN) and 1 or -1)) % rows
                column = math.min(column, row_length(row) - 1)
            end
            UiContainer.play_sound("Cursor")
            cursor:set(row * columns + column + 1)
        end
    end
    return buttons
end

---------------------------------------------------------------------------------------------------
-- ListCursor
---------------------------------------------------------------------------------------------------

--- A cursor over a scrollable list: `rows` visible rows over `count` items.
--
-- The cursor moves inside the window until it hits an edge, then the window scrolls. It never
-- wraps.
--
-- Options:
--  * widget: Cursor widget, with animations "Position1".."PositionRows".
--  * rows: Visible rows.
--  * count: Total items in the list. Can be changed later through the `count` field.
--  * on_scroll: Called when the window scrolls (to repopulate the rows).
--  * on_move: Called after every successful move.
UiContainer.ListCursor = setmetatable({}, { __index = UiContainer.Cursor })
UiContainer.ListCursor.__index = UiContainer.ListCursor

UiContainer.ListCursor.new = function(options)
    local cursor = setmetatable({}, UiContainer.ListCursor)
    cursor.widget = options.widget
    cursor.rows = options.rows
    cursor.total = options.rows
    cursor.count = options.count or options.rows
    cursor.sound = options.sound
    if cursor.sound == nil then cursor.sound = "Cursor" end
    cursor.on_scroll = options.on_scroll
    cursor.on_move = options.on_move
    cursor.position = 1 -- Row inside the window.
    cursor.first = 1    -- First item shown in the window.
    return cursor
end

--- Index of the highlighted item in the whole list.
function UiContainer.ListCursor:index()
    return self.first + self.position - 1
end

--- Resets to the top of the list.
function UiContainer.ListCursor:reset()
    self.first = 1
    self:set(1)
end

function UiContainer.ListCursor:move(delta)
    local index = self:index() + delta
    if index < 1 or index > self.count then
        if self.sound then UiContainer.play_sound("Error") end
        return false
    end
    if self.sound then UiContainer.play_sound(self.sound) end
    local row = self.position + delta
    if row < 1 or row > self.rows then
        -- Scroll instead of moving the cursor.
        self.first = self.first + delta
        if self.on_scroll ~= nil then self.on_scroll(self.first) end
        if self.on_move ~= nil then self.on_move(self.position) end
    else
        self:set(row)
    end
    return true
end

---------------------------------------------------------------------------------------------------
-- Menu
---------------------------------------------------------------------------------------------------

--- Menu state machine.
--
-- A menu is declared as a table of states. Each state describes how it reacts to the buttons, and
-- UiContainer.Menu.new() adds the on_button / set_state plumbing:
--
--     UiContainer.PauseMenu = UiContainer.Menu.new{
--         id = "pause",              -- Value of UiContainer.current_menu while open.
--         initial_state = "",        -- State entered by open(). Defaults to "".
--         states = {
--             [""] = {
--                 cursor = function(self) return self.cursor end,
--                 axis = "vertical",          -- Directional axis handled by the cursor.
--                 confirm = function(self, cursor) ... end,
--                 cancel = function(self) ... end,
--                 buttons = {                 -- Any other button.
--                     [Config.ACTIONS.L1] = function(self, event) ... end,
--                 },
--             },
--         },
--         show = ..., hide = ...,
--     }
--
-- Rules:
--  * When a menu gives the control back (deactivate), the receiving menu's optional `on_resume`
--    hook runs, so it can refresh data the submenu may have changed.
--  * confirm and cancel only fire on "Press" (not on key repeat).
--  * Directions fire on "Press" and "Repeat".
--  * cancel is followed by the "Back" sound. confirm is followed by "Cursor". If either handler
--    returns false, "Error" is played instead.
--  * A state may set `confirm_sound = false` (or `cancel_sound = false`) when its handler plays its
--    own sound.
--  * A state may set `blocked = true` to swallow all input.
--  * A state may define `capture = function(self, key)`, called with the name of the key when the
--    player presses the key requested with input_manager:capture_next_key().
UiContainer.Menu = {}

--- Menus created with UiContainer.Menu.new(), by id.
UiContainer.menus = {}

--- Tells a menu that it has the control again after a submenu closed.
--
-- The menu runs its `on_resume` hook (if it has one), which is the place to refresh whatever the
-- submenu could have changed (stats, equipment, limit level...). Menus not migrated to
-- UiContainer.Menu should call this after giving the control back.
--
-- @param id Id of the menu receiving the control, e.g. "main".
UiContainer.resume_menu = function(id)
    local menu = UiContainer.menus[id]
    if menu ~= nil and menu.on_resume ~= nil then
        menu:on_resume()
    end
end

local menu_methods = {}

--- Switches to another state of the menu.
function menu_methods.set_state(self, state)
    UiContainer.current_submenu = state
end

--- Gets the current state name.
function menu_methods.get_state(self)
    return UiContainer.current_submenu
end

--- Marks this menu as the active one and enters its initial state.
function menu_methods.activate(self)
    UiContainer.current_menu = self.id
    UiContainer.current_submenu = self.initial_state or ""
end

--- Gives the control back to another menu.
--
-- @param id Id of the menu receiving the control (e.g. "main"). Defaults to `self.parent`.
function menu_methods.deactivate(self, id)
    UiContainer.current_menu = id or self.parent or ""
    UiContainer.current_submenu = ""
    UiContainer.resume_menu(UiContainer.current_menu)
end

--- Is this menu the one currently handling the input?
function menu_methods.is_active(self)
    return UiContainer.current_menu == self.id
end

--- Dispatches a button to the current state.
function menu_methods.on_button(self, button, event, key)
    if UiContainer.current_menu ~= self.id then
        if self.on_inactive_button ~= nil then
            return self:on_inactive_button(button, event) or 0
        end
        return 0
    end
    local state = self.states[UiContainer.current_submenu]
    if state == nil or state.blocked then return 0 end

    -- A key asked for with input_manager:capture_next_key() was pressed. The name of the key is the
    -- third argument.
    if button == "KEY_CAPTURE" then
        if state.capture ~= nil then state.capture(self, key) end
        return 0
    end

    local pressed = (event == "Press")
    local cursor = nil
    if state.cursor ~= nil then cursor = state.cursor(self) end

    if button == Config.ACTIONS.CANCEL then
        if pressed and state.cancel ~= nil then
            local result = state.cancel(self)
            if result == false then
                UiContainer.play_sound("Error")
            elseif state.cancel_sound ~= false then
                UiContainer.play_sound("Back")
            end
        end
    elseif button == Config.ACTIONS.CONFIRM then
        if pressed and state.confirm ~= nil then
            local result = state.confirm(self, cursor)
            if result == false then
                UiContainer.play_sound("Error")
            elseif state.confirm_sound ~= false then
                UiContainer.play_sound("Cursor")
            end
        end
    elseif cursor ~= nil and cursor:handle(button, state.axis) then
        return 0
    elseif state.buttons ~= nil and state.buttons[button] ~= nil then
        state.buttons[button](self, event)
    end
    return 0
end

--- Creates a menu from its definition.
--
-- Methods not defined by the menu are filled in with the defaults above (set_state, activate,
-- deactivate, is_active, on_button).
UiContainer.Menu.new = function(definition)
    for name, method in pairs(menu_methods) do
        if definition[name] == nil then definition[name] = method end
    end
    if definition.on_start == nil then
        definition.on_start = function(self) return 0 end
    end
    if definition.states == nil then definition.states = {} end
    if definition.id ~= nil then UiContainer.menus[definition.id] = definition end
    return definition
end
