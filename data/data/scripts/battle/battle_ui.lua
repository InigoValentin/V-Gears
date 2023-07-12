if UiContainer == nil then UiContainer = {} end

--- The main menu.
UiContainer.BattleUi = {

    --- Party members
    party = {},

    --- Enemies
    enemies = {},

    --- Run when the menu is creaded.
    --
    -- It does nothing.
    on_start = function(self)
        return 0
    end,

    --- Indicates what the current cursor is for.
    --
    -- Possible values are "wait" (no cursor), "command" (for command selection), "list" (for
    -- magic, item, summon... selection), and "target" for target selection.
    window_state = "wait", -- wait, command, list, target

    --- Current position for the cursor in the command window.
    cursor_command = 1,

    --- Current position for the cursor in the list window.
    cursor_list = 1,

    --- Current position for the cursor in the target window.
    cursor_target = 1,

    --- ID of the character the cursor is for.
    current_character_window = -1,

    --- Handles button events.
    --
    -- For the current submenu, handles directional keys, enter and escape events.
    -- @param button Pressed button string key. "Up", "Left", "Enter" and "Escape" are handled.
    -- @param event Trigger event. Normally, "Press".
    on_button = function(self, button, event)
        if self.active == false then
            do return 0 end
        end
        if button == "K" then
            Battle.finish()
        end
        if self.window_state == "command" then
            local command_count = #self.party[self.current_character_window].command
            if button == "Down" then
                if self.cursor_command < 4 then
                    -- If on the first three ones, always alow to go down to reach "Item"
                    self.cursor_command = self.cursor_command + 1
                    ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_default_animation("Position" .. tostring(self.cursor_command))
                elseif self.cursor_command % 4 == 0 then
                    -- If on a multiple of 4, go to the top of the column
                    self.cursor_command = self.cursor_command - 3
                    ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_default_animation("Position" .. tostring(self.cursor_command))
                elseif command_count > self.cursor_command then
                    -- Else, allow one position down only if there are more comands.
                    self.cursor_command = self.cursor_command + 1
                    ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_default_animation("Position" .. tostring(self.cursor_command))
                end
            elseif button == "Up" then
                if self.cursor_command == 1 then
                    -- If on "Attack, always go to item
                    self.cursor_command = 4
                    ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_default_animation("Position" .. tostring(self.cursor_command))
                elseif self.cursor_command > 1 and self.cursor_command < 5 then
                    -- If on the second, third, or fourth, always alwow to go down to reach "Attack" from "Item"
                    self.cursor_command = self.cursor_command - 1
                    ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_default_animation("Position" .. tostring(self.cursor_command))
                elseif self.cursor_command % 4 == 1 and command_count > self.cursor_command then
                    -- If on the first row, go to the last where there is a command
                    self.cursor_command = self.cursor_command + 3
                    while self.cursor_command > command_count do
                        self.cursor_command = self.cursor_command - 1
                    end
                    ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_default_animation("Position" .. tostring(self.cursor_command))
                else
                    -- Else, always allow to go one up.
                    self.cursor_command = self.cursor_command - 1
                    ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_default_animation("Position" .. tostring(self.cursor_command))
                end
            elseif button == "Left" then
                if command_count > 4 then
                    if self.cursor_command > 4 then
                        self.cursor_command = self.cursor_command - 4
                        ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_default_animation("Position" .. tostring(self.cursor_command))
                    else
                        local new_position = self.cursor_command + 4 * math.floor(command_count / 4)
                        while new_position > command_count do
                            new_position = new_position - 4
                        end
                        self.cursor_command = new_position
                        ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_default_animation("Position" .. tostring(self.cursor_command))
                    end
                end
            elseif button == "Right" then
                if command_count > 4 then
                    self.cursor_command = self.cursor_command + 4
                    if self.cursor_command > command_count then
                        self.cursor_command = self.cursor_command - 4 * math.floor(self.cursor_command / 4)
                        if self.cursor_command < 1 then
                            self.cursor_command = self.cursor_command + 4
                        end
                    end
                    ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_default_animation("Position" .. tostring(self.cursor_command))
                end
            -- TODO: elseif accept, skip buttons...
            end
        end
        return 0
    end,

    load_party_members = function(self)
        for i = 1, Party.MAX do
            if Party[i] ~= nil then
                self.party[i] = {}
                self.party[i].character = Party[i]
                --math.randomseed(('' .. os.time()):reverse())
                -- TODO: Consider battle layouts.
                -- TODO: Check random seed
                self.party[i].atb = math.random(0, 100)
                self.party[i].command = {}
                self.party[i].magic = {}
                self.party[i].summon = {}
                self.party[i].cover = 0
                self.party[i].counter = 0
                self.party[i].long_range = false
                self.party[i].mega_all = false
                self.party[i].pre_emptive = 0
                local w_magic = false
                local w_item = false
                local w_summon = false
                for e = 1, 2 do -- Loop for weapon, armor
                    local equip = nil
                    if e == 1 then
                        equip = Characters[Party[i]].weapon
                    else
                        equip = Characters[Party[i]].armor
                    end
                    for m = 1, 8 do -- lop for materia in weapon/armor
                        if equip.materia[m - 1] ~= nil then
                            local materia = Game.Materia[equip.materia[m - 1].id]
                            local ap = equip.materia[m - 1].ap
                            local level = 0
                            for l = 1, #materia.levels_ap do
                                if materia.levels_ap[l] <= ap then
                                    level = l
                                end
                            end
                            if materia.type == 5 then -- Summon
                                summon = {
                                    id = materia.summon,
                                    times = materia.times[level],
                                    quadra = 0,
                                    mp_turbo = 0,
                                    mp_absorb = 0,
                                    hp_absorb = 0,
                                    sneak_attack = 0,
                                    final_attack = 0,
                                    added_cut = false,
                                    steal_as_well = false
                                }
                                -- TODO: Check if already inserted.
                                self.party[i].summon[#self.party[i].summon + 1] = summon
                            elseif materia.type == 1 then -- Magic
                                for lv, spell in ipairs(materia.magic) do
                                    if level >= lv then
                                        magic = {
                                            id = spell,
                                            all = 0,
                                            quadra = 0,
                                            mp_turbo = 0,
                                            mp_absorb = 0,
                                            hp_absorb = 0,
                                            sneak_attack = 0,
                                            final_attack = 0,
                                            added_cut = false,
                                            steal_as_well = false
                                        }
                                        -- TODO: Check if already inserted.
                                        self.party[i].magic[#self.party[i].magic + 1] = magic
                                    end
                                end
                            elseif materia.type == 3 then -- Command
                                -- Pick only the highest level command available
                                local command_id = nil
                                for lv, cmd in ipairs(materia.command) do
                                    if level >= lv then
                                        command_id = cmd
                                    end
                                end
                                if command_id ~= nil then
                                    if
                                      command_id ~= 1 and command_id ~= 20 and command_id ~= 24
                                      and command_id ~= 25 and command_id ~= 26
                                      and command_id ~= 26 and #self.party[i].command + 1 == 1
                                    then
                                        -- Leave command index 1 available for atgtack and substitutes
                                        -- 1: Attack, 20: Limit, 24: Slash-all, 25: 2x-Cut, 26: Flash, 27: 4x-Cut
                                        self.party[i].command[#self.party[i].command + 2] = command_id
                                    elseif command_id ~= 23 and #self.party[i].command + 1 == 4 then
                                        -- Leave command index 4 available for item
                                        self.party[i].command[#self.party[i].command + 2] = command_id
                                    else
                                        self.party[i].command[#self.party[i].command + 1] = command_id
                                    end
                                end
                            end
                            -- TODO: Independent materia
                            -- TODO: Linked materia
                        end
                    end
                end
                -- Set default "Attack" command.
                if self.party[i].command[1] == nil then
                    self.party[i].command[1] = 1
                end
                -- Command in slot 4 is reserved for "Item" or "W-Item"
                if self.party[i].command[4] == nil then
                    self.party[i].command[4] = 4
                end
                -- Enable Magic command if any magic is available and W-Magic is disabled.
                if #self.party[i].magic > 0 then
                    local w = false
                    for x = 1, #self.party[i].command do
                        if self.party[i].command[x] == 21 then
                            w = true
                        end
                    end
                    if w == false then
                        if #self.party[i].command + 1 == 1 or #self.party[i].command + 1 == 4 then
                            -- Leave command index 1 available for attack or substitutes, and
                            -- index 4 available for item
                            self.party[i].command[#self.party[i].command + 2] = 2
                        else
                            self.party[i].command[#self.party[i].command + 1] = 2
                        end
                    end
                end
                -- Enable Summon command if any magic is available and W-Summon is disabled.
                if #self.party[i].summon > 0 then
                    local w = false
                    for x = 1, #self.party[i].command do
                        if self.party[i].command[x] == 22 then
                            w = true
                        end
                    end
                    if w == false then
                        if #self.party[i].command + 1 == 1 or #self.party[i].command + 1 == 4 then
                            -- Leave command index 1 available for attack or substitutes, and
                            -- index 4 available for item
                            self.party[i].command[#self.party[i].command + 2] = 3
                        else
                            self.party[i].command[#self.party[i].command + 1] = 3
                        end
                    end
                end
                -- TODO: Sort commands/magic/summons
            end
        end
    end,

    --- Shows the battle UI.
    --
    -- Populates and updates displayed data.
    show = function(self)
        self:populate()
        ui_manager:get_widget("BattleUi"):set_visible(true)
        self.active = true;

        -- Set help window transparency
        ui_manager:get_widget("BattleUi.Container.Help.Center"):set_alpha(0.5)
        ui_manager:get_widget("BattleUi.Container.Message.Center"):set_alpha(0.5)

        -- Lopad party data
        self:load_party_members()

        -- DEBUG:
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Center"):set_alpha(0.5)
        self:show_commands(1)

        return 0;
    end,

    ---
    -- Shows the command window for a character
    -- @param party_id The ID of the party member whose commands to show
    show_commands = function(self, party_id)
        self.current_character_window = party_id
        self.cursor_command = 1
        -- Colour the character name
        for c = 1, #self.party do
            if c == party_id then
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Left.Name"):set_colour(1, 1, 0)
            else
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Left.Name"):set_colour(1, 1, 1)
            end
            ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Left.Name"):set_text(Characters[Party[c]].name)
        end
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands"):set_visible(true)
        -- Resize the window
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands"):set_width(
          40 + 40 * (1 + math.floor(#self.party[party_id].command / 4))
        )
        -- Show command names
        for c = 1, 12 do
            ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cmd" .. tostring(c)):set_visible(false)
        end
        for pos, cmd in ipairs(self.party[party_id].command) do
            ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cmd" .. tostring(pos)):set_text(Game.Commands[cmd].name)
            ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cmd" .. tostring(pos)):set_visible(true)
        end
        -- Set and show cursor
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_visible(true)
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_default_animation("Position" .. tostring(self.cursor_command))
        self.window_state = "command"

    end,

    --- Hides the battle UI.
    --
    -- It also sets this widget to inactive.
    hide = function(self)

        ui_manager:get_widget("BattleUi"):set_visible(false)
        self.active = false

        --MenuSettings.pause_available = true
        --entity_manager:set_paused(false)

        return 0;
    end,


    populate = function(self)

        space_pad = function(val, length)
            str = tostring(val)
            while #(str) < length do
                str = " " .. str
            end
            return str
        end

        for c = 1, 3 do
            if Party[c] ~= nil then
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c)):set_visible(true)
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Left.Name"):set_text(Characters[Party[c]].name)
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Right.Hp.HpCurrent"):set_text(space_pad(Characters[Party[c]].stats.hp.current, 5))
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Right.Hp.HpMax"):set_text(space_pad(Characters[Party[c]].stats.hp.base, 5))
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Right.Mp.MpCurrent"):set_text(space_pad(Characters[Party[c]].stats.mp.current, 5))
            else
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c)):set_visible(false)
            end
        end
        return 0
    end
}
