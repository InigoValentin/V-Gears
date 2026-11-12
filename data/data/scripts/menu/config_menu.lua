if UiContainer == nil then UiContainer = {} end

local ROOT = "ConfigMenu.Container."

--- Range of the values of the red, green and blue sliders of the window colour.
local COLOUR_MIN = 0
local COLOUR_MAX = 255

--- Step of the colour sliders: left and right, and L1 and R1 (big step).
local COLOUR_STEP = 1
local COLOUR_BIG_STEP = 16

--- Position and width in pixels of the colour sliders, relative to the colour window.
local SLIDER_X = 70
local SLIDER_WIDTH = 92

--- Corners of the window, in the order they are shown (2x2 grid), and the corner of the colours
-- table (top-left, top-right, bottom-right, bottom-left) each one is.
local CORNER_COLOUR = { 1, 2, 4, 3 }

--- Actions the player can choose the keys of. The first entry is the button that restores the
-- default keys.
--
--  * name: Name of the row widget (with the prefix "Row", because the window prototype has children
--    called Left and Right) and of the texts.
--  * events: Game events (see bind_game_event in config.cfg) that use the keys of the action. The
--    default keys are the ones config.cfg gives to the first one (see get_default_keys).
local CONTROLS = {
    { name = "Default" },
    { name = "Up", events = { "Up", "walk_up", "message_up" } },
    { name = "Down", events = { "Down", "walk_down", "message_down" } },
    { name = "Left", events = { "Left", "walk_left" } },
    { name = "Right", events = { "Right", "walk_right" } },
    { name = "Ok", events = { "CONFIRM" } },
    { name = "Cancel", events = { "CANCEL" } },
    { name = "Menu", events = { "MENU" } },
    { name = "Extra", events = { "EXTRA" } },
    { name = "Select", events = { "SELECT" } },
    { name = "Start", events = { "START" } },
    { name = "L1", events = { "L1" } },
    { name = "R1", events = { "R1" } },
    { name = "Run", events = { "run" } },
}

--- File where the keys are saved. The game runs it after config.cfg when it starts.
local CONTROLS_FILE = "./controls.cfg"

--- Key that cancels the choice of a key. It can't be assigned.
local CAPTURE_CANCEL_KEY = "Escape"

--- Maximum value of the volume sliders (0 is muted).
local VOLUME_MAX = 4

--- Range of the sliders. 0 is the left end (fast), SLIDER_MAX the right end (slow).
local SLIDER_MAX = 10

--- Types of row.
local KIND = {
    ACTION = "action",   -- Opens something when OK is pressed.
    CHOICE = "choice",   -- Left/right choose one of several options.
    SLIDER = "slider",   -- Left/right move a slider.
    ORDER = "order",     -- Left/right choose one of the orders of the magic menu.
}

--- The rows of the menu, in the order they are shown.
--
--  * name: Name of the row widget.
--  * kind: Type of row (KIND). Sound and controller open their own window, like window color.
--  * key: Field of MenuSettings.config where the value is stored.
--  * count: Number of options (choices and orders).
local ROWS = {
    { name = "WindowColor", kind = KIND.ACTION },
    { name = "Sound", kind = KIND.ACTION },
    { name = "Controller", kind = KIND.ACTION },
    { name = "CursorMode", kind = KIND.CHOICE, key = "cursor", count = 2 },
    { name = "Atb", kind = KIND.CHOICE, key = "atb", count = 3 },
    { name = "BattleSpeed", kind = KIND.SLIDER, key = "battle_speed" },
    { name = "BattleMessage", kind = KIND.SLIDER, key = "battle_message" },
    { name = "FieldMessage", kind = KIND.SLIDER, key = "field_message" },
    { name = "Movement", kind = KIND.CHOICE, key = "movement", count = 2 },
    { name = "CameraAngle", kind = KIND.CHOICE, key = "camera", count = 2 },
    { name = "MagicOrder", kind = KIND.ORDER, key = "magic_order", count = 6 },
}

--- Values of the options. Choices and orders start at 1, sliders at 0.
--
-- TODO: These are placeholders. They don't change the game yet. Each option must be connected to
-- its setting as it is implemented.
-- TODO: Apply the volumes (apply_volumes), the movement (apply_movement) and the window colours
-- when the settings are loaded.
-- TODO: Save and load all of them with the savemap (see the TODO about the settings in save.lua and
-- Savemap.cpp).
MenuSettings.config = MenuSettings.config or {
    cursor = 1,         -- 1: Initial, 2: Memory.
    atb = 2,            -- 1: Active, 2: Recommended, 3: Wait.
    battle_speed = 5,
    battle_message = 5,
    field_message = 5,
    movement = 1,       -- 1: Walk, 2: Run.
    camera = 1,         -- 1: Auto, 2: Fix.
    magic_order = 1,    -- Permutations of restore, attack and indirect.

    music_volume = 4,   -- 0-4.
    sound_volume = 4,   -- 0-4. Sound effects.

    --- Colours of the windows (red, green, blue, 0-255) at the top-left, top-right, bottom-right
    -- and bottom-left corners. They are the ones the windows have in the XML.
    window_colours = {
        { 0, 0, 178 },
        { 0, 0, 127 },
        { 0, 0, 32 },
        { 0, 0, 79 },
    },
}

--- Gets the colours of the windows as the 12 numbers (0-1) the engine takes.
local function window_colours_as_floats()
    local values = {}
    for _, colour in ipairs(MenuSettings.config.window_colours) do
        for channel = 1, 3 do
            values[#values + 1] = colour[channel] / 255
        end
    end
    return values
end

--- Splits a list of keys separated by commas.
local function split_keys(text)
    local keys = {}
    for key in string.gmatch(text, "[^,]+") do keys[#keys + 1] = key end
    return keys
end

--- Gets the keys of an action of CONTROLS.
local function get_keys(action)
    return split_keys(input_manager:get_game_event_keys(action.events[1]))
end

--- Gets the default keys of an action of CONTROLS: the ones of config.cfg.
local function get_default_keys(action)
    return split_keys(input_manager:get_default_game_event_keys(action.events[1]))
end

--- Sets the keys of an action of CONTROLS, in all its game events.
local function set_keys(action, keys)
    local text = table.concat(keys, ",")
    for _, event in ipairs(action.events) do
        input_manager:set_game_event_keys(event, text)
    end
end

--- Handlers of the buttons that change the value of the colour under the cursor.
local function colour_buttons()
    local buttons = {}
    buttons[Config.CONTROLS.LEFT] = function(self, event) self:change_colour(-COLOUR_STEP) end
    buttons[Config.CONTROLS.RIGHT] = function(self, event) self:change_colour(COLOUR_STEP) end
    buttons[Config.ACTIONS.L1] = function(self, event) self:change_colour(-COLOUR_BIG_STEP) end
    buttons[Config.ACTIONS.R1] = function(self, event) self:change_colour(COLOUR_BIG_STEP) end
    return buttons
end

--- Handlers of Left and Right, which change the value of the row under the cursor.
local function value_buttons()
    local buttons = {}
    buttons[Config.CONTROLS.LEFT] = function(self, event) self:change_value(-1) end
    buttons[Config.CONTROLS.RIGHT] = function(self, event) self:change_value(1) end
    return buttons
end

--- The config menu.
--
-- Up and down choose a row, and left and right change its value right away. Window color, sound
-- and controller open their own window with OK.
--
-- Sound: opens a window with two sliders, one for the volume of the music and another for the
-- sound effects (0-4). The volume changes as soon as the slider moves.
--
-- Controller: opens a window with the keys of every action (the directions, OK, cancel...). OK on
-- an action asks for the key to assign to it. If the key was used by another action, they swap
-- their keys. The first entry restores the default keys. The changes are kept apart and are only
-- applied (and saved in controls.cfg, which the game reads when it starts) when the window is
-- closed, if they are valid (see controls_are_valid), so the player can't be left without a way to
-- control the game.
--
-- Window color: the cursor goes to the preview of the window to the right, where one of the four
-- corners is chosen. Each corner opens a window with three sliders (red, green and blue). The
-- preview shows the changes, and the colours are applied to all the windows when the cursor goes
-- back to the list.
--
-- TODO: All the options are placeholders for now.
UiContainer.ConfigMenu = UiContainer.Menu.new{

    id = "config",
    parent = "main",

    --- Cursor over the rows.
    cursor = UiContainer.Cursor.new{
        widget = ROOT .. "Cursor",
        total = #ROWS,
        on_move = function(position)
            UiContainer.ConfigMenu:update_help(position)
        end,
    },

    --- Cursor over the corners of the window preview (2x2 grid).
    corner_cursor = UiContainer.Cursor.new{
        widget = ROOT .. "CornerCursor",
        total = 4,
        sound = false,
    },

    --- Cursor over the actions of the controller window.
    controller_cursor = UiContainer.Cursor.new{
        widget = ROOT .. "ControllerWindow.Cursor",
        total = #CONTROLS,
    },

    --- Action (entry of CONTROLS) that is waiting for a key.
    assigning = nil,

    --- Keys the player has chosen in the controller window, not applied yet. Lists of key names by
    -- position in CONTROLS.
    pending_keys = {},

    --- Cursor over the music and sound effects sliders.
    sound_cursor = UiContainer.Cursor.new{
        widget = ROOT .. "SoundWindow.Cursor",
        total = 2,
    },

    --- Cursor over the red, green and blue sliders.
    channel_cursor = UiContainer.Cursor.new{
        widget = ROOT .. "ColourWindow.Cursor",
        total = 3,
    },

    states = {
        [""] = {
            cursor = function(self) return self.cursor end,
            axis = "vertical",
            buttons = value_buttons(),
            confirm = function(self, cursor)
                local row = ROWS[cursor:index()]
                if row.name == "WindowColor" then
                    self.corner_cursor:set(1)
                    ui_manager:get_widget(ROOT .. "CornerCursor"):set_visible(true)
                    self:set_state("corner")
                    return
                elseif row.name == "Sound" then
                    self.sound_cursor:set(1)
                    self:refresh_sound_window()
                    ui_manager:get_widget(ROOT .. "SoundWindow"):set_visible(true)
                    self:set_state("sound")
                    return
                elseif row.name == "Controller" then
                    self.controller_cursor:set(1)
                    self:load_pending_keys()
                    self:refresh_controller_window()
                    ui_manager:get_widget(ROOT .. "ControllerWindow"):set_visible(true)
                    self:set_state("controller")
                    return
                elseif row.kind == KIND.ACTION then
                    print("TODO: Open the window of " .. row.name)
                end
                -- The other rows change with left and right.
                return false
            end,
            cancel = function(self)
                script:request_end_sync(Script.UI, "ConfigMenu", "hide", 0)
            end,
        },

        --- Choosing the action to change the key of.
        controller = {
            cursor = function(self) return self.controller_cursor end,
            axis = "vertical",
            confirm = function(self, cursor)
                local index = cursor:index()
                if index == 1 then
                    self:reset_controls()
                    self:refresh_controller_window()
                    return
                end
                -- The next key pressed is the new key of the action.
                self.assigning = index
                input_manager:capture_next_key()
                self:set_cursor_blinking(true)
                self:update_help(#ROWS + 1)
                self:set_state("assign")
            end,
            cancel = function(self)
                -- The keys are only applied if they are valid. If not, the window stays open.
                if not self:controls_are_valid() then return false end
                ui_manager:get_widget(ROOT .. "ControllerWindow"):set_visible(false)
                self:apply_controls()
                self:set_state("")
            end,
        },

        --- Waiting for the key to assign to an action.
        assign = {
            capture = function(self, key)
                if key == "UNASSIGNED" then
                    -- A key the game doesn't know: ask again.
                    UiContainer.play_sound("Error")
                    input_manager:capture_next_key()
                    return
                elseif key == CAPTURE_CANCEL_KEY then
                    UiContainer.play_sound("Back")
                else
                    self:assign_key(self.assigning, key)
                    UiContainer.play_sound("Cursor")
                    self:refresh_controller_window()
                end
                self:set_cursor_blinking(false)
                self:update_help(self.cursor:index())
                self:set_state("controller")
            end,
        },

        --- Changing the volume of the music and of the sound effects.
        sound = {
            cursor = function(self) return self.sound_cursor end,
            axis = "vertical",
            buttons = {
                [Config.CONTROLS.LEFT] = function(self, event) self:change_volume(-1) end,
                [Config.CONTROLS.RIGHT] = function(self, event) self:change_volume(1) end,
            },
            confirm = function(self)
                ui_manager:get_widget(ROOT .. "SoundWindow"):set_visible(false)
                self:set_state("")
            end,
            cancel = function(self)
                ui_manager:get_widget(ROOT .. "SoundWindow"):set_visible(false)
                self:set_state("")
            end,
        },

        --- Choosing the corner of the window to change.
        corner = {
            buttons = UiContainer.grid_buttons("corner_cursor", 2),
            confirm = function(self)
                self.channel_cursor:set(1)
                self:refresh_colour_window()
                ui_manager:get_widget(ROOT .. "ColourWindow"):set_visible(true)
                self:set_state("colour")
            end,
            cancel = function(self)
                -- Back to the list: the colours are applied now.
                ui_manager:get_widget(ROOT .. "CornerCursor"):set_visible(false)
                self:apply_window_colours()
                self:set_state("")
            end,
        },

        --- Changing the red, green and blue of a corner.
        colour = {
            cursor = function(self) return self.channel_cursor end,
            axis = "vertical",
            buttons = colour_buttons(),
            confirm = function(self)
                ui_manager:get_widget(ROOT .. "ColourWindow"):set_visible(false)
                self:set_state("corner")
            end,
            cancel = function(self)
                ui_manager:get_widget(ROOT .. "ColourWindow"):set_visible(false)
                self:set_state("corner")
            end,
        },
    },

    --- Makes the cursor of the controller window blink (while a key is asked for), or stop.
    --
    -- @param blinking True to blink, false to go back to the fixed cursor.
    set_cursor_blinking = function(self, blinking)
        local cursor = ui_manager:get_widget(ROOT .. "ControllerWindow.Cursor")
        local animation = (blinking and "Blink" or "Position") .. tostring(self.controller_cursor:index())
        cursor:set_default_animation(animation)
        cursor:play_animation(animation)
        -- The blink leaves it transparent when it stops at the wrong moment.
        if not blinking then cursor:set_alpha(1.0) end
    end,

    --- Shows the keys of every action in the controller window.
    refresh_controller_window = function(self)
        for index = 2, #CONTROLS do
            local widget = ui_manager:get_widget(ROOT .. "ControllerWindow.Row" .. CONTROLS[index].name .. ".Keys")
            widget:set_colour(1.0, 1.0, 1.0)
            widget:set_text(table.concat(self.pending_keys[index], ", "))
        end
    end,

    --- Copies the keys the actions have now to the pending keys.
    load_pending_keys = function(self)
        self.pending_keys = {}
        for index = 2, #CONTROLS do
            self.pending_keys[index] = get_keys(CONTROLS[index])
        end
    end,

    --- Checks that the pending keys leave the game controllable.
    --
    -- Every action must have at least one key, and no key can be used by two actions.
    --
    -- @return True if they can be applied.
    controls_are_valid = function(self)
        local used = {}
        for index = 2, #CONTROLS do
            local keys = self.pending_keys[index]
            if keys == nil or #keys == 0 then return false end
            for _, key in ipairs(keys) do
                if used[key] then return false end
                used[key] = true
            end
        end
        return true
    end,

    --- Applies the pending keys to the game and saves them.
    apply_controls = function(self)
        for index = 2, #CONTROLS do
            set_keys(CONTROLS[index], self.pending_keys[index])
        end
        self:save_controls()
    end,

    --- Gives a key to an action, in the pending keys.
    --
    -- The action keeps only that key. If another action had it, it loses it and, if it was its
    -- only key, it takes the keys the action had before (they swap).
    --
    -- @param index Position of the action in CONTROLS.
    -- @param key Name of the key.
    assign_key = function(self, index, key)
        local previous = self.pending_keys[index]
        for other_index = 2, #CONTROLS do
            if other_index ~= index then
                local keys = self.pending_keys[other_index]
                for position, existing in ipairs(keys) do
                    if existing == key then
                        table.remove(keys, position)
                        if #keys == 0 then
                            keys = (#previous > 0) and previous or get_default_keys(CONTROLS[other_index])
                        end
                        self.pending_keys[other_index] = keys
                        break
                    end
                end
            end
        end
        self.pending_keys[index] = { key }
    end,

    --- Puts the default keys of all the actions in the pending keys.
    reset_controls = function(self)
        for index = 2, #CONTROLS do
            self.pending_keys[index] = get_default_keys(CONTROLS[index])
        end
    end,

    --- Saves the keys in the controls file, which the game runs when it starts.
    save_controls = function(self)
        local file = io.open(CONTROLS_FILE, "w")
        if file == nil then
            print("Can't write the controller settings in " .. CONTROLS_FILE)
            return
        end
        for index = 2, #CONTROLS do
            local action = CONTROLS[index]
            local keys = table.concat(self.pending_keys[index], ",")
            for _, event in ipairs(action.events) do
                file:write('set_game_event_keys "' .. event .. '" "' .. keys .. '"\n')
            end
        end
        file:close()
    end,

    --- Changes the volume (music or sound effects, the one under the cursor).
    --
    -- @param delta -1 for left, 1 for right.
    change_volume = function(self, delta)
        local key = (self.sound_cursor:index() == 1) and "music_volume" or "sound_volume"
        local values = MenuSettings.config
        local value = math.max(0, math.min(VOLUME_MAX, values[key] + delta))
        if value == values[key] then return end
        values[key] = value
        self:apply_volumes()
        self:refresh_sound_window()
        -- Played after applying the volume, so the sound effect is heard with the new volume.
        UiContainer.play_sound("Cursor")
    end,

    --- Applies the movement setting (walk or run by default) to the player.
    apply_movement = function(self)
        entity_manager:set_player_default_run(MenuSettings.config.movement == 2)
    end,

    --- Applies the volumes to the audio.
    apply_volumes = function(self)
        audio_manager:set_music_volume(MenuSettings.config.music_volume / VOLUME_MAX)
        audio_manager:set_sound_volume(MenuSettings.config.sound_volume / VOLUME_MAX)
    end,

    --- Shows the position of the volume sliders.
    refresh_sound_window = function(self)
        local values = MenuSettings.config
        ui_manager:get_widget(ROOT .. "SoundWindow.Music.Handle")
            :set_default_animation("Position" .. tostring(values.music_volume))
        ui_manager:get_widget(ROOT .. "SoundWindow.Effects.Handle")
            :set_default_animation("Position" .. tostring(values.sound_volume))
    end,

    --- Colour (entry of MenuSettings.config.window_colours) of the corner under the cursor.
    current_colour = function(self)
        return MenuSettings.config.window_colours[CORNER_COLOUR[self.corner_cursor:index()]]
    end,

    --- Changes the red, green or blue (the one under the cursor) of the corner.
    --
    -- @param delta Amount added to the value.
    change_colour = function(self, delta)
        local colour = self:current_colour()
        local channel = self.channel_cursor:index()
        local value = math.max(COLOUR_MIN, math.min(COLOUR_MAX, colour[channel] + delta))
        if value == colour[channel] then return end
        colour[channel] = value
        self:refresh_colour_window()
        self:refresh_preview()
    end,

    --- Shows the sliders of the corner under the cursor.
    refresh_colour_window = function(self)
        local colour = self:current_colour()
        local names = { "Red", "Green", "Blue" }
        for channel = 1, 3 do
            local path = ROOT .. "ColourWindow." .. names[channel]
            local value = colour[channel]
            ui_manager:get_widget(path .. ".Value"):set_text(UiContainer.pad_value(value, 3))
            local offset = math.floor((value - COLOUR_MIN) / (COLOUR_MAX - COLOUR_MIN) * SLIDER_WIDTH)
            ui_manager:get_widget(path .. ".Handle"):set_x(0, SLIDER_X + offset)
        end
    end,

    --- Shows the chosen colours in the preview of the window.
    refresh_preview = function(self)
        local v = window_colours_as_floats()
        ui_manager:get_widget(ROOT .. "Body.WindowColor.Preview.Center"):set_colours(
            v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12])
    end,

    --- Applies the chosen colours to all the windows.
    apply_window_colours = function(self)
        local v = window_colours_as_floats()
        ui_manager:set_window_colours(
            v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12])
    end,

    --- Shows only the help text of a row.
    update_help = function(self, position)
        -- There is one more help text than rows: the one shown while a key is asked for.
        for index = 1, #ROWS + 1 do
            ui_manager:get_widget(ROOT .. "Help.H" .. tostring(index)):set_visible(index == position)
        end
    end,

    --- Changes the value of the row under the cursor.
    --
    -- @param delta -1 for left, 1 for right.
    change_value = function(self, delta)
        local row = ROWS[self.cursor:index()]
        if row.kind == KIND.ACTION then return end
        local values = MenuSettings.config
        local min = (row.kind == KIND.SLIDER) and 0 or 1
        local max = (row.kind == KIND.SLIDER) and SLIDER_MAX or row.count
        local value = values[row.key] + delta
        if value < min or value > max then return end
        values[row.key] = value
        if row.key == "movement" then self:apply_movement() end
        UiContainer.play_sound("Cursor")
        self:refresh_row(row)
    end,

    --- Shows the value of a row.
    refresh_row = function(self, row)
        local path = ROOT .. "Body." .. row.name
        local value = MenuSettings.config[row.key]
        if row.kind == KIND.CHOICE then
            -- Every option has a white copy (shown when selected) and a gray one (shown otherwise).
            -- The gray one uses its own text ("...Off") in the texts file, because the colour
            -- attribute of the XML is not applied to texts that come from the texts file.
            for option = 1, row.count do
                local selected = (option == value)
                ui_manager:get_widget(path .. ".Opt" .. tostring(option) .. "On"):set_visible(selected)
                ui_manager:get_widget(path .. ".Opt" .. tostring(option) .. "Off"):set_visible(not selected)
            end
        elseif row.kind == KIND.SLIDER then
            ui_manager:get_widget(path .. ".Handle"):set_default_animation("Position" .. tostring(value))
        elseif row.kind == KIND.ORDER then
            ui_manager:get_widget(path .. ".Number"):set_text(tostring(value))
            for order = 1, row.count do
                ui_manager:get_widget(path .. ".Order" .. tostring(order)):set_visible(order == value)
            end
        end
    end,

    --- Opens the config menu.
    show = function(self)
        self:activate()
        for _, row in ipairs(ROWS) do
            if row.kind ~= KIND.ACTION then self:refresh_row(row) end
        end
        self.cursor:set(1)
        ui_manager:get_widget(ROOT .. "CornerCursor"):set_visible(false)
        ui_manager:get_widget(ROOT .. "ColourWindow"):set_visible(false)
        ui_manager:get_widget(ROOT .. "SoundWindow"):set_visible(false)
        ui_manager:get_widget(ROOT .. "ControllerWindow"):set_visible(false)
        self:refresh_preview()
        ui_manager:get_widget("ConfigMenu"):set_visible(true)
        return 0
    end,

    --- Closes the config menu and goes back to the main menu.
    hide = function(self)
        ui_manager:get_widget("ConfigMenu"):set_visible(false)
        self:deactivate()
        return 0
    end,
}
