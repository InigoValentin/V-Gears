if UiContainer == nil then UiContainer = {} end

--- Entries of the main menu, by cursor position.
local ENTRY = {
    ITEM = 1, MAGIC = 2, MATERIA = 3, EQUIP = 4, STATUS = 5, ORDER = 6, LIMIT = 7,
    CONFIG = 8, PHS = 9, SAVE = 10, QUIT = 11,
}

--- Width in pixels of the inside of the level and limit bars.
local BAR_WIDTH = 62

--- Returned by a character action to keep the character selection open.
local STAY = "stay"

local CHARACTERS_CURSOR = "MainMenu.Container.Characters.Cursor"

--- The main menu.
UiContainer.MainMenu = UiContainer.Menu.new{

    id = "main",

    --- Cursor over the menu entries (1-11).
    cursor = UiContainer.Cursor.new{
        widget = "MainMenu.Container.Menu.Cursor",
        total = 11,
    },

    --- Cursor over the party slots (1-3).
    -- TODO: Skip empty character slots.
    character_cursor = UiContainer.Cursor.new{
        widget = CHARACTERS_CURSOR,
        total = 3,
    },

    --- When selecting a character, the menu entry that will be opened.
    --
    -- If 0, not selecting character.
    select_character_for_menu = 0,

    --- Entries that don't need a character. Return false to signal they can't be opened.
    entry_actions = {
        [ENTRY.ITEM] = function(self)
            script:request_end_sync(Script.UI, "ItemMenu", "show", 0)
        end,
        [ENTRY.CONFIG] = function(self)
            script:request_end_sync(Script.UI, "ConfigMenu", "show", 0)
        end,
        [ENTRY.PHS] = function(self)
            script:request_end_sync(Script.UI, "PhsMenu", "show", 0)
        end,
        [ENTRY.SAVE] = function(self)
            UiContainer.SaveMenu.operation = "save"
            script:request_end_sync(Script.UI, "SaveMenu", "show", 0)
        end,
        [ENTRY.QUIT] = function(self)
            script:request_end_sync(Script.UI, "QuitMenu", "show", 0)
        end,
    },

    --- Entries that need a character. Receive the party slot (1-3).
    --
    -- Return false if the entry can't be opened (error sound), or STAY to keep the character
    -- selection open instead of handing the control to another menu.
    character_actions = {
        [ENTRY.MAGIC] = function(self, slot)
            UiContainer.current_character_id = Party[slot]
            script:request_end_sync(Script.UI, "MagicMenu", "show", 0)
        end,
        [ENTRY.MATERIA] = function(self, slot)
            UiContainer.current_character_id = Party[slot]
            script:request_end_sync(Script.UI, "MateriaMenu", "show", 0)
        end,
        [ENTRY.EQUIP] = function(self, slot)
            UiContainer.current_character_id = Party[slot]
            script:request_end_sync(Script.UI, "EquipMenu", "show", 0)
        end,
        [ENTRY.STATUS] = function(self, slot)
            UiContainer.current_character_id = Party[slot]
            script:request_end_sync(Script.UI, "StatusMenu", "show", 0)
        end,
        --- Toggles the row (front/back) of the character. Stays in the character selection.
        [ENTRY.ORDER] = function(self, slot)
            local character = Characters[Party[slot]]
            character.back_row = not character.back_row
            self:update_portrait(slot)
            return STAY
        end,
        [ENTRY.LIMIT] = function(self, slot)
            UiContainer.current_character_id = Party[slot]
            script:request_end_sync(Script.UI, "LimitMenu", "show", 0)
        end,
    },

    states = {
        --- Choosing an entry of the menu.
        [""] = {
            cursor = function(self) return self.cursor end,
            axis = "vertical",
            cancel = function(self)
                script:request_end_sync(Script.UI, "MainMenu", "hide", 0)
            end,
            confirm = function(self, cursor)
                local position = cursor:index()
                local action = self.entry_actions[position]
                if action ~= nil then
                    return action(self)
                end
                -- Any other entry needs a character.
                self.select_character_for_menu = position
                self:set_state("main_character")
                ui_manager:get_widget(CHARACTERS_CURSOR):set_visible(true)
            end,
        },

        --- Choosing the character to open the entry for.
        main_character = {
            cursor = function(self) return self.character_cursor end,
            axis = "vertical",
            cancel = function(self)
                self:set_state("")
                ui_manager:get_widget(CHARACTERS_CURSOR):set_visible(false)
            end,
            confirm = function(self, cursor)
                local slot = cursor:index()
                local action = self.character_actions[self.select_character_for_menu]
                if action == nil or Party[slot] == nil or Characters[Party[slot]] == nil then
                    return false
                end
                local result = action(self, slot)
                if result == false or result == STAY then
                    return result
                end
                -- The chosen menu takes over. When it closes, the main menu is back at the entry
                -- list, so the character cursor must not stay behind.
                self.select_character_for_menu = 0
                self:set_state("")
                ui_manager:get_widget(CHARACTERS_CURSOR):set_visible(false)
            end,
        },
    },

    --- Handles the menu button while the main menu is closed.
    on_inactive_button = function(self, button, event)
        if ui_manager:get_widget("MainMenu"):is_visible() == false
            and MenuSettings.available == true
            and button == Config.ACTIONS.MENU and event == "Press"
        then
            UiContainer.play_sound("Back")
            script:request_end_sync(Script.UI, "MainMenu", "show", 0)
        end
    end,

    --- Moves the portrait of a party slot according to the row: back row is 20 px to the right.
    update_portrait = function(self, slot)
        local character = Characters[Party[slot]]
        local portrait = ui_manager:get_widget("MainMenu.Container.Characters.Character" .. tostring(slot) .. ".Portrait")
        portrait:set_default_animation(character.back_row and "RowBack" or "RowFront")
    end,

    --- Shows the data of one party slot.
    populate_character = function(self, slot)
        local path = "MainMenu.Container.Characters.Character" .. tostring(slot)
        local character = Party[slot] ~= nil and Characters[Party[slot]] or nil
        UiContainer.populate_character_data(path, character)
        if character == nil then return end
        ui_manager:get_widget(path .. ".Portrait"):set_image("characters/" .. tostring(Party[slot]) .. ".png")
        self:update_portrait(slot)
        ui_manager:get_widget(path .. ".LimitLevel"):set_text(tostring(character.limit.current))
        UiContainer.set_limit_bar(path .. ".LimitLinePart", character, BAR_WIDTH)
        UiContainer.set_bar(path .. ".NextLevelLinePart", UiContainer.exp_progress(character), BAR_WIDTH)
    end,

    --- Refreshes the data a submenu may have changed (level, HP/MP, limit, row, gil...).
    --
    -- Runs when a submenu closes and gives the control back to the main menu.
    on_resume = function(self)
        for slot = 1, 3 do
            self:populate_character(slot)
        end
        ui_manager:get_widget("MainMenu.Container.TimeGil.Gil"):set_text(UiContainer.format_money(Inventory.money))
    end,

    --- Opens the main menu.
    --
    -- Populates and updates displayed data before opening. It also pauses the game.
    show = function(self)
        self:activate()
        entity_manager:set_paused(true)
        MenuSettings.pause_available = false
        ui_manager:get_widget("MainMenu"):set_visible(true)

        for slot = 1, 3 do
            self:populate_character(slot)
        end

        -- TODO: Set portraits of the correct char

        ui_manager:get_widget("MainMenu.Container.Menu.SaveText"):set_colour(0.4, 0.4, 0.4)
        ui_manager:get_widget("MainMenu.Container.Location.Text"):set_text(System["MapChanger"].location_name)
        ui_manager:get_widget("MainMenu.Container.TimeGil.Gil"):set_text(UiContainer.format_money(Inventory.money))

        local location = ui_manager:get_widget("MainMenu.Container.Location")
        ui_manager:get_widget("MainMenu.Container.Characters"):play_animation_stop("Appear")
        ui_manager:get_widget("MainMenu.Container.Menu"):play_animation_stop("Appear")
        ui_manager:get_widget("MainMenu.Container.TimeGil"):play_animation_stop("Appear")
        location:play_animation_stop("Appear")
        -- The cursor starts at the first entry, or where it was left if the cursor memory is on.
        if not UiContainer.cursor_memory() then self.cursor:set(1) end
        self.cursor:refresh()
        location:animation_sync()
        return 0
    end,

    --- Hides the main menu and goes back to the game.
    --
    -- It also unpauses the game.
    hide = function(self)
        self:deactivate("")
        local location = ui_manager:get_widget("MainMenu.Container.Location")

        ui_manager:get_widget("MainMenu.Container.Characters"):play_animation_stop("Disappear")
        ui_manager:get_widget("MainMenu.Container.Menu"):play_animation_stop("Disappear")
        ui_manager:get_widget("MainMenu.Container.TimeGil"):play_animation_stop("Disappear")
        location:play_animation_stop("Disappear")
        location:animation_sync()

        ui_manager:get_widget("MainMenu"):set_visible(false)

        MenuSettings.pause_available = true
        entity_manager:set_paused(false)

        return 0
    end,
}
