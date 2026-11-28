if UiContainer == nil then UiContainer = {} end

--- The item menu.
UiContainer.MateriaMenu = {

    --- Current header cursor row (1: Check/Weapon, 2: Order/Armor).
    header_position_row = 1,

    --- Current header cursor position (1: Check/Order, 2-9: Weapon/Armor slots).
    header_position = 1,

    --- Current header cursor max row.
    header_position_row_total = 2,

    --- Current header cursor max position.
    header_position_total = 9,

    --- ID of the character the menu is open for.
    char_id = -1,

    --- Cursor position in the materia list.
    list_position = 1,

    --- Max cursor postion in the materia list.
    list_position_total = 10,

    --- First visible position in the materia list
    list_first_visible = 1,

    --- Max materias in the list
    list_size = 200,

    --- Commands of the character, shown in the Check window.
    commands = {},

    --- Cursor over the commands of the Check window (columns of four, filled top to bottom).
    command_cursor = UiContainer.Cursor.new{
        widget = "MateriaMenu.Container.Details.Commands.Box.Cursor",
        total = 1,
        on_move = function() UiContainer.MateriaMenu:populate_command_help() end,
    },

    --- Cursor over the options of the Order menu.
    order_cursor = UiContainer.Cursor.new{
        widget = "MateriaMenu.Container.OrderMenu.Cursor",
        total = 4,
        wrap = true,
        on_move = function() UiContainer.MateriaMenu:populate_order_help() end,
    },

    --- Descriptions of the options of the Order menu.
    ORDER_HELP = {
        "Sort the materia in the inventory",
        "Exchange materia between the characters and the inventory",
        "Remove all the materia equipped on the character",
        "Trash materia",
    },

    --- Options of the Order menu that are not available yet.
    ORDER_DISABLED = {},

    --- True while the materia list is used to choose a materia to trash (Order > Trash).
    trash_mode = false,

    --- Entries of the list of spells, summons or enemy skills of a command.
    abilities = {},

    --- Cursor over the abilities list.
    ability_cursor = UiContainer.ListCursor.new{
        widget = "MateriaMenu.Container.Abilities.Cursor",
        rows = 10,
        on_scroll = function() UiContainer.MateriaMenu:populate_abilities() end,
        on_move = function() UiContainer.MateriaMenu:populate_ability_help() end,
    },

    --- Run when the menu is creaded.
    --
    -- It does nothing.
    on_start = function(self)
        return 0
    end,

    --- Handles button events.
    --
    -- For the current submenu, handles directional keys, enter and escape events.
    -- @param button Pressed direction or action name.
    -- @param event Trigger event. Normally, "Press".
    on_button = function(self, button, event)
        if UiContainer.current_menu == "materia" then
            if UiContainer.current_submenu == "" then
                -- TODO: Handle L1/R1 (change character)
                -- TODO: Handle square: Materia menu
                if button == Config.ACTIONS.CANCEL and event == "Press" then
                    audio_manager:play_sound("Back")
                    UiContainer.current_menu = "main"
                    script:request_end_sync(Script.UI, "MateriaMenu", "hide", 0)
                elseif button == Config.CONTROLS.DOWN then
                    audio_manager:play_sound("Cursor")
                    self.header_position_row = self.header_position_row + 1
                    if self.header_position_row > self.header_position_row_total then
                        self.header_position_row = 1
                    end
                    ui_manager:get_widget("MateriaMenu.Container.Character.Cursor"):set_default_animation("Position" .. self.header_position_row .. "-" .. self.header_position)
                    self.populate_details(self, false)
                elseif button == Config.CONTROLS.UP then
                    audio_manager:play_sound("Cursor")
                    self.header_position_row = self.header_position_row - 1
                    if self.header_position_row < 1 then
                        self.header_position_row = self.header_position_row_total
                    end
                    ui_manager:get_widget("MateriaMenu.Container.Character.Cursor"):set_default_animation("Position" .. self.header_position_row .. "-" .. self.header_position)
                    self.populate_details(self, false)
                elseif button == Config.CONTROLS.RIGHT then
                    audio_manager:play_sound("Cursor")
                    self.header_position = self.header_position + 1
                    if self.header_position > self.header_position_total then
                        self.header_position = 1
                    end
                    ui_manager:get_widget("MateriaMenu.Container.Character.Cursor"):set_default_animation("Position" .. self.header_position_row .. "-" .. self.header_position)
                    self.populate_details(self, false)
                elseif button == Config.CONTROLS.LEFT then
                    audio_manager:play_sound("Cursor")
                    self.header_position = self.header_position - 1
                    if self.header_position < 1 then
                        self.header_position = self.header_position_total
                    end
                    ui_manager:get_widget("MateriaMenu.Container.Character.Cursor"):set_default_animation("Position" .. self.header_position_row .. "-" .. self.header_position)
                    self.populate_details(self, false)
                elseif button == Config.ACTIONS.CONFIRM then
                    if self.header_position_row == 1 and self.header_position == 1 then
                        if #self.commands == 0 then
                            audio_manager:play_sound("Error")
                        else
                            audio_manager:play_sound("Cursor")
                            self.submenu_commands(self)
                        end
                    elseif self.header_position_row == 2 and self.header_position == 1 then
                        audio_manager:play_sound("Cursor")
                        self.submenu_order(self)
                    else
                        -- TODO: Check for  avalid slot before entering list
                        if self.header_position_row == 1 and #(Game.Items[Characters[self.char_id].weapon.id].slots) > self.header_position - 2 then
                            self.submenu_list(self)
                        elseif self.header_position_row == 2 and #(Game.Items[Characters[self.char_id].armor.id].slots) > self.header_position -2  then
                            self.submenu_list(self)
                        else
                            audio_manager:play_sound("Error")
                            return 0
                        end
                        audio_manager:play_sound("Cursor")
                    end
                -- TODO: Triangle->Remove
                else
                    return 0
                end
            elseif UiContainer.current_submenu == "order" then
                if button == Config.ACTIONS.CANCEL and event == "Press" then
                    audio_manager:play_sound("Back")
                    self.submenu_none(self)
                elseif button == Config.ACTIONS.CONFIRM and event == "Press" then
                    self.run_order(self)
                elseif event == "Press" or event == "Repeat" then
                    self.order_cursor:handle(button, "vertical")
                end
            elseif UiContainer.current_submenu == "commands" then
                if button == Config.ACTIONS.CANCEL and event == "Press" then
                    audio_manager:play_sound("Back")
                    self.submenu_none(self)
                elseif button == Config.ACTIONS.CONFIRM and event == "Press" then
                    self.open_command(self)
                elseif event == "Press" or event == "Repeat" then
                    self:move_command_cursor(button)
                end
            elseif UiContainer.current_submenu == "abilities" then
                if button == Config.ACTIONS.CANCEL and event == "Press" then
                    audio_manager:play_sound("Back")
                    self.submenu_commands(self)
                elseif event == "Press" or event == "Repeat" then
                    self.ability_cursor:handle(button, "vertical")
                end
            elseif UiContainer.current_submenu == "list" then
                if button == Config.CONTROLS.DOWN then
                    if self.list_first_visible + self.list_position > self.list_size then
                        -- At the very end, do nothing
                        audio_manager:play_sound("Error")
                        return 0
                    end
                    audio_manager:play_sound("Cursor")
                    if self.list_position == self.list_position_total then
                        -- The cursor is at the end. Scroll page, but dont change cursor.
                        self.list_first_visible = self.list_first_visible + 1
                        self.populate_list(self)
                    else
                        -- The cursor is not at the end, move cursor down.
                        self.list_position = self.list_position + 1
                    end
                    ui_manager:get_widget("MateriaMenu.Container.List.Cursor"):set_default_animation("Position" .. self.list_position)
                    -- Show details
                    self.populate_details(self, true)
                elseif button == Config.CONTROLS.UP then
                    if self.list_first_visible == 1 and self.list_position == 1 then
                        -- At the very begining, do nothing
                        audio_manager:play_sound("Error")
                        return 0
                    end
                    audio_manager:play_sound("Cursor")
                    if self.list_position == 1 then
                        -- The cursor is at the end. Scroll page, but dont change cursor.
                        self.list_first_visible = self.list_first_visible - 1
                        self.populate_list(self)
                    else
                        -- The cursor is not at the end, move cursor down.
                        self.list_position = self.list_position - 1
                    end
                    ui_manager:get_widget("MateriaMenu.Container.List.Cursor"):set_default_animation("Position" .. self.list_position)
                    -- Show details
                    self.populate_details(self, true)
                elseif button == Config.ACTIONS.R1 then
                    if self.list_first_visible + self.list_position_total < self.list_size - self.list_position_total then
                        audio_manager:play_sound("Cursor")
                        self.list_first_visible = self.list_first_visible + self.list_position_total
                        self.populate_list(self)
                        self.populate_details(self, true)
                    else
                        audio_manager:play_sound("Error")
                    end
                elseif button == Config.ACTIONS.L1 then
                    if self.list_first_visible - self.list_position_total > 1 then
                        audio_manager:play_sound("Cursor")
                        self.list_first_visible = self.list_first_visible - self.list_position_total
                        self.populate_list(self)
                        self.populate_details(self, true)
                    else
                        audio_manager:play_sound("Error")
                    end
                elseif button == Config.ACTIONS.CANCEL and event == "Press" then
                    audio_manager:play_sound("Back")
                    if self.trash_mode then
                        self.submenu_order(self)
                        self.order_cursor:set(4)
                    else
                        self.submenu_none(self)
                    end
                elseif button == Config.ACTIONS.CONFIRM then
                    if self.trash_mode then
                        self.submenu_trash_confirm(self)
                    else
                        -- TODO: Replace for "Equip materia" sound.
                        audio_manager:play_sound("Cursor")
                        self.equip_materia(self)
                    end
                end
            elseif UiContainer.current_submenu == "trash_confirm" then
                if button == Config.ACTIONS.CANCEL and event == "Press" then
                    audio_manager:play_sound("Back")
                    self.submenu_trash(self)
                elseif button == Config.ACTIONS.CONFIRM and event == "Press" then
                    self.trash_materia(self)
                end
            end
        end
        return 0
    end,

    --- Opens the materia menu.
    --
    -- Populates and updates displayed data before opening.
    show = function(self)
        self.char_id = UiContainer.current_character_id
        ui_manager:get_widget("MateriaMenu"):set_visible(true)
        UiContainer.current_menu = "materia"
        UiContainer.current_submenu = ""
        self.header_position_row = 1
        self.header_position = 1
        self.list_position = 1
        self.list_first_visible = 1
        self.populate_list(self)
        self.submenu_none(self)
        self.populate_commands(self)
        return 0;
    end,

    --- Enters or returns to the main equip menu.
    submenu_none = function(self)
        UiContainer.current_submenu = ""
        self.trash_mode = false
        ui_manager:get_widget("MateriaMenu.Container.TrashWindow"):set_visible(false)
        UiContainer.populate_character_data("MateriaMenu.Container.Character", Characters[self.char_id])
        ui_manager:get_widget("MateriaMenu.Container.Character.Portrait"):set_image("characters/" .. tostring(self.char_id) .. ".png")
        self.populate_equip(self)
        -- TODO: Do something for chars 9 and 10
        ui_manager:get_widget("MateriaMenu.Container.Character.WpnIcon"):set_image("icons/item_weapon_" .. tostring(self.char_id) .. ".png")
        ui_manager:get_widget("MateriaMenu.Container.List.Cursor"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.Abilities"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.OrderMenu"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.Details.Commands.Box.Cursor"):set_visible(false)
        --TODO: Uncomment when implemented: ui_manager:get_widget("MateriaMenu.Container.Order.Cursor"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.Character.Cursor"):set_visible(true)
        self.populate_details(self, false)
    end,

    --- Refreshes the menu after the exchange menu closed (the equipment and stats may have changed).
    on_exchange_closed = function(self)
        UiContainer.populate_character_data("MateriaMenu.Container.Character", Characters[self.char_id])
        self.populate_equip(self)
        self.populate_list(self)
        self.populate_order_help(self)
    end,

    --- Opens the Order menu (Arrange, Exchange, Remove all, Trash).
    submenu_order = function(self)
        UiContainer.current_submenu = "order"
        self.trash_mode = false
        ui_manager:get_widget("MateriaMenu.Container.TrashWindow"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.List.Cursor"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.OrderMenu"):set_visible(true)
        for option = 1, 4 do
            local item = ui_manager:get_widget("MateriaMenu.Container.OrderMenu.Item" .. option)
            if self.ORDER_DISABLED[option] then item:set_colour(0.6, 0.6, 0.6) else item:set_colour(1, 1, 1) end
        end
        self.order_cursor:set(1)
    end,

    --- Enters (or returns to) the materia list to choose the materia to trash.
    submenu_trash = function(self)
        self.trash_mode = true
        -- The Order popup stays open (with Trash highlighted) while choosing.
        ui_manager:get_widget("MateriaMenu.Container.TrashWindow"):set_visible(false)
        self.submenu_list(self)
    end,

    --- Opens the window that shows the highlighted materia before it is trashed.
    submenu_trash_confirm = function(self)
        local materia = Materia[self.list_first_visible + self.list_position - 1]
        if materia == nil then
            audio_manager:play_sound("Error")
            return
        end
        audio_manager:play_sound("Cursor")
        UiContainer.current_submenu = "trash_confirm"
        local data = Game.Materia[materia.id]
        local window = "MateriaMenu.Container.TrashWindow."
        local levels = data.levels_ap or {}
        local level = 0
        for _, level_ap in ipairs(levels) do
            if level_ap > (materia.ap or 0) then break end
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
        ui_manager:get_widget(window .. "AP"):set_text(UiContainer.pad_value(materia.ap or 0, 8))
        ui_manager:get_widget("MateriaMenu.Container.TrashWindow"):set_visible(true)
    end,

    --- Discards the materia shown in the trash window and goes back to the list.
    trash_materia = function(self)
        Materia[self.list_first_visible + self.list_position - 1] = nil
        audio_manager:play_sound("Cursor")
        self.populate_list(self)
        self.submenu_trash(self)
    end,

    --- Shows the description of the highlighted option of the Order menu.
    populate_order_help = function(self)
        ui_manager:get_widget("MateriaMenu.Container.Help.Help"):set_text(self.ORDER_HELP[self.order_cursor:index()] or "")
    end,

    --- Runs the highlighted option of the Order menu.
    run_order = function(self)
        local option = self.order_cursor:index()
        if self.ORDER_DISABLED[option] then
            audio_manager:play_sound("Error")
        elseif option == 1 then
            audio_manager:play_sound("Cursor")
            self.arrange_materia(self)
        elseif option == 2 then
            audio_manager:play_sound("Cursor")
            script:request_end_sync(Script.UI, "ExchangeMenu", "show", 0)
        elseif option == 4 then
            audio_manager:play_sound("Cursor")
            self.submenu_trash(self)
        elseif option == 3 then
            if self.remove_all_materia(self) then
                audio_manager:play_sound("Cursor")
            else
                audio_manager:play_sound("Error")
            end
        end
    end,

    --- IDs of the master materia (Master Command, Master Magic and Master Summon).
    MASTER_MATERIA = {[48] = true, [73] = true, [90] = true},

    --- Sorts the materia of the inventory, as the original game does.
    --
    -- By type (the order of Materia.TYPE: magic, support, command, independent, summon); in each
    -- type the master materia go first, then by ID ascending and by AP descending. The empty slots
    -- go at the end.
    arrange_materia = function(self)
        local sorted = {}
        for slot = 1, MAX_MATERIA_SLOTS do
            if Materia[slot] ~= nil then sorted[#sorted + 1] = {materia = Materia[slot], order = #sorted} end
        end
        local master = self.MASTER_MATERIA
        table.sort(sorted, function(l, r)
            local l_id, r_id = l.materia.id, r.materia.id
            local l_type, r_type = Game.Materia[l_id].type, Game.Materia[r_id].type
            if l_type ~= r_type then return l_type < r_type end
            if (master[l_id] == true) ~= (master[r_id] == true) then return master[l_id] == true end
            if l_id ~= r_id then return l_id < r_id end
            if (l.materia.ap or 0) ~= (r.materia.ap or 0) then return (l.materia.ap or 0) > (r.materia.ap or 0) end
            return l.order < r.order
        end)
        for slot = 1, MAX_MATERIA_SLOTS do
            Materia[slot] = sorted[slot] ~= nil and sorted[slot].materia or nil
        end
        self.list_first_visible = 1
        self.list_position = 1
        ui_manager:get_widget("MateriaMenu.Container.List.Cursor"):set_default_animation("Position1")
        self.populate_list(self)
    end,

    --- Moves all the materia equipped on the character to the inventory.
    --
    -- If the inventory has not room for all of it, as many as fit are removed (the ones of the weapon
    -- first, in slot order).
    -- @return False if nothing was removed.
    remove_all_materia = function(self)
        local equipped = {}
        local character = Characters[self.char_id]
        for _, equipment in ipairs({character.weapon, character.armor}) do
            for slot = 0, 7 do
                if equipment.materia[slot] ~= nil then equipped[#equipped + 1] = {equipment, slot} end
            end
        end
        local free = {}
        for slot = 1, MAX_MATERIA_SLOTS do
            if Materia[slot] == nil then free[#free + 1] = slot end
        end
        local count = math.min(#equipped, #free)
        if count == 0 then return false end
        for i = 1, count do
            local place = equipped[i]
            Materia[free[i]] = place[1].materia[place[2]]
            place[1].materia[place[2]] = nil
        end
        -- The materia change the stats, and the current HP and MP can't be over the new maximums.
        Characters.recalculate_stats(self.char_id)
        self.submenu_none(self)
        self.populate_list(self)
        return true
    end,

    --- Enters (or returns to) the navigable list of commands of the Check window.
    submenu_commands = function(self)
        UiContainer.current_submenu = "commands"
        ui_manager:get_widget("MateriaMenu.Container.Abilities"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.Details.Commands.Box.Cursor"):set_visible(true)
        self.command_cursor.total = #self.commands
        if self.command_cursor.position > #self.commands then self.command_cursor.position = 1 end
        self.command_cursor:refresh()
    end,

    --- Number of commands in each column of the Check window.
    COMMAND_ROWS = 4,

    --- Moves the command cursor. The commands fill the columns from top to bottom.
    --
    -- Up and down wrap inside the column, left and right wrap around the columns keeping the row
    -- (or going to the last command of a shorter column).
    move_command_cursor = function(self, button)
        local rows = self.COMMAND_ROWS
        local index = self.command_cursor:index() - 1
        local column, row = math.floor(index / rows), index % rows
        local columns = math.ceil(#self.commands / rows)
        local function length(c) return math.min(rows, #self.commands - c * rows) end
        if button == Config.CONTROLS.UP then
            row = (row - 1) % length(column)
        elseif button == Config.CONTROLS.DOWN then
            row = (row + 1) % length(column)
        elseif button == Config.CONTROLS.LEFT then
            column = (column - 1) % columns
            row = math.min(row, length(column) - 1)
        elseif button == Config.CONTROLS.RIGHT then
            column = (column + 1) % columns
            row = math.min(row, length(column) - 1)
        else
            return
        end
        audio_manager:play_sound("Cursor")
        self.command_cursor:set(column * rows + row + 1)
    end,

    --- Shows the description of the highlighted command.
    populate_command_help = function(self)
        local command = Game.Commands[self.commands[self.command_cursor:index()]]
        ui_manager:get_widget("MateriaMenu.Container.Help.Help"):set_text(command ~= nil and command.description or "")
    end,

    --- Gets the kind of list a command opens.
    --
    -- @param command Definition of the command (an entry of Game.Commands).
    -- @return "magic", "summon" or "eskill", or nil if the command has no list.
    command_list_kind = function(self, command)
        local menu = command ~= nil and command.menu or nil
        if menu == Battle.Menu.MAGIC or menu == Battle.Menu.W_MAGIC then return "magic"
        elseif menu == Battle.Menu.SUMMON or menu == Battle.Menu.W_SUMMON then return "summon"
        elseif menu == Battle.Menu.E_SKILL then return "eskill"
        end
        return nil
    end,

    --- Opens the list of a command (spells, summons or enemy skills), if it has one.
    open_command = function(self)
        local command = Game.Commands[self.commands[self.command_cursor:index()]]
        local kind = self:command_list_kind(command)
        local entries = kind ~= nil and self:build_abilities(kind) or {}
        if #entries == 0 then
            audio_manager:play_sound("Error")
            return
        end
        audio_manager:play_sound("Cursor")
        self.abilities = entries
        UiContainer.current_submenu = "abilities"
        ui_manager:get_widget("MateriaMenu.Container.Abilities"):set_visible(true)
        self.ability_cursor.count = #entries
        self.ability_cursor:reset()
        self:populate_abilities()
    end,

    --- Gets the spells, summons or enemy skills the character can use in battle.
    --
    -- @param kind "magic", "summon" or "eskill".
    -- @return List of {attack = ID of the attack, times = uses per battle of a summon}.
    build_abilities = function(self, kind)
        local abilities = Characters.get_abilities(self.char_id)
        local entries = {}
        if kind == "magic" then
            -- The spells of a materia go together.
            for _, group in ipairs(abilities.magic_groups) do
                for _, spell in ipairs(group) do entries[#entries + 1] = {attack = spell} end
            end
        elseif kind == "summon" then
            for _, summon in ipairs(abilities.summon) do
                entries[#entries + 1] = {attack = summon.id, times = summon.times}
            end
        else
            for _, skill in ipairs(abilities.e_skill) do entries[#entries + 1] = {attack = skill} end
        end
        return entries
    end,

    --- Fills the rows of the abilities list.
    populate_abilities = function(self)
        local window = "MateriaMenu.Container.Abilities."
        for row = 1, 10 do
            local entry = self.abilities[self.ability_cursor.first + row - 1]
            local data = entry ~= nil and Game.Attacks[entry.attack] or nil
            if data == nil then
                ui_manager:get_widget(window .. "Name" .. row):set_text("")
                ui_manager:get_widget(window .. "Mp" .. row):set_text("")
            else
                local name = data.name
                if entry.times ~= nil and entry.times < 255 then name = name .. " x" .. tostring(entry.times) end
                ui_manager:get_widget(window .. "Name" .. row):set_text(name)
                ui_manager:get_widget(window .. "Mp" .. row):set_text(UiContainer.pad_value(data.mp or 0, 3))
            end
        end
        self:populate_ability_help()
    end,

    --- Shows the description of the highlighted ability.
    populate_ability_help = function(self)
        local entry = self.abilities[self.ability_cursor:index()]
        local data = entry ~= nil and Game.Attacks[entry.attack] or nil
        ui_manager:get_widget("MateriaMenu.Container.Help.Help"):set_text(data ~= nil and data.description or "")
    end,

    submenu_list = function(self)
        UiContainer.current_submenu = "list"
        ui_manager:get_widget("MateriaMenu.Container.List.Cursor"):set_visible(true)
        self.populate_details(self, true)
    end,

    --- Populate the equipment section.
    --
    -- It fills the header with weapn and armor names, slots, and current materia.
    populate_equip = function(self)
        local weapon = Game.Items[Characters[self.char_id].weapon.id]
        local armor = Game.Items[Characters[self.char_id].armor.id]
        ui_manager:get_widget("MateriaMenu.Container.Character.WpnLbl"):set_text(weapon.name)
        ui_manager:get_widget("MateriaMenu.Container.Character.ArmLbl"):set_text(armor.name)
        for i = 1, 8 do
            -- Show/hide slots
            -- TODO: Change image for no-growth slots?
            if #(weapon.slots) >= i then
                ui_manager:get_widget("MateriaMenu.Container.Character.WeaponSlots.Slot" .. tostring(i)):set_visible(true)
            else
                ui_manager:get_widget("MateriaMenu.Container.Character.WeaponSlots.Slot" .. tostring(i)):set_visible(false)
            end
            if #(armor.slots) >= i then
                ui_manager:get_widget("MateriaMenu.Container.Character.ArmorSlots.Slot" .. tostring(i)):set_visible(true)
            else
                ui_manager:get_widget("MateriaMenu.Container.Character.ArmorSlots.Slot" .. tostring(i)):set_visible(false)
            end
            -- Show/hide linkers
            if i % 2 ~= 0 then
                if #(weapon.slots) > i  and weapon.slots[i] == 1 then
                    ui_manager:get_widget("MateriaMenu.Container.Character.WeaponSlots.Link" .. tostring(math.floor(i / 2) + 1)):set_visible(true)
                else
                    ui_manager:get_widget("MateriaMenu.Container.Character.WeaponSlots.Link" .. tostring(math.floor(i / 2) + 1)):set_visible(false)
                end
                if #(armor.slots) > i  and armor.slots[i] == 1 then
                    ui_manager:get_widget("MateriaMenu.Container.Character.ArmorSlots.Link" .. tostring(math.floor(i / 2) + 1)):set_visible(true)
                else
                    ui_manager:get_widget("MateriaMenu.Container.Character.ArmorSlots.Link" .. tostring(math.floor(i / 2) + 1)):set_visible(false)
                end
            end
            -- Show/hide/set materia
            if Characters[self.char_id].weapon.materia[i - 1] ~= nil then
                local mat_type =  Game.Materia[Characters[self.char_id].weapon.materia[i - 1].id].type
                ui_manager:get_widget("MateriaMenu.Container.Character.WeaponSlots.Mat" .. tostring(i)):set_image("icons/materia_" .. tostring(mat_type) .. ".png")
                ui_manager:get_widget("MateriaMenu.Container.Character.WeaponSlots.Mat" .. tostring(i)):set_visible(true)
            else
                ui_manager:get_widget("MateriaMenu.Container.Character.WeaponSlots.Mat" .. tostring(i)):set_visible(false)
            end
            if Characters[self.char_id].armor.materia[i - 1] ~= nil then
                local mat_type =  Game.Materia[Characters[self.char_id].armor.materia[i - 1].id].type
                ui_manager:get_widget("MateriaMenu.Container.Character.ArmorSlots.Mat" .. tostring(i)):set_image("icons/materia_" .. tostring(mat_type) .. ".png")
                ui_manager:get_widget("MateriaMenu.Container.Character.ArmorSlots.Mat" .. tostring(i)):set_visible(true)
            else
                ui_manager:get_widget("MateriaMenu.Container.Character.ArmorSlots.Mat" .. tostring(i)):set_visible(false)
            end
        end
    end,

    --- Hiddes the materia details.
    --
    -- Must be called when the cursor goes into an empty slot
    hide_details = function(self)
        -- TODO
        ui_manager:get_widget("MateriaMenu.Container.Details.Empty"):set_visible(true)
        ui_manager:get_widget("MateriaMenu.Container.Details.Commands"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.Details.EnemySkill"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.Details.Materia"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.Help.Help"):set_text("")
    end,

    --- Show details of an enemy skill materia.
    --
    -- Must be called when the cursor goes over an enemy skill materia
    --
    -- @param materia Selected Enemy Skill materia.
    populate_e_skill_details = function(self, materia)
        if materia.id ~= Materia.ENEMY_SKILL_ID then
            print("ERROR: Tried to get enemy skill info form non-enemy skill materia with ID " .. tostring(materia.id))
            return 0
        elseif materia.skills == nil then
            print("ERROR: Enemy skill materia has no skill information")
            --return 0
        end
        ui_manager:get_widget("MateriaMenu.Container.Help.Help"):set_text(Game.Materia[materia.id].description)
        ui_manager:get_widget("MateriaMenu.Container.Details.EnemySkill.Name"):set_text(Game.Materia[materia.id].name)
        ui_manager:get_widget("MateriaMenu.Container.Details.Empty"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.Details.Commands"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.Details.EnemySkill"):set_visible(true)
        ui_manager:get_widget("MateriaMenu.Container.Details.Materia"):set_visible(false)
        for s = 1, 24 do
            if materia.skills[s] ~= nil and materia.skills[s] == true then
                ui_manager:get_widget("MateriaMenu.Container.Details.EnemySkill.Stars.Star" .. tostring(s)):set_image("icons/materia_star_3.png")
                ui_manager:get_widget("MateriaMenu.Container.Details.EnemySkill.Skills.Skill" .. tostring(s)):set_text(Game.Attacks[71 + s].name)
                ui_manager:get_widget("MateriaMenu.Container.Details.EnemySkill.Skills.Skill" .. tostring(s)):set_visible(true)
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.EnemySkill.Stars.Star" .. tostring(s)):set_image("icons/materia_star_empty_3.png")
                ui_manager:get_widget("MateriaMenu.Container.Details.EnemySkill.Skills.Skill" .. tostring(s)):set_visible(false)
            end
        end


    end,

    --- Displays the command list in the details window
    populate_commands = function(self)
        ui_manager:get_widget("MateriaMenu.Container.Details.Empty"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.Details.Commands"):set_visible(true)
        ui_manager:get_widget("MateriaMenu.Container.Details.EnemySkill"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.Details.Materia"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.Help.Help"):set_text("")
        -- The commands the character has in battle.
        local commands = Characters.get_commands(self.char_id)
        self.commands = commands
        for i = 1, 12 do
            local widget = ui_manager:get_widget("MateriaMenu.Container.Details.Commands.Box.Command" .. i)
            local command = Game.Commands[commands[i]]
            if command ~= nil then
                widget:set_text(command.name)
                widget:set_visible(true)
                -- Commands with a list but nothing in it (no enemy skill learned...) are dimmed.
                local kind = self:command_list_kind(command)
                if kind ~= nil and #self:build_abilities(kind) == 0 then
                    widget:set_colour(0.6, 0.6, 0.6)
                else
                    widget:set_colour(1, 1, 1)
                end
            else
                widget:set_visible(false)
            end
        end
    end,

    --- Populated the details of the selected header item (check, materia, or none).
    --
    -- @param from_list If true, data will be read from the item higlighted in the list on the
    -- right. If false, data wil be red from the currently equipped items.
    populate_details = function(self, from_list)
        local id
        local ap
        local ap_next
        local level
        local max_level
        local master = false
        local materia -- Materia static information, as in Game.Materia[]
        local materia_selected -- Materia instance information, as in Materia[]
        if from_list == false then
            -- Get data from equiped materia
            if self.header_position_row == 1 then
                -- "Check" highlighted
                if self.header_position == 1 then
                    self.populate_commands(self)
                    return 0
                end
                -- Weapon materia
                local weapon_id = Characters[self.char_id].weapon.id
                local weapon = Game.Items[weapon_id]
                if #(weapon.slots) < self.header_position -1 or Characters[self.char_id].weapon.materia[self.header_position - 2] == nil then
                    -- No slot or no materia in slot. Clear and exit
                    self.hide_details(self)
                    return 0
                end
                materia_selected = Characters[self.char_id].weapon.materia[self.header_position - 2]
                id = Characters[self.char_id].weapon.materia[self.header_position - 2].id
                ap = Characters[self.char_id].weapon.materia[self.header_position - 2].ap
                materia = Game.Materia[id]
            elseif self.header_position_row == 2 then
                -- "Order" highlighted
                if self.header_position == 1 then
                    self.hide_details(self)
                    return 0
                end
                -- Armor materia
                local armor_id = Characters[self.char_id].armor.id
                local armor = Game.Items[armor_id]
                if #(armor.slots) < self.header_position - 1 or Characters[self.char_id].armor.materia[self.header_position - 2] == nil then
                    -- No slot or no materia in slot. Clear and exit
                    self.hide_details(self)
                    return 0
                end
                materia_selected = Characters[self.char_id].armor.materia[self.header_position - 2]
                id = Characters[self.char_id].armor.materia[self.header_position - 2].id
                ap = Characters[self.char_id].armor.materia[self.header_position - 2].ap
                materia = Game.Materia[id]
            else
                -- This should not happen, but just in case.
                self.hide_details(self)
                return 0
            end
        else
            --Get materia data from list.
            local mat_position = self.list_first_visible + self.list_position - 1
            if Materia[mat_position] == nil then
                self.hide_details(self)
                return 0
            end
            materia_selected = Materia[mat_position]
            id = Materia[mat_position].id
            ap = Materia[mat_position].ap
            materia = Game.Materia[Materia[mat_position].id]
        end

        -- Materia should be set, but just in case, one last check.
        if materia == nil then
            self.hide_details(self)
            return
        end

        -- Check for enemy skill. It is handled by another function.
        if id == Materia.ENEMY_SKILL_ID then
            self.populate_e_skill_details(self, materia_selected)
            return 0
        end

        ui_manager:get_widget("MateriaMenu.Container.Details.Empty"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.Details.Commands"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.Details.EnemySkill"):set_visible(false)
        ui_manager:get_widget("MateriaMenu.Container.Details.Materia"):set_visible(true)
        ui_manager:get_widget("MateriaMenu.Container.Help.Help"):set_text(Game.Materia[id].description)

        -- Get level and max level, and check for master.
        level = 0
        for lv, lv_ap in ipairs(materia.levels_ap) do
            if lv_ap > ap then
                break
            end
            level = level + 1
        end
        max_level = #(materia.levels_ap)
        if level == max_level then
            master = true
            ap_next = 0
        else
            ap_next = materia.levels_ap[level + 1] - ap
        end
        -- Start displaying data
        ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Icon"):set_image("icons/materia_" .. tostring(materia.type) .. ".png")
        ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Name"):set_text(materia.name)
        for l = 1, 5 do
            if l > max_level then
                -- No such level, hide star.
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Stars.Star" .. tostring(l)):set_visible(false)
            else
                if l <= level then
                    ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Stars.Star" .. tostring(l)):set_image("icons/materia_star_" .. tostring(materia.type) .. ".png")
                else
                    ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Stars.Star" .. tostring(l)):set_image("icons/materia_star_empty_" .. tostring(materia.type) .. ".png")
                end
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Stars.Star" .. tostring(l)):set_visible(true)
            end
        end
        ui_manager:get_widget("MateriaMenu.Container.Details.Materia.AP"):set_text(UiContainer.pad_value(ap, 6))
        ui_manager:get_widget("MateriaMenu.Container.Details.Materia.APNext"):set_text(UiContainer.pad_value(ap_next, 6))
        --ui_manager:get_widget("MateriaMenu.Container.Details.Materia.AP"):set_text(UiContainer.pad_value(200000, 6))
        --ui_manager:get_widget("MateriaMenu.Container.Details.Materia.APNext"):set_text(UiContainer.pad_value(30000, 6))

        -- Hide all attacks
        for a = 1, 5 do
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(a)):set_visible(false)
        end

        -- Show attacks, vary by type
        if materia.type == Materia.TYPE.MAGIC then
            local attack_index = 1
            for attack_level, attack_id in pairs(materia.magic) do
                if level >= attack_level then
                    ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_colour(1, 1, 1)
                else
                    ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_colour(0.6, 0.6, 0.6)
                end
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_text(Game.Attacks[attack_id].name)
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_visible(true)
                attack_index = attack_index + 1
            end
        elseif materia.type == Materia.TYPE.SUMMON then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_colour(1, 1, 1)
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_text(Game.Attacks[materia.summon].name)
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_visible(true)
        elseif materia.type == Materia.TYPE.COMMAND then
            local attack_index = 1
            for attack_level, attack_id in ipairs(materia.command) do
                if level >= attack_level then
                    ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_colour(1, 1, 1)
                else
                    ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_colour(0.6, 0.6, 0.6)
                end
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_text(Game.Commands[attack_id].name)
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_visible(true)
                attack_index = attack_index + 1
            end
        elseif materia.type == Materia.TYPE.INDEPENDENT then
            local bonus = 0
            for l = 1, max_level do
                if l <= level and materia.ability.param[l] ~= nil then
                    bonus = materia.ability.param[l]
                end
            end
            if materia.ability ~= nil then
                local text = "TEXT"
                if materia.ability.id == Materia.ABILITY.MP_PLUS then
                    text = "<include name=\"StatMP\"/> +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.HP_PLUS then
                    text = "<include name=\"StatHP\"/> +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.STR_PLUS then
                    text = "<include name=\"StatStr\"/> +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.VIT_PLUS then
                    text = "<include name=\"StatVit\"/> +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.MAG_PLUS then
                    text = "<include name=\"StatMag\"/> +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.DEX_PLUS then
                    text = "<include name=\"StatDex\"/> +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.LCK_PLUS then
                    text = "<include name=\"StatLck\"/> +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.EXP_PLUS then
                    text = "<include name=\"StatEXP\"/> +"
                elseif materia.ability.id == Materia.ABILITY.MONEY_PLUS then
                    text = "<include name=\"Currency\"/> +"
                elseif materia.ability.id == Materia.ABILITY.ENCOUNTER_DOWN then
                    text = "<include name=\"MateriaEncounters\"/> -"
                elseif materia.ability.id == Materia.ABILITY.ENCOUNTER_UP then
                    text = "<include name=\"MateriaEncountersChocobo\"/> -"
                elseif materia.ability.id == Materia.ABILITY.PREEMPTIVE then
                    text = materia.name
                elseif materia.ability.id == Materia.ABILITY.LONG_RANGE then
                    text = materia.name
                elseif materia.ability.id == Materia.ABILITY.MEGA_ALL then
                    text = materia.name
                elseif materia.ability.id == Materia.ABILITY.COUNTER_ATTACK then
                    text = materia.name
                elseif materia.ability.id == Materia.ABILITY.COVER then
                    text = materia.name .. " +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.UNDERWATER then
                    text = materia.name
                elseif materia.ability.id == Materia.ABILITY.HP_MP then
                    text = materia.name
                end
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_colour(1, 1, 1)
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_text(text)
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_visible(true)
            end
        elseif materia.type == Materia.TYPE.SUPPORT then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_colour(1, 1, 1)
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_text(materia.name)
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_visible(true)
        end

        -- Hide all effects
        for a = 1, 7 do
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(a)):set_visible(false)
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(a)):set_visible(false)
        end

        -- Show effects, if any.
        local effect_index = 1
        if materia.stats.str ~= nil and materia.stats.str ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatStr\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.str < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.str, 2) .. "</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.str, 2) .. "</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
        if materia.stats.vit ~= nil and materia.stats.vit ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatVit\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.vit < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.vit, 2) .. "</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.vit, 2) .. "</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
        if materia.stats.mag ~= nil and materia.stats.mag ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatMag\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.mag < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.mag, 2) .. "</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.mag, 2) .. "</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
        if materia.stats.spr ~= nil and materia.stats.spr ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatSpr\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.spr < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.spr, 2) .. "</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.spr, 2) .. "</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
        if materia.stats.dex ~= nil and materia.stats.dex ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatDex\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.dex < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.dex, 2) .. "</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.dex, 2) .. "</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
        if materia.stats.lck ~= nil and materia.stats.lck ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatLck\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.lck < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.lck, 2) .. "</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.lck, 2) .. "</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
        if materia.stats.hp ~= nil and materia.stats.hp ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatHP\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.hp < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.hp, 2) .. "%</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.hp, 2) .. "%</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
        if materia.stats.mp ~= nil and materia.stats.mp ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatMP\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.mp < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.mp, 2) .. "%</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.mp, 2) .. "%</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
    end,

    --- Populates the materia list.
    populate_list = function(self)
        local pos = 1
        for i = self.list_first_visible, self.list_first_visible + self.list_position_total - 1 do
            if Materia[i] ~= nil then
                local type = Game.Materia[Materia[i].id].type
                local name = Game.Materia[Materia[i].id].name
                ui_manager:get_widget("MateriaMenu.Container.List.Icon" .. tostring(pos)):set_image("icons/materia_" .. tostring(type) .. ".png")
                ui_manager:get_widget("MateriaMenu.Container.List.Icon" .. tostring(pos)):set_visible(true)
                ui_manager:get_widget("MateriaMenu.Container.List.Name" .. tostring(pos)):set_text(name)
                ui_manager:get_widget("MateriaMenu.Container.List.Name" .. tostring(pos)):set_visible(true)
            else
                ui_manager:get_widget("MateriaMenu.Container.List.Icon" .. tostring(pos)):set_visible(false)
                ui_manager:get_widget("MateriaMenu.Container.List.Name" .. tostring(pos)):set_visible(false)
            end
            pos = pos + 1
        end
         -- Set scroll bar position
        local scroll = ui_manager:get_widget("MateriaMenu.Container.List.Scroll")
        -- first_item_in_window == 1 then Top -> 0%3
        -- first_item_in_window == inventory_size - first_item_in_window then top = 100%-21
        local percent = math.floor((self.list_first_visible - 1) * 100 / (self.list_size - self.list_position_total))
        local fixed = 3 + (-1 * math.floor(self.list_first_visible - 1) * 25 / (self.list_size - self.list_position_total))
        scroll:set_y(percent, fixed)
    end,

    --- Equips the materia selected in the list at the currently selected slot.
    --
    -- It can also be used to swap or remove materia.
    equip_materia = function(self)
        local equip
        local equip_slot = self.header_position - 2 -- 0-8
        local list_pos = self.list_first_visible + self.list_position - 1
        local materia_equip = {id = nil, ap = 0}
        local materia_list = {id = nil, ap = 0}
        --- Get info about the seleceted materias (equipped and in the list)
        if self.header_position_row == 1 then
            equip = Inventory.ITEM_TYPE.WEAPON
            if equip_slot + 1 > #(Game.Items[Characters[self.char_id].weapon.id].slots) then
                print("BEEP No slot " .. tostring(equip_slot) .. " in current weapon.")
                return 0
            end
            if Characters[self.char_id].weapon.materia[equip_slot] ~= nil then
                materia_equip = Characters[self.char_id].weapon.materia[equip_slot]
            end
        elseif self.header_position_row == 2 then
            equip = Inventory.ITEM_TYPE.ARMOR
            if equip_slot + 1 > #(Game.Items[Characters[self.char_id].armor.id].slots) then
                print("BEEP No slot " .. tostring(equip_slot) .. " in current armor.")
                return 0
            end
            if Characters[self.char_id].armor.materia[equip_slot] ~= nil then
                materia_equip = Characters[self.char_id].armor.materia[equip_slot]
            end
        end
        if Materia[list_pos] ~= nil then
            materia_list = Materia[list_pos]
        end
        --- Equipped materia to the list (can be null)
        Materia[list_pos] = materia_equip
        if Materia[list_pos].id == nil then
            Materia[list_pos] = nil
        end
        --- Materia from list to weapon or armor
        if equip == Inventory.ITEM_TYPE.WEAPON then
            Characters[self.char_id].weapon.materia[equip_slot] = materia_list
            if Characters[self.char_id].weapon.materia[equip_slot].id == nil then
                Characters[self.char_id].weapon.materia[equip_slot] = nil
            end
        elseif equip == Inventory.ITEM_TYPE.ARMOR then
            Characters[self.char_id].armor.materia[equip_slot] = materia_list
            if Characters[self.char_id].armor.materia[equip_slot].id == nil then
                Characters[self.char_id].armor.materia[equip_slot] = nil
            end
        end
        -- The materia change the stats, and the current HP and MP can't be over the new maximums.
        Characters.recalculate_stats(self.char_id)
        -- Reload character data and list.
        self.populate_equip(self)
        self.populate_list(self)
        self.submenu_none(self)
    end,

    --- Hides the item menu and goes back to the main menu.
    hide = function(self)
        ui_manager:get_widget("MateriaMenu"):set_visible(false)
        UiContainer.current_menu = "main"
        UiContainer.current_submenu = ""
        UiContainer.resume_menu("main")
        return 0;
    end,
}
