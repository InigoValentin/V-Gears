if UiContainer == nil then UiContainer = {} end

--- The name menu.
UiContainer.NameMenu = {

    --- Indicates if a chocobo is being renamed instead of a character.
    chocobo = false,

    --- ID of the character or chocobo being renamed.
    id = -1,

    --- Default name.
    default_name = "",

    --- Currently input-ed name
    name = "",

    --- Max number of characters allowed in a name.
    max_characters = 12,

    --- Current text cursor
    text_cursor = 1,

    letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",

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
            print("BUTTON " .. button .. " on " .. UiContainer.current_menu .. "." .. UiContainer.current_submenu)

            if UiContainer.current_submenu == "confirm" then
            elseif UiContainer.current_submenu == "" then
                local character_index = string.find(self.letters, button)
                if button == "Up" and event == "Press" then
                elseif button == "Down" then
                elseif button == "Left" then
                    if self.text_cursor > 1 then
                        self.text_cursor = self.text_cursor - 1
                        self.draw_cursor(self)
                    end
                elseif button == "Right" then
                    if self.text_cursor < self.max_characters then
                        self.text_cursor = self.text_cursor + 1
                        self.draw_cursor(self)
                    end
                elseif button == "Enter" then
                -- TODO: Else Letters
                elseif character_index ~= nil then
                    print(" LETTER " .. character_index)
                    self.set_char(self, character_index)
                    self.text_cursor = math.min(self.text_cursor + 1, self.max_characters)
                    self.populate_name(self)
                    self.draw_cursor(self)
                end
            end
        end
        return 0
    end,

    --- Opens the main menu.
    --
    -- Populates and updates displayed data before opening.
    show = function(self)
        entity_manager:set_paused(true)
        FFVII.MenuSettings.pause_available = false
        ui_manager:get_widget("NameMenu"):set_visible(true)
        UiContainer.current_menu = "name"
        UiContainer.current_submenu = ""
        ui_manager:get_widget("NameMenu.Container.Character.Portrait"):set_image("images/characters/" .. tostring(self.id) .. ".png")
        self.default_name = Characters[self.id].name
        self.name = Characters[self.id].name
        self.text_cursor = math.min(#(self.name) + 1, self.max_characters)
        self.populate_name(self)
        self.draw_cursor(self)
        return 0;
    end,

    draw_cursor = function(self)
        -- TODO: Do this with animations.
        for i = 1, self.max_characters do
            if self.text_cursor == i then
                print("")
                ui_manager:get_widget("NameMenu.Container.Character.Name.Score" .. tostring(i)):set_height(1.8)
                --ui_manager:get_widget("NameMenu.Container.Character.Name.Score" .. tostring(i)):set_default_animation("Active")
            else
                ui_manager:get_widget("NameMenu.Container.Character.Name.Score" .. tostring(i)):set_height(0.3)
                --ui_manager:get_widget("NameMenu.Container.Character.Name.Score" .. tostring(i)):set_default_animation("Inactive")
            end
        end
    end,

    populate_name = function(self)
        for i = 1, self.max_characters do
            if #(self.name) < i then
                ui_manager:get_widget("NameMenu.Container.Character.Name.Char" .. tostring(i)):set_text("")
            else
                ui_manager:get_widget("NameMenu.Container.Character.Name.Char" .. tostring(i)):set_text(self.name:sub(i, i))

            end
        end
    end,

    --- Sets a character in the current cursor position
    set_char = function(self, character_index)
        local temp_name = ""
        for i = 1, self.max_characters do
            if i == self.text_cursor then
                -- TODO: First position or after space: Use upper case table
                if self.text_cursor == 1 or self.name:sub(i - 1, i - 1) == " " then
                    temp_name = temp_name .. self.letters:sub(character_index, character_index)
                else
                    temp_name = temp_name .. string.lower(self.letters:sub(character_index, character_index))
                end
                print("NEW: " .. self.letters:sub(character_index, character_index))
            elseif #(self.name) >= i then
                -- TODO: First position or after space: to upper, else to lower
                temp_name = temp_name .. self.name:sub(i, i)
                print("OLD: " .. self.name:sub(i, i))
            end
        end
        self.name = temp_name
    end,

    --- Hides the item menu and goes back to the main menu.
    hide = function(self)
        ui_manager:get_widget("NameMenu"):set_visible(false)
        UiContainer.current_menu = ""
        UiContainer.current_submenu = ""
        FFVII.MenuSettings.pause_available = true
        entity_manager:set_paused(false)
        return 0;
    end,
}
