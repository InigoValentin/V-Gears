if UiContainer == nil then UiContainer = {} end

local ROOT = "PhsMenu.Container."

--- Slots of the party (left) and of the reserve (right, a grid of 3x3).
local PARTY_SLOTS = 3
local RESERVE_COLUMNS = 3
local RESERVE_ROWS = 3
local RESERVE_SLOTS = RESERVE_COLUMNS * RESERVE_ROWS

--- The cursor goes over "cells": 1-3 are the party slots, 4-12 the reserve slots.
local TOTAL_CELLS = PARTY_SLOTS + RESERVE_SLOTS

--- Highest character id the PHS shows (Cid). Young Cloud and Sephiroth can't be chosen, and the
-- nine characters fit in the party (3) and the reserve (9).
local MAX_CHARACTER_ID = 8

local function is_party_cell(cell)
    return cell <= PARTY_SLOTS
end

--- Gets the cell the cursor goes to when a direction is pressed.
--
-- @param cell Current cell.
-- @param button A direction button.
-- @return The new cell, or nil if the cursor can't move that way.
local function neighbour(cell, button)
    local UP, DOWN = Config.CONTROLS.UP, Config.CONTROLS.DOWN
    local LEFT, RIGHT = Config.CONTROLS.LEFT, Config.CONTROLS.RIGHT
    if is_party_cell(cell) then
        if button == UP then
            return (cell - 2) % PARTY_SLOTS + 1
        elseif button == DOWN then
            return cell % PARTY_SLOTS + 1
        elseif button == RIGHT then
            -- First column of the reserve, in the same row.
            return PARTY_SLOTS + (cell - 1) * RESERVE_COLUMNS + 1
        end
        return nil
    end

    local index = cell - PARTY_SLOTS - 1
    local column = index % RESERVE_COLUMNS
    local row = math.floor(index / RESERVE_COLUMNS)
    if button == LEFT then
        if column > 0 then return cell - 1 end
        return math.min(row + 1, PARTY_SLOTS)
    elseif button == RIGHT then
        if column < RESERVE_COLUMNS - 1 then return cell + 1 end
    elseif button == UP then
        return PARTY_SLOTS + ((row - 1) % RESERVE_ROWS) * RESERVE_COLUMNS + column + 1
    elseif button == DOWN then
        return PARTY_SLOTS + ((row + 1) % RESERVE_ROWS) * RESERVE_COLUMNS + column + 1
    end
    return nil
end

--- Is a character locked in its place? Locked characters can't be moved or replaced.
local function is_locked(char_id)
    return char_id ~= nil and Characters[char_id].locked == true
end

--- Number of characters in the party.
local function party_count()
    local count = 0
    for slot = 1, PARTY_SLOTS do
        if Party[slot] ~= nil then count = count + 1 end
    end
    return count
end

--- Handlers of the directional buttons.
local function direction_buttons()
    local buttons = {}
    for _, button in ipairs({
        Config.CONTROLS.UP, Config.CONTROLS.DOWN, Config.CONTROLS.LEFT, Config.CONTROLS.RIGHT
    }) do
        buttons[button] = function(self, event)
            local target = neighbour(self.cursor:index(), button)
            if target ~= nil then
                UiContainer.play_sound("Cursor")
                self.cursor:set(target)
            end
        end
    end
    return buttons
end

--- The PHS menu: changes the characters of the party.
--
-- The party is on the left and the rest of the available characters on the right. A character is
-- picked with OK, and then OK on another place swaps both:
--  * party <-> party: changes the order (or moves the character to an empty slot).
--  * party <-> reserve: the character leaves the party and the other one (if any) takes its place.
--  * reserve <-> reserve: not allowed.
-- Locked characters can't be moved or replaced. The party can only be left once it has three
-- characters, or all the available ones if there are fewer.
UiContainer.PhsMenu = UiContainer.Menu.new{

    id = "phs",
    parent = "main",

    --- Cell the cursor is on (see TOTAL_CELLS).
    cursor = UiContainer.Cursor.new{
        widget = ROOT .. "Cursor",
        total = TOTAL_CELLS,
        sound = false,
        on_move = function(cell)
            UiContainer.PhsMenu:update_info(cell)
        end,
    },

    --- Cell of the picked character, or nil.
    held = nil,

    --- IDs of the available characters that aren't in the party, in the order they are shown.
    reserve = {},

    states = {
        [""] = {
            buttons = direction_buttons(),
            confirm = function(self)
                return self:on_confirm(self.cursor:index())
            end,
            cancel = function(self)
                if self.held ~= nil then
                    self:set_held(nil)
                    return
                end
                if not self:is_party_complete() then return false end
                script:request_end_sync(Script.UI, "PhsMenu", "hide", 0)
            end,
        },
    },

    --- Gets the character in a cell.
    -- @return Character ID, or nil if the cell is empty.
    get_character = function(self, cell)
        if is_party_cell(cell) then return Party[cell] end
        return self.reserve[cell - PARTY_SLOTS]
    end,

    --- Puts a character in a cell, or empties it.
    set_character = function(self, cell, char_id)
        if is_party_cell(cell) then
            Party[cell] = char_id
        else
            self.reserve[cell - PARTY_SLOTS] = char_id
        end
    end,

    --- Number of characters in the reserve. The reserve can have empty cells between them.
    reserve_count = function(self)
        local count = 0
        for slot = 1, RESERVE_SLOTS do
            if self.reserve[slot] ~= nil then count = count + 1 end
        end
        return count
    end,

    --- Fills the reserve with the available characters that aren't in the party.
    --
    -- Characters are packed from the first cell, in order. From then on they keep their cell
    -- while the party is being formed (see swap).
    refresh_reserve = function(self)
        self.reserve = {}
        for id = 0, MAX_CHARACTER_ID do
            local character = Characters[id]
            if character ~= nil and character.enabled == true then
                local in_party = false
                for slot = 1, PARTY_SLOTS do
                    if Party[slot] == id then in_party = true end
                end
                if not in_party then
                    self.reserve[#self.reserve + 1] = id
                end
            end
        end
    end,

    --- Has the party all the characters it can have?
    --
    -- Three are needed, unless there are fewer characters available.
    is_party_complete = function(self)
        local available = party_count() + self:reserve_count()
        return party_count() >= math.min(PARTY_SLOTS, available)
    end,

    --- Picks a character, or drops the one that was picked.
    -- @param cell Cell that was selected. nil to drop.
    set_held = function(self, cell)
        self.held = cell
        local held = ui_manager:get_widget(ROOT .. "Held")
        if cell == nil then
            held:set_visible(false)
        else
            held:set_default_animation("Position" .. tostring(cell))
            held:set_visible(true)
        end
    end,

    --- OK was pressed on a cell.
    -- @return false if the action isn't allowed.
    on_confirm = function(self, cell)
        local character = self:get_character(cell)
        if self.held == nil then
            -- Pick a character.
            if character == nil or is_locked(character) then return false end
            self:set_held(cell)
            return
        end

        -- Picking the same one again drops it.
        if self.held == cell then
            self:set_held(nil)
            return
        end

        if not self:swap(self.held, cell) then return false end
        self:set_held(nil)
        self:populate()
    end,

    --- Swaps the characters of two cells.
    -- @return false if it isn't allowed.
    swap = function(self, from, to)
        if not is_party_cell(from) and not is_party_cell(to) then return false end
        local a = self:get_character(from)
        local b = self:get_character(to)
        if is_locked(a) or is_locked(b) then return false end

        -- Both cells exchange their contents, so nobody in the reserve changes place.
        self:set_character(from, b)
        self:set_character(to, a)
        set_party(Party[1], Party[2], Party[3])
        -- TODO: If the leader (first slot) changed, change the player entity of the field too.
        return true
    end,

    --- Moves the characters of the party to the first slots, with no gaps between them.
    compact_party = function(self)
        local members = {}
        for slot = 1, PARTY_SLOTS do
            if Party[slot] ~= nil then members[#members + 1] = Party[slot] end
        end
        set_party(members[1], members[2], members[3])
    end,

    --- Shows the characters of the party and of the reserve.
    populate = function(self)
        self:update_info(self.cursor:index())
        for slot = 1, PARTY_SLOTS do
            local path = ROOT .. "Party.Slot" .. tostring(slot)
            local id = Party[slot]
            if id ~= nil then
                UiContainer.populate_character_data(path, Characters[id])
                ui_manager:get_widget(path .. ".Portrait"):set_image("characters/" .. tostring(id) .. ".png")
            else
                UiContainer.populate_character_data(path, nil)
            end
        end
        for slot = 1, RESERVE_SLOTS do
            local path = ROOT .. "Reserve.Slot" .. tostring(slot)
            local id = self.reserve[slot]
            ui_manager:get_widget(path):set_visible(id ~= nil)
            if id ~= nil then
                ui_manager:get_widget(path .. ".Portrait"):set_image("characters/" .. tostring(id) .. ".png")
            end
        end
    end,

    --- Shows the details of the character under the cursor in the top right window.
    --
    -- Only for the reserve: the characters of the party already show them next to their portrait.
    update_info = function(self, cell)
        local path = ROOT .. "Info.Character"
        local id = nil
        if not is_party_cell(cell) then id = self:get_character(cell) end
        if id ~= nil then
            UiContainer.populate_character_data(path, Characters[id])
        else
            UiContainer.populate_character_data(path, nil)
        end
    end,

    --- Opens the PHS menu.
    show = function(self)
        self:activate()
        -- Empty slots may hold an invalid ID (e.g. -1). Leave them as nil.
        for slot = 1, PARTY_SLOTS do
            if Party[slot] ~= nil and Characters[Party[slot]] == nil then Party[slot] = nil end
        end
        set_party(Party[1], Party[2], Party[3])
        -- Whoever is in the party is available, even if no script enabled them.
        for slot = 1, PARTY_SLOTS do
            if Party[slot] ~= nil then Characters[Party[slot]].enabled = true end
        end
        self:refresh_reserve()
        self:set_held(nil)
        self:populate()
        self.cursor:set(1)
        ui_manager:get_widget("PhsMenu"):set_visible(true)
        return 0
    end,

    --- Closes the PHS menu and goes back to the main menu.
    hide = function(self)
        self:compact_party()
        ui_manager:get_widget("PhsMenu"):set_visible(false)
        self:deactivate()
        return 0
    end,
}
