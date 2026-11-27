if UiContainer == nil then UiContainer = {} end

local ROOT = "LimitMenu.Container."

--- Inside width of the limit bar.
local BAR_WIDTH = 54

--- Number of limit levels, and of techniques of each level.
local LEVELS = 4
local TECHNIQUES = 2

--- What the player is doing with the levels.
local MODE = { SET = 1, CHECK = 2 }

--- Colour of the levels with learned techniques, and of the others.
local COLOUR_ACTIVE = { 1.0, 1.0, 1.0 }
local COLOUR_INACTIVE = { 0.45, 0.45, 0.35 }

local function widget(path)
    return ui_manager:get_widget(ROOT .. path)
end

--- Is a technique of the character learned?
local function is_learned(character, level, technique)
    local learned = character.limit.learned[level]
    return learned ~= nil and learned[technique] == true
end

--- Gets the definition (name and description) of a technique, or nil if it isn't known.
local function get_technique(char_id, level, technique)
    local definition = Game.Characters[char_id]
    if definition == nil or definition.limits == nil or definition.limits[level] == nil then
        return nil
    end
    local entry = definition.limits[level].techniques[technique]
    if entry == nil then return nil end
    return Game.Attacks[entry.command]
end

--- The limit menu.
--
-- On top there are two choices: Set, to choose the limit level of the character, and Check, to see
-- the techniques learned at each level. Both let the player move over the four levels.
--  * Set: choosing a level with learned techniques asks for confirmation. Confirming assigns it
--    and empties the limit bar.
--  * Check: choosing a level with learned techniques lets the player move between its techniques,
--    showing their description.
-- Choosing a level without learned techniques plays the error sound.
UiContainer.LimitMenu = UiContainer.Menu.new{

    id = "limit",
    parent = "main",

    --- ID of the character being shown.
    char_id = -1,

    --- Set or check (MODE).
    mode = MODE.SET,

    --- Cursor over Set and Check.
    mode_cursor = UiContainer.Cursor.new{
        widget = ROOT .. "Character.Modes.Cursor",
        total = 2,
    },

    --- Cursor over the four levels, in a 2x2 grid.
    level_cursor = UiContainer.Cursor.new{
        widget = ROOT .. "Body.LevelCursor",
        total = 4,
        sound = false,
    },

    --- Cursor over the techniques of the chosen level.
    tech_cursor = UiContainer.Cursor.new{
        total = 2,
        on_move = function(position)
            UiContainer.LimitMenu:update_technique(position)
        end,
    },

    --- Cursor over Yes and No in the confirmation window.
    confirm_cursor = UiContainer.Cursor.new{
        widget = ROOT .. "Confirm.Cursor",
        total = 2,
    },

    states = {
        --- Choosing Set or Check.
        [""] = {
            cursor = function(self) return self.mode_cursor end,
            axis = "horizontal",
            confirm = function(self, cursor)
                self.mode = cursor:index()
                self.level_cursor:set(1)
                widget("Body.LevelCursor"):set_visible(true)
                self:set_state("level")
            end,
            cancel = function(self)
                script:request_end_sync(Script.UI, "LimitMenu", "hide", 0)
            end,
        },

        --- Choosing a level.
        level = {
            buttons = UiContainer.grid_buttons("level_cursor", 2),
            confirm = function(self)
                local level = self.level_cursor:index()
                if not self:has_learned(level) then return false end
                if self.mode == MODE.SET then
                    self.confirm_cursor:set(2) -- No.
                    widget("Confirm"):set_visible(true)
                    self:set_state("confirm")
                else
                    widget("Body.LevelCursor"):set_visible(false)
                    widget("Body.TechCursor"):set_visible(true)
                    self.tech_cursor:set(1)
                    self:set_state("technique")
                end
            end,
            cancel = function(self)
                widget("Body.LevelCursor"):set_visible(false)
                self:set_state("")
            end,
        },

        --- Moving between the techniques of a level.
        technique = {
            cursor = function(self) return self.tech_cursor end,
            axis = "vertical",
            cancel = function(self)
                widget("Body.TechCursor"):set_visible(false)
                widget("Help.Help"):set_text("")
                widget("Body.LevelCursor"):set_visible(true)
                self:set_state("level")
            end,
        },

        --- Confirming the change of level.
        confirm = {
            cursor = function(self) return self.confirm_cursor end,
            axis = "vertical",
            confirm = function(self, cursor)
                if cursor:index() == 1 then
                    self:set_level(self.level_cursor:index())
                end
                widget("Confirm"):set_visible(false)
                self:set_state("level")
            end,
            cancel = function(self)
                widget("Confirm"):set_visible(false)
                self:set_state("level")
            end,
        },
    },

    --- Does the character know any technique of a level?
    has_learned = function(self, level)
        local character = Characters[self.char_id]
        for technique = 1, TECHNIQUES do
            if is_learned(character, level, technique) then return true end
        end
        return false
    end,

    --- Assigns a limit level to the character and empties its limit bar.
    set_level = function(self, level)
        local character = Characters[self.char_id]
        character.limit.current = level
        character.limit.bar = 0
        self:populate_limit(character)
    end,

    --- Moves the technique cursor and shows the description of the technique.
    update_technique = function(self, position)
        local level = self.level_cursor:index()
        widget("Body.TechCursor"):set_default_animation("Position" .. tostring((level - 1) * 2 + position))
        local description = ""
        if is_learned(Characters[self.char_id], level, position) then
            local attack = get_technique(self.char_id, level, position)
            if attack ~= nil then description = attack.description end
        end
        widget("Help.Help"):set_text(description)
    end,

    --- Shows the limit level and bar of the character.
    populate_limit = function(self, character)
        widget("Character.LimitLevel"):set_text(tostring(character.limit.current))
        UiContainer.set_limit_bar(ROOT .. "Character.LimitLinePart", character, BAR_WIDTH)
    end,

    --- Shows the names of the levels and of their learned techniques.
    populate = function(self)
        local character = Characters[self.char_id]
        UiContainer.populate_character_data(ROOT .. "Character", character)
        widget("Character.Portrait"):set_image("characters/" .. tostring(self.char_id) .. ".png")
        self:populate_limit(character)

        for level = 1, LEVELS do
            local learned = self:has_learned(level)
            local colour = learned and COLOUR_ACTIVE or COLOUR_INACTIVE
            local label = widget("Body.Level" .. tostring(level) .. ".Lbl")
            label:set_colour(colour[1], colour[2], colour[3])
            label:set_text("LEVEL " .. tostring(level))
            for technique = 1, TECHNIQUES do
                local name = ""
                if is_learned(character, level, technique) then
                    local attack = get_technique(self.char_id, level, technique)
                    if attack ~= nil then name = attack.name end
                end
                widget("Body.Level" .. tostring(level) .. ".Tech" .. tostring(technique)):set_text(name)
            end
        end
        widget("Help.Help"):set_text("")
    end,

    --- Opens the limit menu for UiContainer.current_character_id.
    show = function(self)
        self.char_id = UiContainer.current_character_id
        self:activate()
        self:populate()
        self.mode_cursor:set(1)
        widget("Body.LevelCursor"):set_visible(false)
        widget("Body.TechCursor"):set_visible(false)
        widget("Confirm"):set_visible(false)
        ui_manager:get_widget("LimitMenu"):set_visible(true)
        return 0
    end,

    --- Hides the limit menu and goes back to the main menu.
    hide = function(self)
        ui_manager:get_widget("LimitMenu"):set_visible(false)
        self:deactivate()
        return 0
    end,
}
