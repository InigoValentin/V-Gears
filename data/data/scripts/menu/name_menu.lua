if UiContainer == nil then UiContainer = {} end

--- The name menu.
UiContainer.NameMenu = {

    --- Indicates if a chocobo is being renamed instead of a character.
    chocobo = false,

    --- ID of the character or chocobo being renamed.
    id = -1,

    --- Default name.
    default_name = "",

    --- Currently input-ed name.
    name = "",

    --- Max number of characters allowed in a name.
    max_characters = 12,

    --- Current text cursor
    text_cursor = 1,

    --- Accepted characters in a name.
    letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ",

    --- Current options cursor position.
    options_position = 1,

    --- Total options cursor position.
    options_position_total = 3,

    --- Current confirm dialog cursor position.
    confirm_position = 1,

    --- Total confirm dialog cursor position.
    confirm_position_total = 2,

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
        if UiContainer.current_menu == "name" then
            if UiContainer.current_submenu == "confirm" then
                if button == "Right" then
                    audio_manager:play_sound("Cursor")
                    self.confirm_position = self.confirm_position + 1
                    if self.confirm_position > self.confirm_position_total then
                        self.confirm_position = 1
                    end
                    ui_manager:get_widget("NameMenu.Container.Confirm.Cursor"):set_default_animation("Position" .. self.confirm_position)
                elseif button == "Left" then
                    audio_manager:play_sound("Cursor")
                    self.confirm_position = self.confirm_position - 1
                    if self.confirm_position < 1 then
                        self.confirm_position = self.confirm_position_total
                    end
                    ui_manager:get_widget("NameMenu.Container.Confirm.Cursor"):set_default_animation("Position" .. self.confirm_position)
                elseif button == "Enter" then
                    audio_manager:play_sound("Cursor")
                    if self.confirm_position == 2 then -- Back
                        UiContainer.current_submenu = ""
                        ui_manager:get_widget("NameMenu.Container.Confirm"):set_visible(false)
                    elseif self.confirm_position == 1 then
                        ui_manager:get_widget("NameMenu.Container.Confirm"):set_visible(false)
                        self.apply_name(self);
                    end
                end
            elseif UiContainer.current_submenu == "" then
                -- Before handling buttons, get keycode, for letters and space.
                local character_index
                if button == "Space" then
                    character_index = string.find(self.letters, " ")
                else
                    character_index = string.find(self.letters, button)
                end

                -- Handle buttons.
                if button == "Down" then
                    audio_manager:play_sound("Cursor")
                    self.options_position = self.options_position + 1
                    if self.options_position > self.options_position_total then
                        self.options_position = 1
                    end
                    ui_manager:get_widget("NameMenu.Container.Options.Cursor"):set_default_animation("Position" .. self.options_position)
                elseif button == "Up" then
                    audio_manager:play_sound("Cursor")
                    self.options_position = self.options_position - 1
                    if self.options_position < 1 then
                        self.options_position = self.options_position_total
                    end
                    ui_manager:get_widget("NameMenu.Container.Options.Cursor"):set_default_animation("Position" .. self.options_position)
                elseif button == "Left" then
                    if self.text_cursor > 1 then
                        self.text_cursor = self.text_cursor - 1
                        self.draw_cursor(self)
                        audio_manager:play_sound("Cursor")
                    else
                        audio_manager:play_sound("Error")
                    end
                elseif button == "Right" then
                    if self.text_cursor < self.max_characters then
                        audio_manager:play_sound("Cursor")
                        self.text_cursor = self.text_cursor + 1
                        self.draw_cursor(self)
                    else
                        audio_manager:play_sound("Error")
                    end
                elseif button == "Enter" then
                    if self.options_position == 1 then -- Clear name.
                        audio_manager:play_sound("Cursor")
                        self.name = ""
                        self.text_cursor = 1
                        self.populate_name(self)
                        self.draw_cursor(self)
                    elseif self.options_position == 2 then -- Default name.
                        audio_manager:play_sound("Cursor")
                        self.name = self.default_name
                        self.text_cursor = math.min(#(self.name) + 1, self.max_characters)
                        self.populate_name(self)
                        self.draw_cursor(self)
                    elseif self.options_position == 3 then -- Show confirm dialog.
                        self.format_name(self)
                        self.populate_name(self)
                        self.draw_cursor(self)
                        if #(self.name) > 0 then
                            audio_manager:play_sound("Cursor")
                            UiContainer.current_submenu = "confirm"
                            local name_display = self.name -- Pad with spaces, to display only.
                            while #(name_display) < self.max_characters do
                                name_display = " " .. name_display
                            end
                            ui_manager:get_widget("NameMenu.Container.Confirm.Name"):set_text(name_display)
                            ui_manager:get_widget("NameMenu.Container.Confirm"):set_visible(true)
                            self.confirm_position = 1
                            ui_manager:get_widget("NameMenu.Container.Options.Cursor"):set_default_animation("Position" .. self.options_position)
                        else
                            audio_manager:play_sound("Error")
                        end
                    end
                -- TODO: Implement backspace, but how?
                -- Letters and spaces
                elseif character_index ~= nil then
                    audio_manager:play_sound("Cursor")
                    self.set_char(self, character_index)
                    self.text_cursor = math.min(self.text_cursor + 1, self.max_characters)
                    self.populate_name(self)
                    self.draw_cursor(self)
                end
            end
        end
        return 0
    end,

    --- Opens the name menu.
    --
    -- Populates and updates displayed data before opening. It also pauses the game. Shows a fade
    -- effect.
    show = function(self)
        entity_manager:set_paused(true)
        FFVII.MenuSettings.pause_available = false
        script:request_end_sync(Script.UI, "Fade", "fade_out", 0)
        ui_manager:get_widget("NameMenu"):set_visible(true)
        UiContainer.current_menu = "name"
        UiContainer.current_submenu = ""
        ui_manager:get_widget("NameMenu.Container.Character.Portrait"):set_image("images/characters/" .. tostring(self.id) .. ".png")
        if (string.lower(Characters[self.id].name) == "ex-soldier") then
            -- HACK: Hack for Ex-Soldier default. Can this be extracted from menu.lgp?
            self.default_name = "Cloud"
            self.name = "Cloud"
        else
            self.default_name = Characters[self.id].name
            self.name = Characters[self.id].name
        end
        self.text_cursor = math.min(#(self.name) + 1, self.max_characters)
        self.populate_name(self)
        self.draw_cursor(self)
        script:request_end_sync(Script.UI, "Fade", "fade_in", 0)
        return 0;
    end,

    --- Redraws the text cursor, indicating the current character.
    draw_cursor = function(self)
        for i = 1, self.max_characters do
            if self.text_cursor == i then
                ui_manager:get_widget("NameMenu.Container.Character.Name.Score" .. tostring(i)):set_height(1.8)
            else
                ui_manager:get_widget("NameMenu.Container.Character.Name.Score" .. tostring(i)):set_height(0.3)
            end
        end
    end,

    --- Writes the name characters in the name input line.
    populate_name = function(self)
        for i = 1, self.max_characters do
            if #(self.name) < i then
                ui_manager:get_widget("NameMenu.Container.Character.Name.Char" .. tostring(i)):set_text("")
            else
                ui_manager:get_widget("NameMenu.Container.Character.Name.Char" .. tostring(i)):set_text(self.name:sub(i, i))

            end
        end
    end,

    --- Sets a character in the current cursor position.
    set_char = function(self, character_index)
        local temp_name = ""
        for i = 1, self.max_characters do
            if i == self.text_cursor then
                -- First position or after space: Use upper case.
                if i == 1 or temp_name:sub(i - 1, i - 1) == " " then
                    temp_name = temp_name .. self.letters:sub(character_index, character_index)
                else
                    temp_name = temp_name .. string.lower(self.letters:sub(character_index, character_index))
                end
            elseif #(self.name) >= i then
                if i == 1 or temp_name:sub(i - 1, i - 1) == " " then
                    temp_name = temp_name .. string.upper(self.name:sub(i, i))
                else
                    temp_name = temp_name .. string.lower(self.name:sub(i, i))
                end
            else
                temp_name = temp_name .. " "
            end
        end
        self.name = temp_name
    end,

    --- Formats the current name.
    --
    -- Removes leading, trailing and double spaces, trims the length, and ensures capitalization.
    format_name = function(self)
        local temp_name = self.name
        -- Trailing and leading spaces: Remove.
        temp_name = string.gsub(temp_name, '^%s*(.-)%s*$', '%1')
        -- Double (or more) spaces: One space.
        temp_name = temp_name:gsub("%s+", " ")
        -- Length. Trim to self.max_characters
        if #(temp_name) > self.max_characters then
            temp_name = temp_name:sub(1, self.max_characters)
        end
        -- Capitalization
        self.name = ""
        for i = 1, #(temp_name) do
            if i == 1 or temp_name:sub(i - 1, i - 1) == " " then
                self.name = self.name .. string.upper(temp_name:sub(i, i))
            else
                self.name = self.name .. string.lower(temp_name:sub(i, i))
            end
        end
    end,

    apply_name = function(self)

        if self.chocobo == false then
            Characters[self.id].name = self.name
            export_character_names()
        else
            -- TODO: Do chocobo.
        end
        self.hide(self)
    end,

    --- Hides the item menu and goes back to the main menu.
    --
    -- It also unpauses the game. Shows a fade effect.
    hide = function(self)
        script:request_end_sync(Script.UI, "Fade", "fade_out", 0)
        ui_manager:get_widget("NameMenu"):set_visible(false)
        UiContainer.current_menu = ""
        UiContainer.current_submenu = ""
        FFVII.MenuSettings.pause_available = true
        entity_manager:set_paused(false)
        script:request_end_sync(Script.UI, "Fade", "fade_in", 0)
        return 0;
    end,
}
