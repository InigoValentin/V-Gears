if UiContainer == nil then UiContainer = {} end

local ROOT = "MagicMenu.Container."

--- Rows visible in the list, and abilities in each row.
local ROWS = 9
local COLUMNS = 4

--- Categories of the abilities, in the order of the options of the menu.
local KINDS = { "magic", "summon", "e_skill" }

--- Colour of the abilities that can be used from the menu, and of the rest.
local COLOUR_USABLE = { 1.0, 1.0, 1.0 }
local COLOUR_UNUSABLE = { 0.45, 0.45, 0.35 }

--- Number of uses that means a summon can be used without limit.
local UNLIMITED = 255

--- Attacks (by ID) that can be used from the menu, out of battle. The rest are shown in gray.
--
-- The data of the attacks has a condition for the menu, but it is not reliable (Life has none, and
-- some spells that can't be cast out of battle have one), so the list is explicit.
local MENU_ATTACKS = {
    [0] = true,  -- Cure
    [1] = true,  -- Cure2
    [2] = true,  -- Cure3
    [7] = true,  -- Life
    [8] = true,  -- Life2
}

--- Creates the entry of the list for an attack.
--
-- @param id ID of the attack.
-- @param uses Times it can be used (summons), or nil.
-- @param mp MP the character has now. The attacks that cost more are not usable.
local function create_entry(id, uses, mp)
    local attack = Game.Attacks[id]
    if attack == nil then return nil end
    return {
        id = id,
        name = attack.name,
        description = attack.description or "",
        mp = attack.mp or 0,
        uses = uses,
        usable = MENU_ATTACKS[id] == true and (attack.mp or 0) <= mp,
    }
end

--- Splits a list in rows of at most COLUMNS elements.
local function split_in_rows(list)
    local rows = {}
    for index, item in ipairs(list) do
        local row = math.floor((index - 1) / COLUMNS) + 1
        rows[row] = rows[row] or {}
        rows[row][#rows[row] + 1] = item
    end
    return rows
end

--- The magic menu.
--
-- At first the player chooses what to see (magic, summons or enemy skills) in the box at the right,
-- and the abilities of that kind are shown below in a grid of four columns. Magic has a row for each
-- materia, with the spells it gives. Summons and enemy skills fill the rows in order. OK goes into
-- the grid, where the cursor moves over the abilities. The description of the one that is highlighted is shown in the help
-- window, and the MP it needs (and the times it can be used, for summons) in the box at the right.
--
-- The abilities that can be cast from the menu (MENU_ATTACKS) are white, as long as the character
-- has the MP they need. The rest are gray.
--
-- OK on one of them opens a window with the party to choose who receives it. The spell heals or
-- revives and the MP are spent, or it fails (Error sound) and nothing is spent.
-- TODO: Added-Ability is always "Nothing." The ability linked to the materia isn't calculated.
UiContainer.MagicMenu = UiContainer.Menu.new{

    id = "magic",
    parent = "main",

    --- ID of the character being shown.
    char_id = -1,

    --- Rows of the grid of the character, by kind (magic, summon, e_skill). Each row is a list of
    -- up to COLUMNS entries.
    rows = { magic = {}, summon = {}, e_skill = {} },

    --- Kind of abilities whose grid is shown.
    kind = "magic",

    --- Position of the cursor in the grid: row and column of the highlighted ability, and the first
    -- row that is visible.
    row = 1,
    column = 1,
    first_row = 1,

    --- Cursor over the options: magic, summon and enemy skills.
    category_cursor = UiContainer.Cursor.new{
        widget = ROOT .. "Side.Options.Cursor",
        total = #KINDS,
        on_move = function(position)
            UiContainer.MagicMenu:show_category(position)
        end,
    },

    --- Party slots shown in the window to choose the target of a spell, by position of the cursor.
    target_slots = {},

    --- Cursor over the characters of the party, to choose who receives the spell.
    target_cursor = UiContainer.Cursor.new{
        total = 1,
        on_move = function(position)
            UiContainer.MagicMenu:update_target_cursor(position)
        end,
    },

    states = {
        --- Choosing what to see.
        [""] = {
            cursor = function(self) return self.category_cursor end,
            axis = "vertical",
            confirm = function(self)
                if #self.rows[self.kind] == 0 then return false end
                self:set_browsing(true)
                self.row, self.column, self.first_row = 1, 1, 1
                self:update_cursor()
                self:set_state("list")
            end,
            cancel = function(self)
                script:request_end_sync(Script.UI, "MagicMenu", "hide", 0)
            end,
        },

        --- Moving over the abilities of the list.
        list = {
            confirm = function(self)
                local entry = self.rows[self.kind][self.row][self.column]
                if not entry.usable then return false end
                self:open_targets()
                self:set_state("target")
            end,
            buttons = {
                [Config.CONTROLS.UP] = function(self, event) self:move_cursor(-1, 0) end,
                [Config.CONTROLS.DOWN] = function(self, event) self:move_cursor(1, 0) end,
                [Config.CONTROLS.LEFT] = function(self, event) self:move_cursor(0, -1) end,
                [Config.CONTROLS.RIGHT] = function(self, event) self:move_cursor(0, 1) end,
            },
            cancel = function(self)
                self:set_browsing(false)
                self:set_state("")
            end,
        },

        --- Choosing the character that receives the spell.
        target = {
            cursor = function(self) return self.target_cursor end,
            axis = "vertical",
            confirm = function(self, cursor)
                return self:cast_spell(cursor:index())
            end,
            cancel = function(self)
                self:close_targets()
                self:set_state("list")
            end,
        },
    },

    --- Opens the window to choose the character that receives the spell.
    open_targets = function(self)
        self.target_slots = {}
        for slot = 1, Party.MAX do
            if Party[slot] ~= nil and Characters[Party[slot]] ~= nil then
                self.target_slots[#self.target_slots + 1] = slot
            end
        end
        self.target_cursor.total = #self.target_slots
        self:populate_targets()
        ui_manager:get_widget(ROOT .. "Targets"):set_visible(true)
        self.target_cursor:set(1)
    end,

    --- Closes the window to choose the target.
    close_targets = function(self)
        ui_manager:get_widget(ROOT .. "Targets"):set_visible(false)
    end,

    --- Shows the data of the party in the window to choose the target.
    populate_targets = function(self)
        for slot = 1, Party.MAX do
            local path = ROOT .. "Targets.Slot" .. tostring(slot)
            local id = Party[slot]
            if id ~= nil and Characters[id] ~= nil then
                UiContainer.populate_character_data(path, Characters[id])
                ui_manager:get_widget(path .. ".Portrait"):set_image("characters/" .. tostring(id) .. ".png")
            else
                UiContainer.populate_character_data(path, nil)
            end
        end
    end,

    --- Puts the cursor of the targets on a character.
    -- @param position Position in target_slots.
    update_target_cursor = function(self, position)
        local slot = self.target_slots[position]
        if slot ~= nil then
            ui_manager:get_widget(ROOT .. "Targets.Cursor"):set_default_animation("Position" .. tostring(slot))
        end
    end,

    --- Casts the highlighted spell on a character of the party.
    --
    -- @param position Position of the target in target_slots.
    -- @return False if the spell can't be cast (Error sound).
    cast_spell = function(self, position)
        local entry = self.rows[self.kind][self.row][self.column]
        local target = Party[self.target_slots[position]]
        if not Characters.cast_menu_spell(self.char_id, entry.id, target) then return false end

        -- HP and MP changed.
        self:populate_targets()
        UiContainer.populate_character_data(ROOT .. "Character", Characters[self.char_id])
        self:refresh_usable()
        -- Without MP for another one, go back to the list.
        if not entry.usable then
            self:close_targets()
            self:set_state("list")
        end
    end,

    --- Updates which spells can be cast, as the MP of the character changed.
    refresh_usable = function(self)
        local mp = Characters[self.char_id].stats.mp.current
        for _, rows in pairs(self.rows) do
            for _, row in ipairs(rows) do
                for _, entry in ipairs(row) do
                    entry.usable = MENU_ATTACKS[entry.id] == true and entry.mp <= mp
                end
            end
        end
        self:populate_list()
    end,

    --- Builds the grids of abilities of the character.
    build_rows = function(self)
        local abilities = Characters.get_abilities(self.char_id)
        local mp = Characters[self.char_id].stats.mp.current
        local rows = { magic = {}, summon = {}, e_skill = {} }
        -- Magic: one row for each materia (more rows if it gives more than COLUMNS spells).
        for _, group in ipairs(abilities.magic_groups) do
            local entries = {}
            for _, id in ipairs(group) do entries[#entries + 1] = create_entry(id, nil, mp) end
            for _, row in ipairs(split_in_rows(entries)) do rows.magic[#rows.magic + 1] = row end
        end
        local summons = {}
        for _, summon in ipairs(abilities.summon) do
            summons[#summons + 1] = create_entry(summon.id, summon.times, mp)
        end
        rows.summon = split_in_rows(summons)
        local e_skills = {}
        for _, id in ipairs(abilities.e_skill) do e_skills[#e_skills + 1] = create_entry(id, nil, mp) end
        rows.e_skill = split_in_rows(e_skills)
        self.rows = rows
    end,

    --- Shows the grid of a kind of abilities, with the cursor out of it.
    --
    -- @param position Position of the option (1 magic, 2 summon, 3 enemy skill).
    show_category = function(self, position)
        self.kind = KINDS[position]
        self.row, self.column, self.first_row = 1, 1, 1
        self:populate_list()
        self:update_scroll()
        ui_manager:get_widget(ROOT .. "Help.Help"):set_text("")
    end,

    --- Shows the abilities of the rows that are visible.
    populate_list = function(self)
        local rows = self.rows[self.kind]
        for visible_row = 1, ROWS do
            local row = rows[self.first_row + visible_row - 1]
            for column = 1, COLUMNS do
                local widget = ui_manager:get_widget(
                    ROOT .. "List.R" .. tostring(visible_row) .. "C" .. tostring(column))
                local entry = row ~= nil and row[column] or nil
                if entry == nil then
                    widget:set_visible(false)
                else
                    -- The colour has to be set before the text, as it applies to it when it's set.
                    local colour = entry.usable and COLOUR_USABLE or COLOUR_UNUSABLE
                    widget:set_colour(colour[1], colour[2], colour[3])
                    widget:set_text(entry.name)
                    widget:set_visible(true)
                end
            end
        end
    end,

    --- Moves the scroll bar of the list. It is only shown if the grid doesn't fit.
    update_scroll = function(self)
        local count = #self.rows[self.kind]
        local visible = count > ROWS
        ui_manager:get_widget(ROOT .. "List.ScrollBg"):set_visible(visible)
        local scroll = ui_manager:get_widget(ROOT .. "List.Scroll")
        scroll:set_visible(visible)
        if visible then
            local fraction = (self.first_row - 1) / (count - ROWS)
            -- Between 3 pixels from the top and 21 from the bottom of the list.
            scroll:set_y(fraction * 100, 3 - fraction * 24)
        end
    end,

    --- Moves the cursor over the grid.
    --
    -- Up and down keep the column, or go to the last ability if the row has fewer.
    --
    -- @param delta_row -1 to go up, 1 to go down.
    -- @param delta_column -1 to go left, 1 to go right.
    move_cursor = function(self, delta_row, delta_column)
        local rows = self.rows[self.kind]
        local row, column = self.row, self.column + delta_column
        if delta_column ~= 0 then
            -- Left and right stay in the row.
            if column < 1 or column > #rows[row] then return end
        else
            row = row + delta_row
            if row < 1 or row > #rows then
                UiContainer.play_sound("Error")
                return
            end
            column = math.min(column, #rows[row])
        end
        self.row, self.column = row, column
        UiContainer.play_sound("Cursor")
        -- Scroll if the cursor leaves the visible rows.
        local first_row = self.first_row
        if row < first_row then first_row = row
        elseif row > first_row + ROWS - 1 then first_row = row - ROWS + 1 end
        if first_row ~= self.first_row then
            self.first_row = first_row
            self:populate_list()
            self:update_scroll()
        end
        self:update_cursor()
    end,

    --- Puts the cursor on the highlighted ability and shows its description and cost.
    update_cursor = function(self)
        local visible_row = self.row - self.first_row + 1
        ui_manager:get_widget(ROOT .. "List.Cursor"):set_default_animation(
            "Position" .. tostring(visible_row) .. "-" .. tostring(self.column))
        local entry = self.rows[self.kind][self.row][self.column]
        ui_manager:get_widget(ROOT .. "Help.Help"):set_text(entry.description)
        ui_manager:get_widget(ROOT .. "Side.Cost.MpValue"):set_text(UiContainer.zero_pad_value(entry.mp, 3))
        local uses = ui_manager:get_widget(ROOT .. "Side.Cost.Uses")
        uses:set_visible(entry.uses ~= nil)
        if entry.uses ~= nil then
            local text = (entry.uses >= UNLIMITED) and "---" or UiContainer.zero_pad_value(entry.uses, 3)
            ui_manager:get_widget(ROOT .. "Side.Cost.Uses.Value"):set_text(text)
        end
    end,

    --- Switches between choosing what to see and moving over the grid.
    --
    -- @param browsing True when the cursor is in the grid.
    set_browsing = function(self, browsing)
        ui_manager:get_widget(ROOT .. "Side.Options"):set_visible(not browsing)
        ui_manager:get_widget(ROOT .. "Side.Cost"):set_visible(browsing)
        ui_manager:get_widget(ROOT .. "Character.Added"):set_visible(browsing)
        ui_manager:get_widget(ROOT .. "List.Cursor"):set_visible(browsing)
        if not browsing then
            ui_manager:get_widget(ROOT .. "Help.Help"):set_text("")
        end
    end,

    --- Opens the magic menu for UiContainer.current_character_id.
    show = function(self)
        self.char_id = UiContainer.current_character_id
        self:activate()
        self:build_rows()
        local character = Characters[self.char_id]
        UiContainer.populate_character_data(ROOT .. "Character", character)
        ui_manager:get_widget(ROOT .. "Character.Portrait"):set_image("characters/" .. tostring(self.char_id) .. ".png")
        self:set_browsing(false)
        self.category_cursor:set(1)
        ui_manager:get_widget("MagicMenu"):set_visible(true)
        return 0
    end,

    --- Closes the magic menu and goes back to the main menu.
    hide = function(self)
        ui_manager:get_widget("MagicMenu"):set_visible(false)
        self:deactivate()
        return 0
    end,
}
