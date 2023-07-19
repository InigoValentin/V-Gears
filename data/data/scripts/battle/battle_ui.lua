if UiContainer == nil then UiContainer = {} end

--- The battle UI
--
-- Controls the battle UI and the battle flow
-- TODO: Merge this code into Battle
UiContainer.BattleUi = {

    --- Party members
    --
    --
    party = {},

    --- Enemies
    enemies = {},

    --- Battle action queue.
    --
    -- Every battle action gets added to this queue, sorted by priority, and the actions are
    -- executed in order. Actions for enemies and allies are stored here.
    action_queue = {},

    --- Determines if an action is being executed.
    action_executing = false,

    --- Party command selection queue.
    --
    -- When a party member ATB is filled, the character is added to this queue. Command selection
    -- windows are show in this order. If a character is dismissed (triangle in PSX), the character
    -- gets moved to the end of the queue.
    command_queue = {},

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

    --- First visible item in the list window
    first_list = 1,

    --- Current horizontal position for the cursor in the target window.
    cursor_target_h = 1,

    --- Current vertical position for the cursor in the target window.
    cursor_target_v = 1,

    --- First visible enemy in the target window
    first_target = 1,

    --- ID of the character the cursor is for.
    current_character_window = -1,

    --- Indicates the currently opened menu
    --
    -- Possible values are none, magic, wmagic1, wmagic2, summon, wsummon1, wsummon2, item, witem1,
    -- witem2, eskill, throw, limit, coin.
    current_list = "none",

    --- Target options of the currently selected attack
    target_options = {},

    --- Target mode.
    target_type = Battle.TargetType.SINGLE,

    --- The targeted group.
    target_group = Battle.TargetGroup.ENEMIES,

    --- The targeted unit index in the targeted group.
    target_index = 1,


    --- Handles button events.
    --
    -- For the current submenu, handles directional keys, enter and escape events.
    -- @param button Pressed button string key. "Up", "Left", "Enter" and "Escape" are handled.
    -- @param event Trigger event. Normally, "Press".
    on_button = function(self, button, event)
        if self.active == false then do return 0 end end
        if button == "K" then Battle.finish() end
        -- Handle command cursors
        if self.window_state == "command" then
            local command_count = #self.party[self.current_character_window].command
            local cursor = ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor")
            if button == "Down" then
                if self.cursor_command < 4 then
                    -- If on the first three ones, always alow to go down to reach "Item"
                    self.cursor_command = self.cursor_command + 1
                    cursor:set_default_animation("Position" .. tostring(self.cursor_command))
                    audio_manager:play_sound("Cursor")
                elseif self.cursor_command % 4 == 0 then
                    -- If on a multiple of 4, go to the top of the column
                    self.cursor_command = self.cursor_command - 3
                    cursor:set_default_animation("Position" .. tostring(self.cursor_command))
                    audio_manager:play_sound("Cursor")
                elseif command_count > self.cursor_command then
                    -- If there is a command below, allow one position down.
                    self.cursor_command = self.cursor_command + 1
                    cursor:set_default_animation("Position" .. tostring(self.cursor_command))
                    audio_manager:play_sound("Cursor")
                else
                    -- Back to the first row
                    self.cursor_command = 4 * math.floor(self.cursor_command / 4) + 1
                    cursor:set_default_animation("Position" .. tostring(self.cursor_command))
                    audio_manager:play_sound("Cursor")
                end
            elseif button == "Up" then
                if self.cursor_command == 1 then
                    -- If on "Attack, always go to item
                    self.cursor_command = 4
                    cursor:set_default_animation("Position" .. tostring(self.cursor_command))
                    audio_manager:play_sound("Cursor")
                elseif self.cursor_command > 1 and self.cursor_command < 5 then
                    -- If on the second, third, or fourth, always alwow to go down to
                    -- reach "Attack" from "Item"
                    self.cursor_command = self.cursor_command - 1
                    cursor:set_default_animation("Position" .. tostring(self.cursor_command))
                    audio_manager:play_sound("Cursor")
                elseif self.cursor_command % 4 == 1 and command_count > self.cursor_command then
                    -- If on the first row, go to the last where there is a command
                    self.cursor_command = self.cursor_command + 3
                    while self.cursor_command > command_count do self.cursor_command = self.cursor_command - 1 end
                    cursor:set_default_animation("Position" .. tostring(self.cursor_command))
                    audio_manager:play_sound("Cursor")
                else
                    -- Else, always allow to go one up.
                    self.cursor_command = self.cursor_command - 1
                    cursor:set_default_animation("Position" .. tostring(self.cursor_command))
                    audio_manager:play_sound("Cursor")
                end
            elseif button == "Left" then
                if command_count > 4 then
                    if self.cursor_command > 4 then
                        self.cursor_command = self.cursor_command - 4
                        cursor:set_default_animation("Position" .. tostring(self.cursor_command))
                        audio_manager:play_sound("Cursor")
                    else
                        local new_pos = self.cursor_command + 4 * math.floor(command_count / 4)
                        while new_pos > command_count do new_pos = new_pos - 4 end
                        self.cursor_command = new_pos
                        cursor:set_default_animation("Position" .. tostring(self.cursor_command))
                        audio_manager:play_sound("Cursor")
                    end
                end
            elseif button == "Right" then
                if command_count > 4 then
                    self.cursor_command = self.cursor_command + 4
                    if self.cursor_command > command_count then
                        self.cursor_command = self.cursor_command - 4 * math.floor(self.cursor_command / 4)
                        if self.cursor_command < 1 then self.cursor_command = self.cursor_command + 4 end
                    end
                    cursor:set_default_animation("Position" .. tostring(self.cursor_command))
                    audio_manager:play_sound("Cursor")
                end
            elseif button == "Escape" then
                -- Push the current character to the end of the command queue, and show the first.
                local new_queue = {}
                for i = 1, #self.command_queue do
                    if self.command_queue[i + 1] ~= nil then table.insert(new_queue, self.command_queue[i + 1]) end
                end
                table.insert(new_queue, self.command_queue[1])
                self.command_queue = new_queue
                audio_manager:play_sound("Cursor")
                self:show_commands(self.command_queue[1])
            elseif button == "Enter" then
                -- Check if a menu must be shown
                local cmd = Game.Commands[self.party[self.current_character_window].command[self.cursor_command]]
                if cmd.menu == Battle.Menu.NONE then print("DIRECT COMMAND")
                elseif cmd.menu == Battle.Menu.TARGET then print("TARGET COMMAND")
                else self:show_list(cmd.menu)
                end
            end
        -- Handle menu list cursor
        elseif self.window_state == "list" then
            local total = 0
            if self.current_list == "magic" or self.current_list == "wmagic1" or self.current_list == "wmagic2" then
                total = #self.party[self.current_character_window].magic
            elseif self.current_list == "summon" or self.current_list == "wsummon1" or self.current_list == "wsummon2"
            then total = #self.party[self.current_character_window].summon
            end
            -- TODO: Rest of cases
            local cursor = ui_manager:get_widget("BattleUi.Container.Bottom.List.Cursor")
            if button == "LCtrl" then
                audio_manager:play_sound("Cursor")
                self:close_list()
            elseif button == "Down" then
                if total <=  self.cursor_list + self.first_list - 1 then do return 0 end end
                if self.cursor_list < 4 then
                    -- Move cursor down
                    audio_manager:play_sound("Cursor")
                    self.cursor_list = self.cursor_list + 1
                    cursor:set_default_animation("Position" .. tostring(self.cursor_list))
                else
                    -- Scroll down
                    audio_manager:play_sound("Cursor")
                    self.first_list = self.first_list + 1
                    self:repopulate_list()
                end
            elseif button == "Up" then
                if self.cursor_list == 1 and self.first_list == 1 then do return 0 end end
                if self.cursor_list > 1 then
                    -- Move cursor up
                    audio_manager:play_sound("Cursor")
                    self.cursor_list = self.cursor_list - 1
                    cursor:set_default_animation("Position" .. tostring(self.cursor_list))
                else
                    -- Scroll up
                    audio_manager:play_sound("Cursor")
                    self.first_list = self.first_list - 1
                    self:repopulate_list()
                end
            elseif button == "Enter" then
                audio_manager:play_sound("Cursor")
                self:select_target()
            end
        elseif self.window_state == "target" then
            if button == "LCtrl" then
                audio_manager:play_sound("Cursor")
                self:close_target()
            elseif button == "Up" then
                if self.cursor_target_h == 2 then
                    -- Allies cursor, never scroll. Go un up or to the bottom
                    self.cursor_target_v = self.cursor_target_v - 1
                    if self.cursor_target_v == 0 then self.cursor_target_v = #self.party end
                    audio_manager:play_sound("Cursor")
                    self:populate_target_list()
                else
                    -- Cursor on the left. Can be enemies or allies, depends on the options.
                    if self.target_options.fixed_row == 1 and self.target_options.default_enemy == 0 then
                        -- Allies
                        self.cursor_target_v = self.cursor_target_v - 1
                        if self.cursor_target_v == 0 then self.cursor_target_v = #self.party end
                        audio_manager:play_sound("Cursor")
                        self:populate_target_list()
                    else
                        -- Enemies
                        if #self.enemies > 4 then
                            if self.cursor_target_v == 1 then
                                if self.first_target == 1 then
                                    -- Scroll to the bottom, set  cursor in last
                                    self.first_target = #self.enemies - 4
                                    self.cursor_target_v = 4
                                    audio_manager:play_sound("Cursor")
                                    self:populate_target_list()
                                else
                                    self.first_target = self.first_target - 1
                                    audio_manager:play_sound("Cursor")
                                    self:populate_target_list()
                                end
                            else
                                self.cursor_target_v = self.cursor_target_v - 1
                                audio_manager:play_sound("Cursor")
                                self:populate_target_list()
                            end
                        else
                            self.cursor_target_v = self.cursor_target_v - 1
                            if self.cursor_target_v == 0 then self.cursor_target_v = #self.enemies end
                            audio_manager:play_sound("Cursor")
                            self:populate_target_list()
                        end
                    end
                end
            elseif button == "Down" then
                if self.cursor_target_h == 2 then
                    -- Allies cursor, never scroll. Go un up or to the bottom
                    self.cursor_target_v = self.cursor_target_v + 1
                    if self.cursor_target_v > #self.party then self.cursor_target_v = 1 end
                    audio_manager:play_sound("Cursor")
                    self:populate_target_list()
                else
                    -- Cursor on the left. Can be enemies or allies, depends on the options.
                    if self.target_options.fixed_row == 1 and self.target_options.default_enemy == 0 then
                        -- Allies
                        self.cursor_target_v = self.cursor_target_v + 1
                        if self.cursor_target_v > #self.party then self.cursor_target_v = 1 end
                        audio_manager:play_sound("Cursor")
                        self:populate_target_list()
                    else
                        -- Enemies
                        if #self.enemies > 4 then
                            if self.cursor_target_v == 4 then
                                if self.first_target + 4 == #self.enemies then
                                    -- Scroll to the top, set  cursor in first
                                    self.first_target = 1
                                    self.cursor_target_v = 1
                                    audio_manager:play_sound("Cursor")
                                    self:populate_target_list()
                                else
                                    self.first_target = self.first_target + 1
                                    audio_manager:play_sound("Cursor")
                                    self:populate_target_list()
                                end
                            else
                                self.cursor_target_v = self.cursor_target_v + 1
                                audio_manager:play_sound("Cursor")
                                self:populate_target_list()
                            end
                        else
                            self.cursor_target_v = self.cursor_target_v + 1
                            if self.cursor_target_v > #self.enemies then self.cursor_target_v = 1 end
                            audio_manager:play_sound("Cursor")
                            self:populate_target_list()
                        end
                    end
                end
            elseif button == "Left" then
                if self.target_options.fixed_row == 0 and self.target_options.toggle_multiple == 0 then
                    -- If both rows are present and multiple toggling is not available...
                    if self.cursor_target_h == 2 then
                        -- and the cursor is in the second column, go to the fisrt. Also, move the
                        --cursor up until a valid enemy is pointed.
                        self.cursor_target_h = 1
                        while self.cursor_target_v + self.first_target - 1 > #self.enemies do self.cursor_target_v = self.cursor_target_v - 1 end
                        audio_manager:play_sound("Cursor")
                        self:populate_target_list()
                    end
                elseif self.target_options.fixed_row == 0 and self.target_options.toggle_multiple == 1 then
                    -- If both rows are present, and multiple targets can be toggled...
                    if self.cursor_target_h == 2 and self.target_type == Battle.TargetType.GROUP then
                        -- ... and the cursor is on the rigth, with multiple targets, set to single
                        -- target
                        self.target_type = Battle.TargetType.SINGLE
                        audio_manager:play_sound("Cursor")
                        self:populate_target_list()
                    elseif self.cursor_target_h == 2 and self.target_type == Battle.TargetType.SINGLE then
                        -- ... and the cursor is on the rigth, with single targets, go to the left
                        -- target. Also, move the cursor up until a valid enemy is pointed.
                        self.cursor_target_h = 1
                        while self.cursor_target_v + self.first_target - 1 > #self.enemies do self.cursor_target_v = self.cursor_target_v - 1 end
                        audio_manager:play_sound("Cursor")
                        self:populate_target_list()
                    elseif self.cursor_target_h == 1 and self.target_type == Battle.TargetType.SINGLE then
                        -- ... and the cursor is on the left, with single targets, set to multiple
                        -- targets
                        self.target_type = Battle.TargetType.GROUP
                        audio_manager:play_sound("Cursor")
                        self:populate_target_list()
                    end
                elseif self.target_options.fixed_row == 1 and self.target_options.toggle_multiple == 1 then
                    -- If only one row is present and multiple can be toggled, toggle it
                    local prev_mode = self.target_type
                    if prev_mode == Battle.TargetType.GROUP then self.target_type = Battle.TargetType.SINGLE
                    elseif prev_mode == Battle.TargetType.SINGLE then self.target_type = Battle.TargetType.GROUP
                    end
                end
            elseif button == "Right" then
                if self.target_options.fixed_row == 0 and self.target_options.toggle_multiple == 0 then
                    -- If both rows are present and multiple toggling is not available...
                    if self.cursor_target_h == 1 then
                        -- and the cursor is in the left column, go to the right. Also, move the
                        --cursor up until a valid party member is pointed.
                        self.cursor_target_h = 2
                        while self.cursor_target_v + self.first_target - 1 > #self.party do self.cursor_target_v = self.cursor_target_v - 1 end
                        audio_manager:play_sound("Cursor")
                        self:populate_target_list()
                    end
                elseif self.target_options.fixed_row == 0 and self.target_options.toggle_multiple == 1 then
                    -- If both rows are present, and multiple targets can be toggled...
                    if self.cursor_target_h == 1 and self.target_type == Battle.TargetType.GROUP then
                        -- ... and the cursor is on the left, on multiple target mode, set to
                        -- single target mode.
                        self.target_type = Battle.TargetType.SINGLE
                        audio_manager:play_sound("Cursor")
                        self:populate_target_list()
                    elseif self.cursor_target_h == 1 and self.target_type == Battle.TargetType.SINGLE then
                        -- ... and the cursor is on the left, on single target mode, go to the
                        -- right. Also, move the cursor up until a valid party member is pointed.
                        self.cursor_target_h = 2
                        while self.cursor_target_v + self.first_target - 1 > #self.party do self.cursor_target_v = self.cursor_target_v - 1 end
                        audio_manager:play_sound("Cursor")
                        self:populate_target_list()
                    elseif self.cursor_target_h == 2 and self.target_type == Battle.TargetType.SINGLE then
                        -- ... and the cursor is on the right, on single target mode, set to
                        -- multiple target mode.
                        self.target_type = Battle.TargetType.GROUP
                        audio_manager:play_sound("Cursor")
                        self:populate_target_list()
                    end
                elseif self.target_options.fixed_row == 1 and self.target_options.toggle_multiple == 1 then
                    -- If only one row is present and multiple can be toggled, toggle it
                    local prev_mode = self.target_type
                    if prev_mode == Battle.TargetType.GROUP then self.target_type = Battle.TargetType.SINGLE
                    elseif prev_mode == Battle.TargetType.SINGLE then self.target_type = Battle.TargetType.GROUP
                    end
                end
            elseif button == "Enter" then
                -- Determine target type and target index
                if self.target_options.fixed_row == 1 then
                    if self.target_options.default_enemy == 1 then
                        self.target_group = Battle.TargetGroup.ENEMIES
                        if self.target_type == Battle.TargetType.SINGLE then self.target_index = self.cursor_target_v + self.first_target - 1
                        else self.target_index = nil
                        end
                    else
                        self.target_group = Battle.TargetGroup.PARTY
                        if self.target_type == Battle.TargetType.SINGLE then self.target_index = self.cursor_target_v
                        else self.target_index = nil
                        end
                    end
                else
                    if self.cursor_target_h == 1 then
                        self.target_group = Battle.TargetGroup.ENEMIES
                        if self.target_type == Battle.TargetType.SINGLE then self.target_index = self.cursor_target_v + self.first_target - 1
                        else self.target_index = nil
                        end
                    else
                        self.target_group = Battle.TargetGroup.PARTY
                        if self.target_type == Battle.TargetType.SINGLE then self.target_index = self.cursor_target_v
                        else self.target_index = nil
                        end
                    end
                end
                -- Add the action to the queue.
                self:queue_character_action()
            end
        end
        return 0
    end,

    --- Repopulates a list
    --
    -- Repopulates the currently shown list with the current cursor and scroll positions.
    repopulate_list = function(self)
        if self.current_list == "magic" or self.current_list == "wmagic1" or self.current_list == "wmagic2" then
            self:populate_magic_list()
        elseif self.current_list == "summon" or self.current_list == "wsummon1" or self.current_list == "wsummon2" then
            self:populate_summon_list()
        -- TODO: all other cases
        end
    end,

    --- Adds a party character action to the queue
    queue_character_action = function(self)
        local action = {}
        action.actor = self.current_character_window
        if self.current_list == "none" then action.type = Battle.ActionType.COMMAND
        else action.type = Battle.ActionType.ATTACK
        end
        action.target_type = self.target_type
        action.target_index = self.target_index
        action.priority = 6 -- TODO: find out
        table.insert(self.action_queue, action)
        -- TODO: Reorder queue by priority.

    end,

    --- Handles a time tick.
    --
    -- Increases the ATBs of party and enemies. If one of them gets filled, it shows tha command
    -- window
    tick = function(self)
        if System:is_paused() == true then do return end end
        -- Advance party ATBs
        for i = 1, #self.party do
            if self.party[i].dead == false then -- TODO: Or stopped
                if self.party[i].atb < 255 then
                    -- TODO: Consider haste/slow
                    self.party[i].atb = self.party[i].atb + 0.04 * Characters[self.party[i].character].stats.dex.total -- TODO
                    if self.party[i].atb >= 255 then
                        print(Characters[self.party[i].character].name .. " ATB full")
                        ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(i) .. ".Right.AtbFill"):set_colour(0, 1, 0)
                        table.insert(self.command_queue, i)
                    end
                    -- Fill ATB UI bar
                    -- 0   -> 0%
                    -- 255 -> 18%-6
                    local width_percent = math.min(18, 18 * self.party[i].atb / 255)
                    local width_fixed = math.max(-6, -6 * width_percent / 18)
                    ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(i) .. ".Right.AtbFill"):set_width(width_percent, width_fixed)
                end
            end
        end
        -- TODO: Advance enemy ATBs
        for i = 1, #self.enemies do
            if self.enemies[i].dead == false then -- TODO: Or stopped
                if self.enemies[i].atb < 255 then
                    -- TODO: Consider haste/slow
                    self.enemies[i].atb = self.enemies[i].atb + 0.04 * self.enemies[i].stats.dex -- TODO
                    if self.enemies[i].atb >= 255 then
                        print("Enemy " .. self.enemies[i].name .. " ATB full")
                        -- TODO: Select an action and add to queue
                        --table.insert(self.command_queue, i)
                    end
                end
            end
        end
        -- Show a command window if required
        if self.window_state == "wait" and #self.command_queue > 0 then self:show_commands(self.command_queue[1]) end
        -- Execute an action if none is being executed
        if self.action_executing == false and #self.action_queue > 0 then
            self:execute_action()
        end
    end,

    --- Executes an action
    --
    -- The top action in action_queue gets executed, and then removed from the queue
    execute_action = function(self)
        if self.action_executing == true or #self.action_queue == 0 then do return 0 end end
        print("EXECUTE ACTION")
    end,

    --- Shows a list of actions.
    --
    -- Can be used for the windows of magic, summons, items...
    -- @param list The list to open, as in {@see Battle.Menu}
    show_list = function(self, list)
        local cursor = ui_manager:get_widget("BattleUi.Container.Bottom.List.Cursor")
        if list == Battle.Menu.MAGIC then
            audio_manager:play_sound("Cursor")
            self.cursor_list = 1
            self.first_list = 1
            self.current_list = "magic"
            self:populate_magic_list()
            self.window_state = "list"
            ui_manager:get_widget("BattleUi.Container.Bottom.List"):set_visible(true)
            cursor:set_default_animation("Position" .. tostring(self.cursor_list))
            cursor:set_visible(true)
        elseif list == Battle.Menu.SUMMON then
            audio_manager:play_sound("Cursor")
            self.cursor_list = 1
            self.first_list = 1
            self.current_list = "summon"
            self:populate_summon_list()
            self.window_state = "list"
            ui_manager:get_widget("BattleUi.Container.Bottom.List"):set_visible(true)
            cursor:set_default_animation("Position" .. tostring(self.cursor_list))
            cursor:set_visible("Position" .. tostring(self.cursor_list))
        elseif (list == Battle.Menu.ITEM) then
            print("Show item list: Not implemented")
        elseif (list == Battle.Menu.E_SKILL) then
            print("Show e. skill list: Not implemented")
        elseif (list == Battle.Menu.THROW) then
            print("Show throw list: Not implemented")
        elseif (list == Battle.Menu.LIMIT) then
            print("Show magic list: Not implemented")
        elseif (list == Battle.Menu.W_MAGIC) then
            print("Show w-magic list: Not implemented")
        elseif (list == Battle.Menu.W_SUMMON) then
            print("Show w-summon list: Not implemented")
        elseif (list == Battle.Menu.W_ITEM) then
            print("Show w-magic list: Not implemented")
        elseif (list == Battle.Menu.COIN) then
            print("Show coin list: Not implemented")
        end
    end,

    --- Closes the currently opened list
    close_list = function(self)
        self.current_list = "none"
        self.window_state = "command"
        ui_manager:get_widget("BattleUi.Container.Bottom.List"):set_visible(false)
    end,

    --- Shows the target selection window.
    --
    -- The elements displayed in the window and the navegability will depend on what is set in
    -- {@see target_options}.
    select_target = function(self)
        local action = nil
        if self.window_state == "list" then
            if self.current_list == "magic" then action = Game.Attacks[self.party[self.current_character_window].magic[self.cursor_list + self.first_list - 1].id] end
        end
        if action == nil then do return end end
        self.target_options = action.target
        if self.target_options.default_multiple == 1 then target_type = Battle.TargetType.GROUP
        else target_type = Battle.TargetType.SINGLE
        end
        self.cursor_target_h = 1
        self.cursor_target_v = 1
        self.first_target = 1
        if self.target_options.default_enemy == 1 then self.cursor_target_h = 1
        else self.cursor_target_h = 2
        end
        -- TODO: if self.target_options.selectable == 0, skip this and directly queue the action.
        -- TODO: Populate options, hide and show right column and separator, and resize window
        -- according to self.target_options
        self:populate_target_list()
        ui_manager:get_widget("BattleUi.Container.Bottom.Target"):set_visible(true)
        self.window_state = "target"
    end,

    --- Closes the target window and activates the previous cursor.
    close_target = function(self)
        if self.current_list == "none" then self.window_state = "command"
        else self.window_state = "list"
        end
        ui_manager:get_widget("BattleUi.Container.Bottom.Target"):set_visible(false)
    end,

    --- Populates the target selection window.
    --
    -- The elements displayed in the window and the navegability will depend on what is set in
    -- {@see target_options}.
    populate_target_list = function(self)
        print("POPULATE TARGET LIST: H: " .. tostring(self.cursor_target_h) .. " V: " .. tostring(self.cursor_target_v) .. " MODE: " .. tostring(self.target_type))
        if self.target_options == nil then do return end end
        if self.target_options.fixed_row == 1 then
            if self.target_options.default_enemy == 1 then
                -- Show only enemies
                print("Enemies only")
            else
                -- Show only allies
                print("Enemies only")
            end
            if self.target_type == Battle.TargetType.GROUP then
                ui_manager:get_widget("BattleUi.Container.Bottom.Target.Cursor"):set_visible(false)
                ui_manager:get_widget("BattleUi.Container.Bottom.Target.CursorAllRight"):set_visible(false)
                ui_manager:get_widget("BattleUi.Container.Bottom.Target.CursorAllLeft"):set_visible(true)
            else
                ui_manager:get_widget("BattleUi.Container.Bottom.Target.Cursor"):set_default_animation("Position" .. tostring(self.cursor_target_h) .. "_" .. tostring(self.cursor_target_v))
                ui_manager:get_widget("BattleUi.Container.Bottom.Target.Cursor"):set_visible(true)
                ui_manager:get_widget("BattleUi.Container.Bottom.Target.CursorAllRight"):set_visible(false)
                ui_manager:get_widget("BattleUi.Container.Bottom.Target.CursorAllLeft"):set_visible(false)
            end
        else
            -- Enemies and allies
            for e = 1, 4 do
                if self.enemies[e + self.first_target - 1] ~= nil then
                    ui_manager:get_widget("BattleUi.Container.Bottom.Target.L" .. tostring(e)):set_text(self.enemies[e + self.first_target - 1].name)
                    ui_manager:get_widget("BattleUi.Container.Bottom.Target.L" .. tostring(e)):set_visible(true)
                else
                    ui_manager:get_widget("BattleUi.Container.Bottom.Target.L" .. tostring(e)):set_visible(false)
                end
            end
            for a = 1, 4 do
                if self.party[a] ~= nil then
                    ui_manager:get_widget("BattleUi.Container.Bottom.Target.R" .. tostring(a)):set_text(Characters[self.party[a].character].name)
                    ui_manager:get_widget("BattleUi.Container.Bottom.Target.R" .. tostring(a)):set_visible(true)
                else
                    ui_manager:get_widget("BattleUi.Container.Bottom.Target.R" .. tostring(a)):set_visible(false)
                end
            end
            if self.target_options.all_rows == 1 or self.target_options.random == 1 then
                ui_manager:get_widget("BattleUi.Container.Bottom.Target.Cursor"):set_visible(false)
                ui_manager:get_widget("BattleUi.Container.Bottom.Target.CursorAllLeft"):set_visible(true)
                ui_manager:get_widget("BattleUi.Container.Bottom.Target.CursorAllRight"):set_visible(true)
            elseif self.target_type == Battle.TargetType.GROUP then
                ui_manager:get_widget("BattleUi.Container.Bottom.Target.Cursor"):set_visible(false)
                if self.cursor_target_h == 1 then
                    ui_manager:get_widget("BattleUi.Container.Bottom.Target.CursorAllLeft"):set_visible(true)
                    ui_manager:get_widget("BattleUi.Container.Bottom.Target.CursorAllRight"):set_visible(false)
                else
                    ui_manager:get_widget("BattleUi.Container.Bottom.Target.CursorAllLeft"):set_visible(false)
                    ui_manager:get_widget("BattleUi.Container.Bottom.Target.CursorAllRight"):set_visible(true)
                end
            else
                ui_manager:get_widget("BattleUi.Container.Bottom.Target.CursorAllLeft"):set_visible(false)
                ui_manager:get_widget("BattleUi.Container.Bottom.Target.CursorAllRight"):set_visible(false)
                ui_manager:get_widget("BattleUi.Container.Bottom.Target.Cursor"):set_visible(true)

                ui_manager:get_widget("BattleUi.Container.Bottom.Target.Cursor"):set_default_animation("Position" .. tostring(self.cursor_target_h) .. "_" .. tostring(self.cursor_target_v))
                ui_manager:get_widget("BattleUi.Container.Bottom.Target.Cursor"):set_visible(true)
            end
        end
    end,

    --- Populates the magic selction window.
    --
    -- It uses the magics the current character can use.
    populate_magic_list = function(self)
        local magic = self.party[self.current_character_window].magic
        for pos = 1, 4 do
            if magic[pos + self.first_list - 1] == nil then
                ui_manager:get_widget("BattleUi.Container.Bottom.List.List" .. tostring(pos)):set_visible(false)
            else
                ui_manager:get_widget("BattleUi.Container.Bottom.List.List" .. tostring(pos)):set_text(Game.Attacks[magic[pos + self.first_list - 1].id].name)
                ui_manager:get_widget("BattleUi.Container.Bottom.List.List" .. tostring(pos)):set_visible(true)
            end
        end
    end,

    --- Populates the summon selction window.
    --
    -- It uses the summons the current character can use.
    populate_summon_list = function(self)
        local summon = self.party[self.current_character_window].summon
        for pos = 1, 4 do
            if summon[pos + self.first_list - 1] == nil then
                ui_manager:get_widget("BattleUi.Container.Bottom.List.List" .. tostring(pos)):set_visible(false)
            else
                ui_manager:get_widget("BattleUi.Container.Bottom.List.List" .. tostring(pos)):set_text(Game.Attacks[summon[pos + self.first_list - 1].id].name)
                ui_manager:get_widget("BattleUi.Container.Bottom.List.List" .. tostring(pos)):set_visible(true)
            end
        end
    end,

    --- Load enemies
    load_enemies = function(self)
        local total = battle_manager:get_enemy_count()
        for e = 0, total - 1 do
            local enemy_src = battle_manager:get_enemy(e)
            enemy = {}
            enemy.dead = false
            enemy.atb = math.random(0, 255)
            enemy.id = enemy_src:get_id()
            enemy.level = enemy_src:get_level()
            enemy.name = enemy_src:get_name()
            enemy.exp = enemy_src:get_exp()
            enemy.ap = enemy_src:get_ap()
            enemy.money = enemy_src:get_money()
            enemy.stats = {}
            enemy.stats.str = enemy_src:get_str();
            enemy.stats.def = enemy_src:get_def();
            enemy.stats.mag = enemy_src:get_mag();
            enemy.stats.spr = enemy_src:get_spr();
            enemy.stats.dex = enemy_src:get_dex();
            enemy.stats.lck = enemy_src:get_lck();
            enemy.stats.eva = enemy_src:get_eva();
            enemy.stats.meva = enemy_src:get_meva();
            enemy.stats.hp = enemy_src:get_hp();
            enemy.stats.hp_max = enemy_src:get_hp_max();
            enemy.stats.mp = enemy_src:get_mp();
            enemy.stats.mp_max = enemy_src:get_mp_max();
            -- TODO: Get more data
            table.insert(self.enemies, enemy)
        end
    end,

    --- Loads party character data
    --
    -- Reads materia, calculates stats, computes attacks...
    -- TODO: Move most of this code to Characters.recalculate_stats()
    load_party_members = function(self)
        for i = 1, Party.MAX do
            if Party[i] ~= nil then
                self.party[i] = {}
                self.party[i].character = Party[i]
                --math.randomseed(('' .. os.time()):reverse())
                -- TODO: Consider battle layouts.
                -- TODO: Check random seed
                self.party[i].dead = false
                self.party[i].atb = math.random(0, 255)
                self.party[i].command = {}
                -- Set default "Attack" command.
                self.party[i].command[1] = 1
                self.party[i].magic = {}
                self.party[i].summon = {}
                self.party[i].cover = 0
                self.party[i].counter = 0
                self.party[i].long_range = false
                self.party[i].mega_all = false
                self.party[i].pre_emptive = 0
                -- Fill Limit UI bar
                -- 0   -> 0%
                -- 255 -> 18%-6
                local width_percent = math.min(18, 18 * Characters[self.party[i].character].limit.bar / 255)
                local width_fixed = math.max(-6, -6 * width_percent / 18)
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(i) .. ".Right.LimitFill"):set_width(width_percent, width_fixed)
                -- Read materia
                local w_magic = false
                local w_item = false
                local w_summon = false
                for e = 1, 2 do -- Loop for weapon, armor
                    local equip = nil
                    if e == 1 then equip = Characters[Party[i]].weapon
                    else equip = Characters[Party[i]].armor
                    end

                    for m = 1, 8 do -- lop for materia in weapon/armor
                        if equip.materia[m - 1] ~= nil then
                            local materia = Game.Materia[equip.materia[m - 1].id]
                            local ap = equip.materia[m - 1].ap
                            local level = 0
                            for l = 1, #materia.levels_ap do if materia.levels_ap[l] <= ap then level = l end end
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
                                for lv, cmd in ipairs(materia.command) do if level >= lv then command_id = cmd end end
                                if command_id ~= nil then
                                    if command_id == 1 or command_id == 20 or command_id == 24 or command_id == 25 or command_id == 26 or command_id == 26 then
                                        -- Attack substitutes, index 1
                                        -- TODO: Handle priorities
                                        -- 1: Attack, 20: Limit, 24: Slash-all,
                                        -- 25: 2x-Cut, 26: Flash, 27: 4x-Cut
                                        self.party[i].command[1] = command_id
                                    else
                                        self.party[i].command[#self.party[i].command + 1] = command_id
                                    end
                                    if command_id == 21 then w_magic = true end
                                    if command_id == 22 then w_summon = true end
                                    if command_id == 23 then w_item = true end
                                end
                            end
                            -- TODO: Independent materia
                            -- TODO: Linked materia
                        end
                    end
                end

                -- Enable Item command if W-Item is disabled
                if w_item == false then self.party[i].command[#self.party[i].command + 1] = 4 end

                -- Enable Magic command if any magic is available and W-Magic is disabled.
                if #self.party[i].magic > 0 and w_magic == false then self.party[i].command[#self.party[i].command + 1] = 2 end
                -- Enable Summon command if any magic is available and W-Summon is disabled.
                if #self.party[i].summon > 0 and w_summon == false then self.party[i].command[#self.party[i].command + 1] = 3 end
                -- TODO: Sort commands/magic/summons
                table.sort(
                  self.party[i].command,
                  function(l, r)
                      -- Attack substitutes always go first
                      if l == 1 or l == 20 or l == 24 or l == 25 or l == 26 or l == 26 then return true
                      else return l < r
                      end
                  end
                )

                if #self.party[i].magic > 1 then table.sort(self.party[i].magic, function(l, r) return l.id < r.id end) end
                if #self.party[i].summon > 1 then table.sort(self.party[i].summon, function(l, r) return l.id < r.id end) end
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

        -- Lopad combatant data
        self:load_party_members()
        self:load_enemies()

        -- Set various transparencies
        ui_manager:get_widget("BattleUi.Container.Help.Center"):set_alpha(0.5)
        ui_manager:get_widget("BattleUi.Container.Message.Center"):set_alpha(0.5)
        ui_manager:get_widget("BattleUi.Container.Message.Center"):set_alpha(0.5)
        for i = 1, #self.party do
            ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(i) .. ".Right.AtbFill"):set_alpha(0.45)
            ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(i) .. ".Right.LimitFill"):set_alpha(0.45)
        end

        -- DEBUG:
        --ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Center"):set_alpha(0.5)
        --self:show_commands(1)

        return 0;
    end,

    --- Shows the command window for a character
    --
    -- @param party_id The ID of the party member whose commands to show
    show_commands = function(self, party_id)
        self.current_character_window = party_id
        self.cursor_command = 1
        self.window_state = "command"
        -- Colour the character name
        for c = 1, #self.party do
            if c == party_id then ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Left.Name" ):set_colour(1, 1, 0)
            else ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Left.Name"):set_colour(1, 1, 1)
            end
            ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Left.Name"):set_text(Characters[Party[c]].name)
        end
        -- Show command names
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands"):set_visible(false)
        local total_commands = 0
        for c = 1, 12 do
            if self.party[party_id].command[c] == nil then ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cmd" .. tostring(c)):set_visible(false)
            else
                ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cmd" .. tostring(c)):set_text(Game.Commands[self.party[party_id].command[c]].name)
                ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cmd" .. tostring(c)):set_visible(true)
                total_commands = total_commands + 1
            end
        end
        -- Resize the window
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands"):set_width(60 * (1 + math.floor(total_commands / 4)))
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands"):set_visible(true)
        -- Set and show cursor
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_default_animation("Position" .. tostring(self.cursor_command))
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_visible(true)
        self.window_state = "command"

    end,

    --- Hides the battle UI.
    --
    -- It also sets this widget to inactive.
    hide = function(self)
        ui_manager:get_widget("BattleUi"):set_visible(false)
        self.active = false
        return 0;
    end,

    --- Populates the battle bottom left and right pannels
    populate = function(self)

        space_pad = function(val, length)
            str = tostring(val)
            while #(str) < length do str = " " .. str end
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
