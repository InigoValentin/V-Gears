if UiContainer == nil then UiContainer = {} end

--- Animation settings used in battle.
--
-- Battle models have animations named XX_NN (XX are the first two letters of the model name, NN a
-- two digit number). A model also has numbered animation scripts (generated in
-- scripts/battle_models/) that chain animations, mark the moment an attack hits and so on. Attacks,
-- hurt reactions and deaths play one of those scripts.
--
-- Enemies attacks use the script number set in the enemy data. The scripts for the other
-- reactions are set here. Player models share the layout of their scripts, and can be overridden
-- by model name (like "rt_cloud") in `models`. Use the animation viewer in battle (keys V, N and
-- B) to see what each animation looks like.
BattleAnimations = {
    enemy = {hurt_script = 1},
    -- Script 12 is the victory pose, 5 and 6 are hurt reactions, 7 is the fall to the floor (animation
    -- 18, the root height goes from 673 to 58), 13 and 14 are the run away.
    player = {attack_script = 20, hurt_script = 5, death_script = 7, victory_script = 12, escape_script = 13,
        -- Script 29 (17 and 30 are the same): ready the weapon, incantation and release.
        magic_script = 29},
    models = {},
}

--- Music of the battles, by the names of the music index.
--
-- `battle` is the music of normal battles and `victory` the fanfare played once when a battle is won.
-- The music of the battles of a field can be changed by its scripts, with the BMUSC opcode.
-- Setting `next` makes the next battle use that track only.
BattleMusic = {
    battle = "bat",
    boss = "chu",
    victory = "fanfare",
    next = nil,
}

--- Status effects.
--
-- Status IDs are the bit positions of the original game, as stored in the attack and enemy data.
BattleStatus = {
    DEATH = 0, NEAR_DEATH = 1, SLEEP = 2, POISON = 3, FURY = 4, SADNESS = 5, CONFUSION = 6,
    SILENCE = 7, HASTE = 8, SLOW = 9, STOP = 10, FROG = 11, SMALL = 12, SLOW_NUMB = 13,
    PETRIFY = 14, REGEN = 15, BARRIER = 16, M_BARRIER = 17, REFLECT = 18, DUAL = 19, SHIELD = 20,
    D_SENTENCE = 21, MANIPULATE = 22, BERSERK = 23, PEERLESS = 24, PARALYSIS = 25, DARKNESS = 26,
}

BattleStatus.NAME = {
    [0] = "Death", "Near-death", "Sleep", "Poison", "Fury", "Sadness", "Confusion", "Silence",
    "Haste", "Slow", "Stop", "Frog", "Small", "Slow-numb", "Petrify", "Regen", "Barrier",
    "M.Barrier", "Reflect", "Dual", "Shield", "D.Sentence", "Manipulate", "Berserk", "Peerless",
    "Paralysis", "Darkness",
}

--- Seconds each status lasts. Statuses that are not here last until they are cured.
BattleStatus.DURATION = {
    [BattleStatus.SLEEP] = 40, [BattleStatus.SILENCE] = 40, [BattleStatus.HASTE] = 60,
    [BattleStatus.SLOW] = 60, [BattleStatus.STOP] = 25, [BattleStatus.REGEN] = 60,
    [BattleStatus.BARRIER] = 60, [BattleStatus.M_BARRIER] = 60, [BattleStatus.DARKNESS] = 60,
    [BattleStatus.PARALYSIS] = 15, [BattleStatus.REFLECT] = 60, [BattleStatus.PEERLESS] = 20,
    [BattleStatus.CONFUSION] = 30, [BattleStatus.D_SENTENCE] = 25, [BattleStatus.SLOW_NUMB] = 20,
}

--- Statuses that stop the ATB, so the combatant can't act.
BattleStatus.FROZEN = {
    [BattleStatus.SLEEP] = true, [BattleStatus.STOP] = true, [BattleStatus.PETRIFY] = true,
    [BattleStatus.PARALYSIS] = true,
}

--- Seconds between the damage or healing of poison and regen.
BattleStatus.TICK = 3

--- Fraction of the maximum HP lost by poison and recovered by regen on each tick.
BattleStatus.POISON_FRACTION = 1 / 32
BattleStatus.REGEN_FRACTION = 1 / 16

--- Playback speed of battle animations.
--
-- Animations are converted at 30 frames per second, but battle animations were designed to be
-- slower. 1 is the converted speed, 0.5 is half of it. It can be changed during a battle with
-- the keys Z (slower) and X (faster).
BattleAnimations.SPEED = 0.4

--- Seconds an action lasts when the animation to play is unknown.
BattleAnimations.DEFAULT_ACTION_TIME = 0.6

--- Distance, in battle units, party members run when escaping from a battle.
BattleAnimations.ESCAPE_DISTANCE = 40

--- Seconds an enemy takes to disappear after dying.
BattleAnimations.ENEMY_DEATH_TIME = 0.5

--- Shortest time, in seconds, a single frame animation is shown.
BattleAnimations.MIN_FRAME_TIME = 0.1

--- Speed, in battle units per second, at which combatants run to their targets and back.
BattleAnimations.RUN_SPEED = 12

--- Distances in the scripts are in original game units. This is the divisor to turn them into
-- battle units (the same one the installer uses for model coordinates).
BattleAnimations.DISTANCE_DIVISOR = 255

--- Cache of the animation scripts read from the files, by model name.
BattleAnimations.script_cache = {}

--- Reads the animation scripts of a model.
--
-- The generated files can't be run (some have jumps to missing labels), so they are read as text.
-- @param model Model name, without extension (like "aq_mp").
-- @return A table indexed by script number, each a list of steps: {type = "anim", number = N} for
-- animations to play, {type = "hit"} where the attack lands. Empty if the file can't be read.
BattleAnimations.get_scripts = function(model)
    if BattleAnimations.script_cache[model] ~= nil then return BattleAnimations.script_cache[model] end
    local scripts = {}
    local file = io.open("./data/scripts/battle_models/" .. model .. ".lua", "r")
    if file == nil then
        print("No animation scripts found for model " .. model)
    else
        local text = file:read("*a")
        file:close()
        for number, body in text:gmatch("%.script(%d+) = function%(%)(.-)\nend") do
            local steps = {}
            for fn, args in body:gmatch("([%w_]+)%(([^)]*)%)") do
                if fn == "play_animation" and tonumber(args) ~= nil then
                    table.insert(steps, {type = "anim", number = tonumber(args)})
                elseif fn == "execute_attack" then
                    table.insert(steps, {type = "hit"})
                elseif fn == "move_to_target" then
                    -- Arguments: distance to stop from the target, unknown, unknown.
                    table.insert(steps, {type = "move", distance = tonumber(args:match("^%s*(-?%d+)")) or 0})
                elseif fn == "return_position" then
                    table.insert(steps, {type = "return"})
                end
                -- TODO: set_direction, effects, sounds, camera...
            end
            scripts[tonumber(number)] = steps
        end
    end
    BattleAnimations.script_cache[model] = scripts
    return scripts
end

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

    --- Indexes in {@see enemies} of the living enemies shown in the target window.
    --
    -- Dead enemies can't be targeted. Refreshed every time the target window opens.
    target_enemies = {},

    --- Refreshes the list of enemies that can be targeted.
    refresh_target_enemies = function(self)
        self.target_enemies = {}
        for i = 1, #self.enemies do
            if self.enemies[i].dead == false then table.insert(self.target_enemies, i) end
        end
    end,

    --- Result pages shown after a victory (EXP, gil, items), as strings.
    result_pages = {},

    --- Index of the result page being shown.
    result_page = 0,

    --- Battle to start after the results, if any.
    pending_next_battle = -1,

    --- Animation scripts being played, by combatant.
    animation_queues = {},

    --- The action being executed. Nil if none.
    current_action = nil,

    --- Entities to hide when their death animation ends: {entity name, side, index, time}.
    pending_hides = {},

    --- Animation viewer: combatant and animation being shown.
    viewer = {combatant = 1, animation = 0},

    --- Attack data already read, by attack ID.
    attack_cache = {},

    --- Entries of the list window, see {@see build_list_entries}.
    list_entries = {},

    --- Item, spell or summon selected to be used.
    selected_item = nil,
    selected_summon = nil,

    --- Data of the basic physical attack ("Attack" command).
    --
    -- TODO: Use the weapon attack data and element.
    basic_attack = {
        id = -1, name = "Attack", mp = 0,
        damage = {formula = 1, modifier = 1, power = 16, accuracy = 1},
        target = {}, effects = {}, elements = {}, animation = {},
    },

    --- Determines if an action is being executed.
    action_executing = false,

    --- Last time (game time) L1 and R1 were pressed, to detect that both are being held.
    escape_seen = {},

    --- Party members that have turned their back to the enemies while trying to escape, by
    -- party index: {rotation = original rotation, animation = run animation, next_time = ...}.
    escape_pose = {},

    --- Time (in seconds) L1 and R1 have been held together.
    escape_progress = 0,

    --- Game time of the last tick, to calculate the time delta.
    last_tick_time = 0,

    --- Party ATBs are not filling (back attacks).
    party_hold = false,

    --- Enemy ATBs are not filling (preemptive attacks).
    enemy_hold = false,

    --- Game time at which the ATB hold started.
    hold_start = 0,

    --- Seconds before a held ATB starts to fill again if nobody has acted.
    HOLD_TIME = 15,

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

    --- Last command chosen by each party member (position in the command window), by party slot.
    -- Used when the cursor memory is on.
    memory_command = {},

    --- Last position chosen by each party member in the lists (magic, summon, item), by party slot
    -- and list kind: {first = first visible entry, cursor = row of the cursor}.
    memory_list = {},

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
        if self.window_state == "gameover" then do return 0 end end
        if self.window_state == "result" then
            if button == Config.ACTIONS.CONFIRM then
                audio_manager:play_sound("Cursor")
                self:show_next_result()
            end
            return 0
        end
        if button == "K" then Battle.finish() end
        if button == "V" or button == "N" or button == "B" then self:viewer_input(button) end
        if button == "Z" or button == "X" then self:change_animation_speed(button) end
        if button == "F" then self:viewer_rotate() end
        if button == "C" then
            -- Debug: cycle the cameras of the formation.
            local count = battle_manager:set_camera(0)
            if count > 0 then
                self.camera_index = ((self.camera_index or 0) + 1) % count
                battle_manager:set_camera(self.camera_index)
                ui_manager:get_widget("BattleUi.Container.Message.Name"):set_text("Camera " .. tostring(self.camera_index + 1) .. "/" .. tostring(count))
            end
        end
        -- Escape: L1 and R1 must be held together. Release events are not received, so
        -- the key repeat events are used to know if they are still held.
        if button == Config.ACTIONS.L1 or button == Config.ACTIONS.R1 then
            self.escape_seen[button] = timer:get_game_time_total()
        end
        -- Handle command cursors
        if self.window_state == "command" then
            local command_count = #self.party[self.current_character_window].command
            local cursor = ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor")
            if button == Config.CONTROLS.DOWN then
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
            elseif button == Config.CONTROLS.UP then
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
            elseif button == Config.CONTROLS.LEFT then
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
            elseif button == Config.CONTROLS.RIGHT then
                if command_count > 4 then
                    self.cursor_command = self.cursor_command + 4
                    if self.cursor_command > command_count then
                        self.cursor_command = self.cursor_command - 4 * math.floor(self.cursor_command / 4)
                        if self.cursor_command < 1 then self.cursor_command = self.cursor_command + 4 end
                    end
                    cursor:set_default_animation("Position" .. tostring(self.cursor_command))
                    audio_manager:play_sound("Cursor")
                end
            elseif button == Config.ACTIONS.CANCEL then
                -- Push the current character to the end of the command queue, and show the first.
                local new_queue = {}
                for i = 1, #self.command_queue do
                    if self.command_queue[i + 1] ~= nil then table.insert(new_queue, self.command_queue[i + 1]) end
                end
                table.insert(new_queue, self.command_queue[1])
                self.command_queue = new_queue
                audio_manager:play_sound("Cursor")
                self:show_commands(self.command_queue[1])
            elseif button == Config.ACTIONS.CONFIRM then
                self.memory_command[self.current_character_window] = self.cursor_command
                -- Check if a menu must be shown
                local cmd = Game.Commands[self.party[self.current_character_window].command[self.cursor_command]]
                if cmd.menu == Battle.Menu.NONE then print("DIRECT COMMAND")
                elseif cmd.menu == Battle.Menu.TARGET then
                    audio_manager:play_sound("Cursor")
                    self:select_command_target(self.party[self.current_character_window].command[self.cursor_command])
                elseif cmd.menu == Battle.Menu.MAGIC and (self:has_status(Battle.TargetGroup.PARTY, self.current_character_window, BattleStatus.SILENCE)
                  or self:has_status(Battle.TargetGroup.PARTY, self.current_character_window, BattleStatus.FROG)
                  or self:has_status(Battle.TargetGroup.PARTY, self.current_character_window, BattleStatus.SMALL)) then
                    -- Silenced, frog and small characters can't cast spells.
                    audio_manager:play_sound("Error")
                else
                    self.selected_command = self.party[self.current_character_window].command[self.cursor_command]
                    self:show_list(cmd.menu)
                end
            end
        -- Handle menu list cursor
        elseif self.window_state == "list" then
            local total = #self.list_entries
            local cursor = ui_manager:get_widget("BattleUi.Container.Bottom.List.Cursor")
            if button == Config.ACTIONS.CANCEL then
                audio_manager:play_sound("Cursor")
                self:close_list()
            elseif button == Config.CONTROLS.DOWN then
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
            elseif button == Config.CONTROLS.UP then
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
            elseif button == Config.ACTIONS.CONFIRM then
                audio_manager:play_sound("Cursor")
                self:select_target()
            end
        elseif self.window_state == "target" then
            if button == Config.ACTIONS.CANCEL then
                audio_manager:play_sound("Cursor")
                self:close_target()
            elseif button == Config.CONTROLS.UP then
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
                        if #self.target_enemies > 4 then
                            if self.cursor_target_v == 1 then
                                if self.first_target == 1 then
                                    -- Scroll to the bottom, set  cursor in last
                                    self.first_target = #self.target_enemies - 4
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
                            if self.cursor_target_v == 0 then self.cursor_target_v = #self.target_enemies end
                            audio_manager:play_sound("Cursor")
                            self:populate_target_list()
                        end
                    end
                end
            elseif button == Config.CONTROLS.DOWN then
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
                        if #self.target_enemies > 4 then
                            if self.cursor_target_v == 4 then
                                if self.first_target + 4 == #self.target_enemies then
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
                            if self.cursor_target_v > #self.target_enemies then self.cursor_target_v = 1 end
                            audio_manager:play_sound("Cursor")
                            self:populate_target_list()
                        end
                    end
                end
            elseif button == Config.CONTROLS.LEFT then
                if self.target_options.fixed_row == 0 and self.target_options.toggle_multiple == 0 then
                    -- If both rows are present and multiple toggling is not available...
                    if self.cursor_target_h == 2 then
                        -- and the cursor is in the second column, go to the fisrt. Also, move the
                        --cursor up until a valid enemy is pointed.
                        self.cursor_target_h = 1
                        while self.cursor_target_v + self.first_target - 1 > #self.target_enemies do self.cursor_target_v = self.cursor_target_v - 1 end
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
                        while self.cursor_target_v + self.first_target - 1 > #self.target_enemies do self.cursor_target_v = self.cursor_target_v - 1 end
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
            elseif button == Config.CONTROLS.RIGHT then
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
            elseif button == Config.ACTIONS.CONFIRM then
                -- Determine target type and target index
                if self.target_options.fixed_row == 1 then
                    if self.target_options.default_enemy == 1 then
                        self.target_group = Battle.TargetGroup.ENEMIES
                        if self.target_type == Battle.TargetType.SINGLE then self.target_index = self.target_enemies[self.cursor_target_v + self.first_target - 1]
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
                        if self.target_type == Battle.TargetType.SINGLE then self.target_index = self.target_enemies[self.cursor_target_v + self.first_target - 1]
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
        self:populate_list()
    end,

    --- Opens the target window for a command that goes straight to target selection.
    --
    -- Used by commands with {@see Battle.Menu.TARGET}, like "Attack".
    -- @param command_id ID of the command in Game.Commands.
    select_command_target = function(self, command_id)
        self.selected_command = command_id
        self.current_list = "none"
        self.selected_attack = nil
        self.selected_item = nil
        self.selected_summon = nil
        self.target_options = {
            fixed_row = 0, default_enemy = 1, default_multiple = 0, toggle_multiple = 0,
            all_rows = 0, random = 0, selectable = 1
        }
        self.target_type = Battle.TargetType.SINGLE
        self.cursor_target_h = 1
        self.cursor_target_v = 1
        self.first_target = 1
        self:refresh_target_enemies()
        self:populate_target_list()
        ui_manager:get_widget("BattleUi.Container.Bottom.Target"):set_visible(true)
        self.window_state = "target"
    end,

    --- Adds a party character action to the queue
    --
    -- Closes the selection windows and removes the character from the command queue.
    queue_character_action = function(self)
        local action = {}
        action.actor = self.current_character_window
        action.actor_side = Battle.TargetGroup.PARTY
        action.command = self.selected_command
        if self.current_list == "none" then action.type = Battle.ActionType.COMMAND
        else action.type = Battle.ActionType.ATTACK
        end
        -- Spells, summons and items carry the attack to use. The basic attack has none.
        if self.current_list ~= "none" then
            action.attack_id = self.selected_attack
            action.item_id = self.selected_item
            action.summon = self.selected_summon
        end
        action.target_type = self.target_type
        action.target_group = self.target_group
        action.target_index = self.target_index
        action.priority = 6 -- TODO: find out
        table.insert(self.action_queue, action)
        -- The member has chosen: it doesn't choose again until the action is executed.
        self.party[action.actor].acting = true
        -- TODO: Reorder queue by priority.

        -- The character has chosen: remove it from the command queue and close the windows.
        for i = #self.command_queue, 1, -1 do
            if self.command_queue[i] == action.actor then table.remove(self.command_queue, i) end
        end
        ui_manager:get_widget("BattleUi.Container.Bottom.Target"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.List"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands"):set_visible(false)
        self.current_list = "none"
        self.window_state = "wait"
    end,

    --- Handles a time tick.
    --
    -- Increases the ATBs of party and enemies. If one of them gets filled, it shows tha command
    -- window
    tick = function(self)
        if System:is_paused() == true then do return end end
        local now = timer:get_game_time_total()
        if self.ending == true then
            -- Nothing else advances once the battle is over, but animations (victory poses) go on.
            self:update_animations(now)
            UiContainer.BattleFx:update(now)
            do return end
        end
        local delta = now - self.last_tick_time
        self.last_tick_time = now
        if self:update_escape(now, delta) then return end
        self:update_animations(now)
        UiContainer.BattleFx:update(now)
        self:update_hides(now)
        self:update_action(now)
        -- Held ATBs start to fill after some time.
        if (self.party_hold or self.enemy_hold) and now - self.hold_start > self.HOLD_TIME then
            self:release_hold()
        end
        self:update_statuses(now)
        -- Advance party ATBs
        for i = 1, #self.party do
            local member = self.party[i]
            if member.dead == false and self.party_hold == false then
                -- 0 if a status stops the ATB.
                local rate = self:get_atb_rate(Battle.TargetGroup.PARTY, i)
                if member.atb < 255 and rate > 0 then
                    member.atb = member.atb + 0.04 * Characters[member.character].stats.dex.total * rate -- TODO
                    -- Fill ATB UI bar
                    -- 0   -> 0%
                    -- 255 -> 18%-6
                    local width_percent = math.min(18, 18 * member.atb / 255)
                    local width_fixed = math.max(-6, -6 * width_percent / 18)
                    ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(i) .. ".Right.AtbFill"):set_width(width_percent, width_fixed)
                end
                -- A full ATB can choose a command, unless a status stops it or it already has one.
                local queued = false
                for _, q in ipairs(self.command_queue) do if q == i then queued = true end end
                if member.atb >= 255 and rate > 0 and not member.acting and self:acts_by_itself(Battle.TargetGroup.PARTY, i) then
                    -- Berserk and confused characters don't choose.
                    self:queue_auto_action(Battle.TargetGroup.PARTY, i)
                elseif member.atb >= 255 and rate > 0 and not member.acting then
                    if not queued then
                        print(Characters[member.character].name .. " ATB full")
                        ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(i) .. ".Right.AtbFill"):set_colour(0, 1, 0)
                        table.insert(self.command_queue, i)
                    end
                elseif queued then
                    self:remove_from_command_queue(i)
                end
            end
        end
        -- Advance enemy ATBs
        for i = 1, #self.enemies do
            if self.enemies[i].dead == false and self.enemy_hold == false then
                local rate = self:get_atb_rate(Battle.TargetGroup.ENEMIES, i)
                if rate > 0 then
                    if self.enemies[i].atb < 255 and self.enemies[i].waiting ~= true then
                        self.enemies[i].atb = self.enemies[i].atb + 0.04 * self.enemies[i].stats.dex * rate -- TODO
                    end
                    if self.enemies[i].atb >= 255 and self.enemies[i].waiting ~= true then
                        print("Enemy " .. self.enemies[i].name .. " ATB full")
                        self:queue_enemy_action(i)
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

    --- Sets the initial ATBs, depending on the battle layout.
    --
    -- Layouts as in BattleManager::LAYOUT: 0 normal, 1 preemptive, 2 back attack, 3 side attack,
    -- 4 pincer attack, 5 solo, 8 locked. ATBs start random, except:
    -- preemptive: party full, enemies empty and held; back attack: party empty and held, enemies
    -- full; side attack: party full; pincer attack: enemies full.
    init_atb = function(self)
        local layout = battle_manager:get_layout()
        self.party_hold = false
        self.enemy_hold = false
        self.hold_start = timer:get_game_time_total()
        self.last_tick_time = self.hold_start
        self.escape_seen = {}
        self.escape_progress = 0
        self.escape_pose = {}
        local party_atb, enemy_atb = nil, nil
        if layout == 1 then
            party_atb, enemy_atb = 255, 0
            self.enemy_hold = true
        elseif layout == 2 then
            party_atb, enemy_atb = 0, 255
            self.party_hold = true
        elseif layout == 3 then party_atb = 255
        elseif layout == 4 then enemy_atb = 255
        end
        for i = 1, #self.party do
            if party_atb ~= nil then self.party[i].atb = party_atb end
            if self.party[i].atb >= 255 then
                table.insert(self.command_queue, i)
                local bar = ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(i) .. ".Right.AtbFill")
                bar:set_width(18, -6)
                bar:set_colour(0, 1, 0)
            end
        end
        for i = 1, #self.enemies do
            if enemy_atb ~= nil then self.enemies[i].atb = enemy_atb end
        end
    end,

    --- Lets held ATBs start to fill.
    release_hold = function(self)
        self.party_hold = false
        self.enemy_hold = false
    end,

    --- Handles escaping.
    --
    -- While L1 and R1 are held, the party turns its back to the enemies and runs in place. If they
    -- are held long enough, and the formation allows it, the party runs away and the battle ends.
    -- Releasing them makes the party turn back to the enemies. The time depends on the escape
    -- difficulty of the formation.
    -- @param now Current game time.
    -- @param delta Seconds since the last tick.
    -- @return True if the battle was escaped.
    update_escape = function(self, now, delta)
        local l1 = self.escape_seen[Config.ACTIONS.L1]
        local r1 = self.escape_seen[Config.ACTIONS.R1]
        -- Key repeat is not instant, so both keys are considered held for a while after the
        -- last event.
        local window = 0.8
        local held = l1 ~= nil and r1 ~= nil and now - l1 <= window and now - r1 <= window
        local difficulty = battle_manager:get_escape_difficulty()
        if not held then
            self.escape_progress = 0
            if next(self.escape_pose) ~= nil then
                print("Escape: keys released, turning back.")
                self:end_escape_pose()
            end
            return false
        end
        if difficulty < 0 or Battle.has_mode(Battle.Mode.NO_ESCAPE) then
            if self.escape_progress == 0 then print("This battle can't be escaped.") end
            self.escape_progress = delta
            return false
        end
        if self.escape_progress == 0 and next(self.escape_pose) == nil then
            print("Escape: keys held, turning around.")
            self:begin_escape_pose()
        end
        self.escape_progress = self.escape_progress + delta
        self:update_escape_pose(now)
        if self.escape_progress >= 1 + 4 * difficulty then
            print("Escaped from the battle.")
            self:start_escape()
            return true
        end
        return false
    end,

    --- Gets the data of an attack.
    --
    -- @param id Attack ID. Nil for the basic attack.
    -- @return The attack data as in the attack files, nil if it doesn't exist.
    get_attack_data = function(self, id)
        if id == nil or id < 0 then return self.basic_attack end
        if self.attack_cache[id] == nil then
            local data = nil
            -- Attacks of the scenes have a file. The other ones (summons) come from the kernel.
            if battle_manager:has_attack(id) then data = battle_manager:get_attack(id)
            else data = self:kernel_attack_to_data(id)
            end
            self.attack_cache[id] = data or false
        end
        return self.attack_cache[id] or nil
    end,

    --- Converts the status effects of the kernel data (items, kernel attacks) to the format of
    -- the attack files.
    convert_kernel_effects = function(self, status)
        local effects = {}
        for _, st in ipairs(status or {}) do
            local mode = "inflict"
            if st.mode == 1 then mode = "cure" elseif st.mode == 2 then mode = "toggle" end
            table.insert(effects, {id = st.status, chance = st.chance / 63, mode = mode})
        end
        return effects
    end,

    --- Converts the target data of the kernel data to the format of the attack files.
    convert_kernel_target = function(self, target)
        local result = {}
        for key, value in pairs(target or {}) do result[key == "selectable" and "selection_enabled" or key] = value end
        return result
    end,

    --- Builds the attack data of a kernel attack, with the format of the attack files.
    --
    -- @return The attack data, nil if there is no such attack.
    kernel_attack_to_data = function(self, id)
        local k = Game.Attacks[id]
        if k == nil then return nil end
        return {
            id = id, name = k.name, mp = k.mp, reflectable = false,
            damage = {
                formula = k.dmg_formula, modifier = k.dmg_modifier, power = k.power,
                accuracy = (k.accuracy or 255) / 255, restore = k.restore,
            },
            target = self:convert_kernel_target(k.target), animation = {},
            effects = self:convert_kernel_effects(k.status), elements = k.elements or {},
        }
    end,

    --- Builds attack data from an item, so it can be used as any attack.
    --
    -- Item formulas: (2, 6) restores power * 20 HP or MP, (2, 4) restores power / 32 of the
    -- maximum HP or MP, (0, 8) restores everything, (3, 7) is fixed damage of power * 20, and (2, 3)
    -- is a fraction of the current HP. Items without damage only apply statuses.
    item_to_attack = function(self, item_id)
        local item = Game.Items[item_id]
        if item == nil then return nil end
        return {
            id = -2, item = true, name = item.name, mp = 0, reflectable = false,
            damage = {
                formula = item.dmg_formula, modifier = item.dmg_modifier, power = item.power,
                accuracy = 1, restore = item.restore,
            },
            target = self:convert_kernel_target(item.target), animation = {},
            effects = self:convert_kernel_effects(item.status), elements = item.elements or {},
        }
    end,

    --- Gets the attack data of an action: an item, a spell or summon, or the basic attack.
    get_action_attack = function(self, action)
        if action.item_id ~= nil then return self:item_to_attack(action.item_id) or self.basic_attack end
        return self:get_attack_data(action.attack_id) or self.basic_attack
    end,

    --- Checks if an attack brings back the dead (it cures death), like Life or Phoenix Down.
    attack_revives = function(self, attack)
        for _, effect in ipairs(attack.effects or {}) do
            if effect.id == BattleStatus.DEATH and effect.mode == "cure" then return true end
        end
        return false
    end,

    --- Checks if a combatant acts on its own because of its statuses (berserk or confusion).
    acts_by_itself = function(self, side, index)
        return self:has_status(side, index, BattleStatus.BERSERK) or self:has_status(side, index, BattleStatus.CONFUSION)
    end,

    --- Queues a basic attack for a combatant that can't choose what to do.
    --
    -- Berserk combatants attack the other side, confused ones attack anybody.
    queue_auto_action = function(self, side, index)
        local other = side == Battle.TargetGroup.PARTY and Battle.TargetGroup.ENEMIES or Battle.TargetGroup.PARTY
        local target_side = other
        if self:has_status(side, index, BattleStatus.CONFUSION) and math.random() < 0.5 then target_side = side end
        local target = self:random_living(target_side)
        if target == nil then
            target_side = other
            target = self:random_living(other)
        end
        if target == nil then do return end end
        table.insert(self.action_queue, {
            actor = index, actor_side = side, type = Battle.ActionType.COMMAND, command = 1,
            target_type = Battle.TargetType.SINGLE, target_group = target_side, target_index = target,
            priority = 6,
        })
        local combatant = self:get_combatant(side, index)
        if side == Battle.TargetGroup.PARTY then combatant.acting = true
        else
            combatant.atb = 0
            combatant.waiting = true
        end
    end,

    -- ================================================================================
    -- Enemy AI scripts (see battle_ai_vm.lua)
    -- ================================================================================

    --- Registers of the actor fields the scripts can read: field address (in bits) of each.
    AI_REGISTERS = {
        0x00, 0x20, 0x40, 0x48, 0x58, 0x60, 0x68, 0x70, 0x78, 0x80, 0x88, 0x90, 0x98, 0xA0, 0xA8,
        0xB0, 0xB8, 0xC0, 0xD0, 0xE0, 0xF0, 0x100, 0x110, 0x120, 0x130, 0x140, 0x150, 0x160, 0x180,
        0x268, 0x270, 0x2B0, 0x2C0, 0x2E0,
    },

    --- Gets the actor number a combatant has in the scripts: 0 to 2 are the party, 4 to 9 enemies.
    ai_actor_of = function(self, side, index)
        if side == Battle.TargetGroup.PARTY then return index - 1 end
        return index + 3
    end,

    --- Gets the combatant of an actor number.
    -- @return side and index, nil if there is no such actor.
    ai_combatant_of = function(self, actor)
        if actor >= 0 and actor <= 2 and self.party[actor + 1] ~= nil then return Battle.TargetGroup.PARTY, actor + 1 end
        if actor >= 4 and actor <= 9 and self.enemies[actor - 3] ~= nil then return Battle.TargetGroup.ENEMIES, actor - 3 end
        return nil
    end,

    --- Gets the mask of the actors of a side.
    -- @param alive_only True to only include the ones that are not dead.
    ai_mask = function(self, side, alive_only)
        local list = side == Battle.TargetGroup.PARTY and self.party or self.enemies
        local mask = 0
        for i = 1, #list do
            if not alive_only or list[i].dead == false then mask = mask + 2 ^ self:ai_actor_of(side, i) end
        end
        return mask
    end,

    --- Gets the side of an actor number.
    ai_side_of = function(self, actor)
        if actor <= 2 then return Battle.TargetGroup.PARTY end
        return Battle.TargetGroup.ENEMIES
    end,

    --- Reads a global value of the scripts.
    ai_read_global = function(self, owner, address)
        local own = self:ai_side_of(owner)
        local other = own == Battle.TargetGroup.PARTY and Battle.TargetGroup.ENEMIES or Battle.TargetGroup.PARTY
        if address == 0x2060 then return 2 ^ owner
        elseif address == 0x2050 then return self:ai_mask(Battle.TargetGroup.PARTY, true) + self:ai_mask(Battle.TargetGroup.ENEMIES, true)
        elseif address == 0x2070 then return self.ai_targets or 0
        elseif address == 0x2080 then return self:ai_mask(own, false)
        elseif address == 0x2090 then return self:ai_mask(own, true)
        elseif address == 0x20A0 then return self:ai_mask(other, false)
        elseif address == 0x20B0 then return self:ai_mask(other, true)
        elseif address == 0x20C0 then return self:ai_mask(Battle.TargetGroup.PARTY, true)
        elseif address == 0x20E0 then return self:ai_mask(Battle.TargetGroup.PARTY, false) + self:ai_mask(Battle.TargetGroup.ENEMIES, false)
        elseif address == 0x2020 then return battle_manager:get_layout()
        elseif address == 0x2140 then return battle_manager:get_formation_id()
        elseif address == 0x2000 then return 0
        elseif address == 0x2008 or address == 0x2150 then return self.ai_last_action or 0
        elseif address == 0x2120 then return self.ai_last_elements or 0
        elseif address == 0x2010 then return self.ai_membank or 0
        elseif address >= 0x2110 and address < 0x2120 then return self.ai_end_flags or 0
        end
        return 0
    end,

    --- Writes a global value of the scripts.
    ai_write_global = function(self, owner, address, size, value)
        if address == 0x2070 then self.ai_targets = value
        elseif address == 0x2010 then self.ai_membank = value
        elseif address >= 0x2110 and address < 0x2120 then
            -- Flags of the end of the battle, one bit each: 0x2114 ends the battle.
            local bit = address - 0x2110
            if size == 0 and bit == 4 and value ~= 0 then self.ai_end_battle = true end
        end
    end,

    --- Gets the value of one of the registers of an actor (see AI_REGISTERS).
    ai_read_register = function(self, actor, base)
        local side, index = self:ai_combatant_of(actor)
        if side == nil then return 0 end
        local c = self:get_combatant(side, index)
        local stats = self:get_combat_stats(side, index)
        if base == 0x00 then
            local mask = 0
            for id, _ in pairs(c.status or {}) do mask = mask + 2 ^ id end
            if c.dead and math.floor(mask) % 2 == 0 then mask = mask + 1 end
            return mask
        elseif base == 0x20 then
            local back = false
            if side == Battle.TargetGroup.PARTY then back = Characters[c.character].back_row == true
            else back = not battle_manager:get_enemy(index - 1):is_front() end
            local flags = 0
            if back then flags = flags + 2 ^ 6 end
            if c.dead then flags = flags + 2 ^ 13 end
            if side == Battle.TargetGroup.ENEMIES and c.hidden then flags = flags + 2 ^ 14 end
            return flags
        elseif base == 0x40 or base == 0x120 then return actor
        elseif base == 0x48 then return stats.level
        elseif base == 0x60 then return side == Battle.TargetGroup.PARTY and (c.character + 16) or index
        elseif base == 0x68 then return stats.str
        elseif base == 0x70 then return stats.mag
        elseif base == 0xA0 then
            if side == Battle.TargetGroup.PARTY then return Characters[c.character].stats.dex.total end
            return c.stats.dex
        elseif base == 0xA8 then
            if side == Battle.TargetGroup.PARTY then return Characters[c.character].stats.lck.total end
            return c.stats.lck
        elseif base == 0x100 then return stats.def
        elseif base == 0x110 then return stats.mdef
        elseif base == 0x140 then return stats.mp
        elseif base == 0x150 then return stats.mp_max
        elseif base == 0x160 then return stats.hp
        elseif base == 0x180 then return stats.hp_max
        elseif base == 0x270 then
            if side == Battle.TargetGroup.PARTY then return Characters[c.character].back_row and 1 or 0 end
            return battle_manager:get_enemy(index - 1):is_front() and 0 or 1
        elseif base == 0x2B0 then return c.ap or 0
        elseif base == 0x2C0 then return c.money or 0
        elseif base == 0x2E0 then return c.exp or 0
        elseif base == 0xC0 then return c.ai_last_targets or 0
        elseif base == 0xD0 then return c.ai_prev_attacker or 0
        elseif base == 0xE0 then return c.ai_prev_physical or 0
        elseif base == 0xF0 then return c.ai_prev_magical or 0
        end
        return 0
    end,

    --- Reads an actor field of the scripts.
    -- @param field Field address, in bits (the address minus 0x4000).
    ai_read_actor = function(self, actor, field, size)
        local base = nil
        for _, register in ipairs(self.AI_REGISTERS) do
            if register <= field then base = register end
        end
        if base == nil then return 0 end
        local value = math.floor(self:ai_read_register(actor, base))
        local offset = field - base
        if size == 0 then return math.floor(value / 2 ^ offset) % 2 end
        return math.floor(value / 2 ^ offset) % 2 ^ (8 * BattleAI.SIZE_BYTES[size])
    end,

    --- Writes an actor field of the scripts. Only some fields do something.
    ai_write_actor = function(self, actor, field, size, value)
        local side, index = self:ai_combatant_of(actor)
        if side == nil then do return end end
        local c = self:get_combatant(side, index)
        if field < 0x20 then
            -- Statuses. A bit sets or cures one, a whole value changes all of them.
            local function change(id, on)
                if id == BattleStatus.DEATH then
                    if on and not c.dead then
                        local stats = self:get_combat_stats(side, index)
                        if stats.hp > 0 then self:apply_damage(side, index, stats.hp, false) end
                    end
                elseif on then self:set_status(side, index, id)
                else self:clear_status(side, index, id)
                end
            end
            if size == 0 then change(field, value ~= 0)
            else
                for id = 0, 31 do
                    local on = math.floor(value / 2 ^ id) % 2 == 1
                    if on ~= (c.status[id] ~= nil) then change(id, on) end
                end
            end
        elseif field >= 0x20 and field < 0x40 then
            local bit = field - 0x20
            if size == 0 and bit == 14 and side == Battle.TargetGroup.ENEMIES then
                -- Invisible: the model is hidden, and it can't be targeted.
                c.hidden = value ~= 0
                battle_manager:set_enemy_visible(index - 1, value == 0)
            end
        elseif field >= 0x160 and field < 0x180 and size == 3 then
            -- HP.
            local stats = self:get_combat_stats(side, index)
            local difference = stats.hp - math.floor(value)
            if difference ~= 0 then self:apply_damage(side, index, difference, false) end
        elseif field >= 0x140 and field < 0x150 and size == 2 then
            -- MP.
            local stats = self:get_combat_stats(side, index)
            local difference = stats.mp - math.floor(value)
            if difference ~= 0 then self:apply_damage(side, index, difference, true) end
        end
    end,

    --- Decodes a text of the FF7 encoding.
    ai_decode_text = function(self, bytes)
        local text = {}
        for i = 1, #bytes do
            local code = bytes:byte(i)
            -- Characters are the ASCII ones minus 0x20. The other codes are control codes.
            if code <= 0x5F then table.insert(text, string.char(code + 0x20))
            elseif code == 0xE0 or code == 0xE1 then table.insert(text, " ")
            end
        end
        return table.concat(text)
    end,

    --- The interface the AI scripts use to talk to the battle.
    ai_host = nil,

    --- Creates the interface of the AI interpreter, once.
    get_ai_host = function(self)
        if self.ai_host ~= nil then return self.ai_host end
        local ui = self
        local host = {}
        function host:read_global(address, size) return ui:ai_read_global(self.owner, address) end
        function host:write_global(address, size, value) ui:ai_write_global(self.owner, address, size, value) end
        function host:read_actor(actor, field, size) return ui:ai_read_actor(actor, field, size) end
        function host:write_actor(actor, field, size, value) ui:ai_write_actor(actor, field, size, value) end
        function host:random(n) return math.random(0, n - 1) end
        function host:mp_cost(id)
            local attack = ui:get_attack_data(id)
            return attack ~= nil and attack.mp or 0
        end
        function host:text(owner, bytes)
            local text = ui:ai_decode_text(bytes)
            print("AI text: " .. text)
            ui_manager:get_widget("BattleUi.Container.Message.Name"):set_text(text)
        end
        function host:memory_bank(write, offset)
            -- Access to the game memory banks: the value at 0x2010 is stored there, or read from
            -- there. TODO: Map the offsets to the variables of the game.
        end
        function host:action(owner, kind, id) ui:ai_action(owner, kind, id) end
        self.ai_host = host
        return host
    end,

    --- Handles an action requested by a script.
    ai_action = function(self, owner, kind, id)
        local side, index = self:ai_combatant_of(owner)
        if side == nil then do return end end
        if kind == BattleAI.ACTION.RUN_SCRIPT then
            self:ai_run(owner, id)
            do return end
        end
        if kind ~= BattleAI.ACTION.ENEMY_ATTACK then do return end end
        -- Targets: the actors of the mask, all of the side of the first one.
        local mask = math.floor(self.ai_targets or 0)
        local group, list = nil, {}
        for actor = 0, 31 do
            if math.floor(mask / 2 ^ actor) % 2 == 1 then
                local target_side, target_index = self:ai_combatant_of(actor)
                if target_side ~= nil and (group == nil or group == target_side) then
                    group = target_side
                    table.insert(list, target_index)
                end
            end
        end
        if group == nil then do return end end
        -- The animation of the attack is the one of the enemy slot that has it.
        local script = nil
        if side == Battle.TargetGroup.ENEMIES then
            local source = battle_manager:get_enemy(index - 1)
            for slot = 0, source:get_attack_count() - 1 do
                if source:get_attack(slot) == id then
                    script = source:get_attack_animation(slot)
                    break
                end
            end
            if script ~= nil and script < 0 then script = nil end
        end
        local action = {
            actor = index, actor_side = side, type = Battle.ActionType.COMMAND, command = 1,
            attack_id = id, script = script,
            target_type = #list == 1 and Battle.TargetType.SINGLE or Battle.TargetType.GROUP,
            target_group = group, target_list = list, target_index = #list == 1 and list[1] or nil,
            priority = 6, allow_dead = self.ai_counter == true, is_counter = self.ai_counter == true,
        }
        if self.ai_counter then table.insert(self.action_queue, 1, action)
        else table.insert(self.action_queue, action)
        end
        self.ai_queued = (self.ai_queued or 0) + 1
    end,

    --- Runs one of the AI scripts of an actor.
    --
    -- @param slot 0 init, 1 main, 2 general counter, 3 death counter, 4 physical counter, 5 magic
    -- counter, 6 battle end, 7 pre-turn, 8 to 15 events.
    -- @return True if the actor has a script in that slot.
    ai_run = function(self, owner, slot)
        local side, index = self:ai_combatant_of(owner)
        if side ~= Battle.TargetGroup.ENEMIES then return false end
        local code = battle_manager:get_enemy(index - 1):get_ai_script(slot)
        if code == nil or code == "" then return false end
        -- Scripts set the targets before an action; they start from nothing.
        local previous_targets = self.ai_targets
        self.ai_targets = 0
        BattleAI.run_safe(code, owner, self:get_ai_host())
        self.ai_targets = previous_targets
        return true
    end,

    --- Prepares the AI of the battle: clears the variables and runs the initialization scripts.
    ai_setup = function(self)
        BattleAI.reset()
        self.ai_targets = 0
        self.ai_queued = 0
        self.ai_counter = false
        self.ai_end_battle = false
        self.ai_end_flags = 0
        self.ai_last_action = 0
        self.ai_last_elements = 0
        for i = 1, #self.enemies do self:ai_run(self:ai_actor_of(Battle.TargetGroup.ENEMIES, i), BattleAI.SLOT.INIT) end
    end,

    --- Lets the AI scripts of an enemy decide its turn.
    --
    -- @return True if the enemy has AI scripts and they were used, false to use a random attack.
    ai_take_turn = function(self, index)
        local owner = self:ai_actor_of(Battle.TargetGroup.ENEMIES, index)
        local source = battle_manager:get_enemy(index - 1)
        if source:get_ai_script(BattleAI.SLOT.MAIN) == "" then return false end
        self.ai_counter = false
        self.ai_queued = 0
        self:ai_run(owner, BattleAI.SLOT.PRE_TURN)
        self:ai_run(owner, BattleAI.SLOT.MAIN)
        self.enemies[index].atb = 0
        -- Without actions, the enemy skips the turn. Otherwise it waits for them to be executed.
        if self.ai_queued > 0 then self.enemies[index].waiting = true end
        return true
    end,

    --- Runs the counter scripts of an enemy after it's attacked.
    --
    -- @param attacker_side Side of the attacker, and attacker_index its index.
    -- @param kind The formula of the attack: 1 physical, 2 magical.
    ai_after_hit = function(self, attack, attacker_side, attacker_index, target_index, dead)
        local enemy = self.enemies[target_index]
        local owner = self:ai_actor_of(Battle.TargetGroup.ENEMIES, target_index)
        local attacker = 2 ^ self:ai_actor_of(attacker_side, attacker_index)
        local formula = math.floor(attack.damage.formula or 0)
        enemy.ai_prev_attacker = attacker
        if formula == 1 then enemy.ai_prev_physical = attacker else enemy.ai_prev_magical = attacker end
        self.ai_last_action = attack.id >= 0 and attack.id or 0
        local elements = 0
        for _, element in ipairs(attack.elements or {}) do elements = elements + 2 ^ element end
        self.ai_last_elements = elements
        -- Counterattacks go before everything else in the queue.
        self.ai_counter = true
        if not dead then
            self:ai_run(owner, BattleAI.SLOT.COUNTER_GENERAL)
            if formula == 1 then self:ai_run(owner, BattleAI.SLOT.COUNTER_PHYSICAL)
            else self:ai_run(owner, BattleAI.SLOT.COUNTER_MAGIC)
            end
        else
            self:ai_run(owner, BattleAI.SLOT.COUNTER_DEATH)
        end
        self.ai_counter = false
    end,

    --- Queues an attack for an enemy whose ATB is full.
    --
    -- The attack is picked at random between the ones of the enemy.
    -- TODO: Use the enemy AI scripts.
    -- @param index Enemy index in {@see enemies}.
    queue_enemy_action = function(self, index)
        if self:acts_by_itself(Battle.TargetGroup.ENEMIES, index) then
            self:queue_auto_action(Battle.TargetGroup.ENEMIES, index)
            do return end
        end
        -- The AI scripts of the enemy decide, if it has any.
        if self:ai_take_turn(index) then do return end end
        local source = battle_manager:get_enemy(index - 1)
        local attack_id, script = nil, nil
        local count = source:get_attack_count()
        if count > 0 then
            local slot = math.random(0, count - 1)
            attack_id = source:get_attack(slot)
            script = source:get_attack_animation(slot)
            if script < 0 then script = nil end
        end
        local attack = self:get_attack_data(attack_id) or self.basic_attack
        -- Enemy attacks aimed at "enemies" are aimed at the party, the other ones at the enemies
        -- (restoring, buffs...).
        local target_group = Battle.TargetGroup.PARTY
        if attack.target.default_enemy == 0 then target_group = Battle.TargetGroup.ENEMIES end
        local list = target_group == Battle.TargetGroup.PARTY and self.party or self.enemies
        local candidates = {}
        for i = 1, #list do
            if list[i].dead == false then table.insert(candidates, i) end
        end
        if #candidates == 0 then do return end end
        local target_index = nil
        if attack.target.default_multiple ~= 1 then target_index = candidates[math.random(1, #candidates)] end
        table.insert(self.action_queue, {
            actor = index,
            actor_side = Battle.TargetGroup.ENEMIES,
            type = Battle.ActionType.COMMAND,
            command = 1,
            attack_id = attack_id,
            script = script,
            target_type = target_index == nil and Battle.TargetType.GROUP or Battle.TargetType.SINGLE,
            target_group = target_group,
            target_index = target_index,
            priority = 6,
        })
        -- Stop the enemy ATB from refilling until its action is done.
        self.enemies[index].atb = 0
        self.enemies[index].waiting = true
    end,

    --- Gets the stats a damage formula needs from a combatant.
    --
    -- @param side {@see Battle.TargetGroup.PARTY} or {@see Battle.TargetGroup.ENEMIES}.
    -- @param index Index in the party or enemy list.
    -- @return A table with level, str (attack), def, mag, mdef, hp, hp_max, mp.
    get_combat_stats = function(self, side, index)
        if side == Battle.TargetGroup.PARTY then
            local char = Characters[self.party[index].character]
            local stats = char.stats
            local str = stats.str.total or stats.str.base or 1
            if char.weapon ~= nil and char.weapon.attack ~= nil then str = str + char.weapon.attack end
            return {
                level = char.level or 1, str = str,
                def = stats.vit.total or stats.vit.base or 0,
                mag = stats.mag.total or stats.mag.base or 1,
                mdef = stats.spr.total or stats.spr.base or 0,
                hp = stats.hp.current, hp_max = self:get_stat_max(stats.hp),
                mp = stats.mp.current, mp_max = self:get_stat_max(stats.mp),
            }
        end
        local enemy = self.enemies[index]
        return {
            level = enemy.level, str = enemy.stats.str, def = enemy.stats.def, mag = enemy.stats.mag,
            mdef = enemy.stats.spr, hp = enemy.stats.hp, hp_max = enemy.stats.hp_max, mp = enemy.stats.mp,
            mp_max = enemy.stats.mp_max,
        }
    end,

    --- Gets how an enemy reacts to the elements of an attack.
    --
    -- @return The damage factor: 1 normal, 2 double, 0.5 half, 0 nullified, negative absorbed (it
    -- heals), 10 or more kills, -10 or less heals everything. Party members have no reactions yet.
    get_element_factor = function(self, attack, side, index)
        if side ~= Battle.TargetGroup.ENEMIES then return 1 end
        local source = battle_manager:get_enemy(index - 1)
        local factor = 1
        for _, element in ipairs(attack.elements) do
            for e = 0, source:get_element_count() - 1 do
                if source:get_element(e) == element then factor = factor * source:get_element_factor(e) end
            end
        end
        return factor
    end,

    --- Calculates the result of an attack on a target.
    --
    -- Formulas as in the original game: 1 is physical, 2 is magical (modifier 5 heals, 3 takes a
    -- fraction of the current HP). Other formulas are not implemented and do no damage.
    -- TODO: Criticals, evasion, rows, defending, statuses.
    -- @param attack Attack data, as in {@see get_attack_data}.
    -- @return A table: hit (false if the attack misses), amount (positive damages, negative heals),
    -- mp (true if the amount is for MP).
    calculate_attack = function(self, attack, attacker_side, attacker_index, target_side, target_index)
        local data = attack.damage
        local formula, modifier, power = math.floor(data.formula or 0), math.floor(data.modifier or 0), data.power or 0
        local result = {hit = true, amount = 0, mp = (data.damage_mp or 0) == 1, extra_mp = 0}
        local is_item = attack.item == true
        -- Accuracy is stored as the original byte divided by 255. That byte is a percentage, with
        -- 255 meaning that it never misses. Older installs stored 0 or 1, so 0 is treated as 1.
        local accuracy = data.accuracy or 1
        if accuracy <= 0 then accuracy = 1
        else accuracy = math.min(1, math.floor(accuracy * 255 + 0.5) / 100)
        end
        if math.random() > accuracy then
            result.hit = false
            return result
        end
        -- Physical attacks of a blinded attacker miss half of the times.
        if formula == 1 and self:has_status(attacker_side, attacker_index, BattleStatus.DARKNESS) and math.random() < 0.5 then
            result.hit = false
            return result
        end
        local a = self:get_combat_stats(attacker_side, attacker_index)
        local t = self:get_combat_stats(target_side, target_index)
        local amount = 0
        local revives = self:attack_revives(attack)
        if is_item then
            local restores_mp = (data.restore or 0) == 1
            if formula == 2 and modifier == 6 then
                amount = -power * 20
                result.mp = restores_mp
            elseif formula == 2 and modifier == 4 then
                result.mp = restores_mp
                amount = -((restores_mp and t.mp_max or t.hp_max) * power / 32)
            elseif formula == 0 and modifier == 8 then
                -- Elixirs restore all HP and MP.
                amount = -(t.hp_max - t.hp)
                result.extra_mp = -(t.mp_max - t.mp)
            elseif formula == 3 and modifier == 7 then
                amount = power * 20 * (15 + math.random()) / 16
            elseif formula == 2 and modifier == 3 then
                amount = t.hp * power / 32
            end
        elseif formula == 1 then
            local base = a.str + ((a.level + a.str) / 32) * ((a.level * a.str) / 32)
            amount = base * (512 - math.min(t.def, 511)) * power / (16 * 512)
            -- Small characters hit for almost nothing. TODO: Frog and Fury values are guesses.
            if self:has_status(attacker_side, attacker_index, BattleStatus.SMALL) then amount = 1
            elseif self:has_status(attacker_side, attacker_index, BattleStatus.FROG) then amount = amount / 2
            elseif self:has_status(attacker_side, attacker_index, BattleStatus.FURY) then amount = amount * 1.3
            end
        elseif formula == 2 then
            local base = 6 * (a.mag + ((a.mag + a.level) / 32) * ((a.mag * a.level) / 32))
            if revives then
                -- Life and Life2: a fraction of the maximum HP, a quarter if the power is unknown.
                amount = -t.hp_max * (power > 0 and power or 8) / 32
            elseif modifier == 5 then amount = -base * power / 16
            elseif modifier == 3 then amount = (result.mp and t.mp or t.hp) * power / 32
            else amount = base * (512 - math.min(t.mdef, 511)) * power / (16 * 512)
            end
        end
        if amount ~= 0 and modifier ~= 3 and not (is_item and formula ~= 3) then amount = amount * (15 + math.random()) / 16 end
        -- Barriers halve the damage of physical attacks, and magic barriers the one of magic.
        if amount > 0 then
            if formula == 1 and self:has_status(target_side, target_index, BattleStatus.BARRIER) then amount = amount / 2 end
            if formula == 2 and self:has_status(target_side, target_index, BattleStatus.M_BARRIER) then amount = amount / 2 end
            if self:has_status(target_side, target_index, BattleStatus.SADNESS) then amount = amount * 0.7 end
            -- Peerless can't be hurt.
            if self:has_status(target_side, target_index, BattleStatus.PEERLESS) then amount = 0 end
        end
        local factor = self:get_element_factor(attack, target_side, target_index)
        if factor >= 10 then amount = t.hp_max
        elseif factor <= -10 then amount = -t.hp_max
        elseif amount > 0 or factor < 0 then
            -- Absorbed attacks heal, whatever they were doing.
            amount = (amount == 0 and 0 or math.abs(amount)) * factor
        end
        if amount > 0 then result.amount = math.max(1, math.floor(amount))
        elseif amount < 0 then result.amount = math.min(-1, math.ceil(amount))
        end
        if result.extra_mp < 0 then result.extra_mp = math.ceil(result.extra_mp) end
        return result
    end,

    --- Checks if a party slot has a character with data, so it can fight.
    --
    -- @param id Character ID in the party slot, nil if empty.
    -- @return True if the character can be loaded into the battle.
    has_character_data = function(self, id)
        if id == nil then return false end
        if Characters[id] == nil or Characters[id].stats == nil then
            print("Battle: no data for the party character " .. tostring(id) .. ", it won't fight.")
            return false
        end
        return true
    end,

    --- Gets a combatant from its side and index.
    get_combatant = function(self, side, index)
        if side == Battle.TargetGroup.PARTY then return self.party[index] end
        return self.enemies[index]
    end,

    --- Checks if a combatant has a status.
    has_status = function(self, side, index, id)
        local combatant = self:get_combatant(side, index)
        return combatant ~= nil and combatant.status ~= nil and combatant.status[id] ~= nil
    end,

    --- Checks if a status stops the ATB of a combatant.
    is_frozen = function(self, side, index)
        for id, _ in pairs(BattleStatus.FROZEN) do
            if self:has_status(side, index, id) then return true end
        end
        return false
    end,

    --- Gets how fast the ATB of a combatant fills: 1 is normal, 0 is stopped.
    get_atb_rate = function(self, side, index)
        if self:is_frozen(side, index) then return 0 end
        local rate = 1
        if self:has_status(side, index, BattleStatus.HASTE) then rate = rate * 1.5 end
        if self:has_status(side, index, BattleStatus.SLOW) then rate = rate * 0.5 end
        return rate
    end,

    --- Gets the name of a combatant, for messages.
    get_combatant_name = function(self, side, index)
        if side == Battle.TargetGroup.PARTY then return Characters[self.party[index].character].name end
        return self.enemies[index].name
    end,

    --- Gives a status to a combatant, or renews it if it already has it.
    set_status = function(self, side, index, id)
        local combatant = self:get_combatant(side, index)
        local now = timer:get_game_time_total()
        local duration = BattleStatus.DURATION[id]
        if combatant.status[id] == nil then
            print(self:get_combatant_name(side, index) .. " gets " .. (BattleStatus.NAME[id] or tostring(id)))
            -- Good statuses are shown in a different colour than bad ones.
            local good = id == BattleStatus.HASTE or id == BattleStatus.REGEN or id == BattleStatus.BARRIER
              or id == BattleStatus.M_BARRIER or id == BattleStatus.REFLECT or id == BattleStatus.SHIELD
              or id == BattleStatus.DUAL or id == BattleStatus.PEERLESS
            UiContainer.BattleFx:text(side, index, BattleStatus.NAME[id] or tostring(id), good and {0.5, 1, 0.8} or {0.9, 0.5, 1})
        end
        combatant.status[id] = {finish = duration ~= nil and now + duration or nil, next_tick = now + BattleStatus.TICK}
        self:status_changed(side, index)
    end,

    --- Cures a status of a combatant.
    clear_status = function(self, side, index, id)
        local combatant = self:get_combatant(side, index)
        if combatant.status[id] == nil then do return end end
        combatant.status[id] = nil
        print(self:get_combatant_name(side, index) .. " is cured of " .. (BattleStatus.NAME[id] or tostring(id)))
        self:status_changed(side, index)
    end,

    --- Updates what depends on the statuses of a combatant after they change.
    status_changed = function(self, side, index)
        if side == Battle.TargetGroup.PARTY then
            -- Someone that can't act can't choose a command.
            if self:is_frozen(side, index) then self:remove_from_command_queue(index) end
            self:update_name_colours()
        end
    end,

    --- Removes a party member from the queue of members that have to choose a command.
    --
    -- If the member was choosing, the command windows are closed.
    remove_from_command_queue = function(self, index)
        for i = #self.command_queue, 1, -1 do
            if self.command_queue[i] == index then table.remove(self.command_queue, i) end
        end
        if self.current_character_window == index and self.window_state ~= "wait" and self.window_state ~= "result" and self.window_state ~= "gameover" then
            ui_manager:get_widget("BattleUi.Container.Bottom.Target"):set_visible(false)
            ui_manager:get_widget("BattleUi.Container.Bottom.List"):set_visible(false)
            ui_manager:get_widget("BattleUi.Container.Bottom.Commands"):set_visible(false)
            self.current_list = "none"
            self.window_state = "wait"
        end
    end,

    --- Checks if an enemy is immune to a status. Party members have no immunities yet.
    is_immune = function(self, side, index, id)
        if side ~= Battle.TargetGroup.ENEMIES then return false end
        local source = battle_manager:get_enemy(index - 1)
        for i = 0, source:get_immunity_count() - 1 do
            if source:get_immunity(i) == id and math.random() < source:get_immunity_rate(i) then return true end
        end
        return false
    end,

    --- Applies the status effects of an attack to a target.
    --
    -- Effects have a mode (inflict, cure or toggle), and a chance between 0 and 1.
    apply_attack_effects = function(self, attack, side, index)
        for _, effect in ipairs(attack.effects) do
            local chance = math.abs(effect.chance)
            local has = self:has_status(side, index, effect.id)
            if effect.mode == "cure" or (effect.mode == "toggle" and has) then
                if has and math.random() < chance then self:clear_status(side, index, effect.id) end
            elseif not self:is_immune(side, index, effect.id) and math.random() < chance then
                if effect.id == BattleStatus.DEATH then
                    -- Death takes all the HP.
                    local stats = self:get_combat_stats(side, index)
                    if stats.hp > 0 then self:apply_damage(side, index, stats.hp, false) end
                elseif effect.id == BattleStatus.NEAR_DEATH then
                    -- TODO: Near death is set when the HP is low, and has no effect yet.
                else
                    self:set_status(side, index, effect.id)
                end
            end
        end
    end,

    --- Updates the timed statuses: expiration, and the damage of poison and the healing of regen.
    update_statuses = function(self, now)
        for _, side in ipairs({Battle.TargetGroup.PARTY, Battle.TargetGroup.ENEMIES}) do
            local list = side == Battle.TargetGroup.PARTY and self.party or self.enemies
            for index = 1, #list do
                local combatant = list[index]
                if combatant.dead == false and combatant.status ~= nil then
                    local ids = {}
                    for id, _ in pairs(combatant.status) do table.insert(ids, id) end
                    for _, id in ipairs(ids) do
                        local status = combatant.status[id]
                        if status ~= nil then
                            if status.finish ~= nil and now >= status.finish then
                                self:clear_status(side, index, id)
                                -- Some statuses end with something worse.
                                if id == BattleStatus.D_SENTENCE then
                                    local stats = self:get_combat_stats(side, index)
                                    if stats.hp > 0 then self:apply_damage(side, index, stats.hp, false) end
                                elseif id == BattleStatus.SLOW_NUMB then
                                    self:set_status(side, index, BattleStatus.PETRIFY)
                                end
                            elseif (id == BattleStatus.POISON or id == BattleStatus.REGEN) and now >= status.next_tick then
                                status.next_tick = now + BattleStatus.TICK
                                local stats = self:get_combat_stats(side, index)
                                if id == BattleStatus.POISON then
                                    -- Poison can't kill.
                                    local damage = math.min(math.max(1, math.floor(stats.hp_max * BattleStatus.POISON_FRACTION)), stats.hp - 1)
                                    if damage > 0 then self:apply_damage(side, index, damage, false) end
                                else
                                    self:apply_damage(side, index, -math.max(1, math.floor(stats.hp_max * BattleStatus.REGEN_FRACTION)), false)
                                end
                            end
                        end
                    end
                end
            end
        end
    end,

    --- Gets the colour of the name of a party member in the battle window.
    --
    -- Yellow for the one choosing a command, red if it's dead, purple if poisoned, grey if it
    -- can't act.
    -- @return r, g, b.
    get_name_colour = function(self, index, active)
        if self.party[index].dead then return 1, 0, 0 end
        if active then return 1, 1, 0 end
        if self:has_status(Battle.TargetGroup.PARTY, index, BattleStatus.POISON) then return 0.75, 0.4, 1 end
        if self:is_frozen(Battle.TargetGroup.PARTY, index) then return 0.6, 0.6, 0.6 end
        -- Near death: less than a quarter of the HP.
        local stats = Characters[self.party[index].character].stats
        if stats.hp.current < self:get_stat_max(stats.hp) / 4 then return 1, 0.7, 0.2 end
        return 1, 1, 1
    end,

    --- Updates the colours of the party names. The colour has to be set before the text.
    update_name_colours = function(self)
        for c = 1, #self.party do
            local widget = ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Left.Name")
            local active = c == self.current_character_window and self.window_state ~= "wait"
            widget:set_colour(self:get_name_colour(c, active))
            widget:set_text(Characters[Party[c]].name)
        end
    end,

    --- Gets the maximum of a character HP or MP.
    --
    -- Characters start with a max of 65535 until their stats are recalculated, so the base value is
    -- used in that case.
    -- @param stat A character stat with base and max, like stats.hp.
    get_stat_max = function(self, stat)
        if stat.max == nil or stat.max >= 65535 then return stat.base end
        return stat.max
    end,

    --- Updates the HP and MP numbers and lines of a party member.
    --
    -- @param index Index of the party member.
    update_party_bars = function(self, index)
        local stats = Characters[self.party[index].character].stats
        local base = "BattleUi.Container.Bottom.Character" .. tostring(index) .. ".Right."
        ui_manager:get_widget(base .. "Hp.HpCurrent"):set_text(string.format("%5d", stats.hp.current))
        ui_manager:get_widget(base .. "Mp.MpCurrent"):set_text(string.format("%5d", stats.mp.current))
        local hp_max = self:get_stat_max(stats.hp)
        local mp_max = self:get_stat_max(stats.mp)
        -- The lines under the numbers show the proportion of HP and MP left.
        ui_manager:get_widget(base .. "Hp.HpLineCurrent"):set_width(hp_max > 0 and 100 * math.min(1, stats.hp.current / hp_max) or 0, 0)
        ui_manager:get_widget(base .. "Mp.MpLineCurrent"):set_width(mp_max > 0 and 100 * math.min(1, stats.mp.current / mp_max) or 0, 0)
        -- The name changes colour when the HP is low.
        self:update_name_colours()
    end,

    --- Applies damage or healing to a combatant, and handles its death.
    --
    -- @param amount Positive to damage, negative to heal.
    -- @param mp True to change MP instead of HP.
    apply_damage = function(self, side, index, amount, mp)
        UiContainer.BattleFx:number(side, index, amount, mp)
        if side == Battle.TargetGroup.PARTY then
            local char = Characters[self.party[index].character]
            if mp then
                char.stats.mp.current = math.max(0, math.min(self:get_stat_max(char.stats.mp), char.stats.mp.current - amount))
                self:update_party_bars(index)
                print(char.name .. (amount >= 0 and " loses " or " restores ") .. tostring(math.abs(amount)) .. " MP. MP: " .. tostring(char.stats.mp.current))
                return
            end
            char.stats.hp.current = math.max(0, math.min(self:get_stat_max(char.stats.hp), char.stats.hp.current - amount))
            print(char.name .. (amount >= 0 and " takes " .. tostring(amount) .. " damage." or " recovers " .. tostring(-amount) .. " HP.") .. " HP: " .. tostring(char.stats.hp.current))
            self:update_party_bars(index)
            if char.stats.hp.current == 0 then
                self.party[index].dead = true
                self.party[index].atb = 0
                self.party[index].status = {}
                self:update_name_colours()
                for i = #self.command_queue, 1, -1 do
                    if self.command_queue[i] == index then table.remove(self.command_queue, i) end
                end
            end
        else
            local enemy = self.enemies[index]
            if mp then
                enemy.stats.mp = math.max(0, math.min(enemy.stats.mp_max, enemy.stats.mp - amount))
                print(enemy.name .. (amount >= 0 and " loses " or " restores ") .. tostring(math.abs(amount)) .. " MP.")
                return
            end
            enemy.stats.hp = math.max(0, math.min(enemy.stats.hp_max, enemy.stats.hp - amount))
            print(enemy.name .. (amount >= 0 and " takes " .. tostring(amount) .. " damage." or " recovers " .. tostring(-amount) .. " HP.") .. " HP: " .. tostring(enemy.stats.hp))
            if enemy.stats.hp == 0 then
                enemy.dead = true
                enemy.status = {}
                -- The model is hidden once the death animation ends, see update_action.
            end
        end
    end,

    --- Calculates the rewards for winning the battle and gives them to the party.
    --
    -- EXP is split between the party members still alive, AP is given in full to each of them.
    -- Every enemy gives its gil, EXP and AP, dead or not, and may drop items.
    -- TODO: Level ups, kills counter.
    -- @return A list of strings with the results, one per result page.
    apply_rewards = function(self)
        local gil, exp, ap = 0, 0, 0
        local items = {}
        for i = 1, #self.enemies do
            gil = gil + self.enemies[i].money
            exp = exp + self.enemies[i].exp
            ap = ap + self.enemies[i].ap
            local source = battle_manager:get_enemy(i - 1)
            if source ~= nil then
                for d = 0, source:get_drop_count() - 1 do
                    local id = source:get_drop_id(d)
                    if id >= 0 and math.random() < source:get_drop_rate(d) then table.insert(items, id) end
                end
            end
        end
        local alive = {}
        for i = 1, #self.party do
            if self.party[i].dead == false then table.insert(alive, i) end
        end
        local exp_each = 0
        if #alive > 0 then exp_each = math.floor(exp / #alive) end
        for _, i in ipairs(alive) do
            local char = Characters[self.party[i].character]
            char.exp = (char.exp or 0) + exp_each
            if char.exp_to_next ~= nil then
                char.exp_to_next = char.exp_to_next - exp_each
                -- TODO: Level up, using the growth tables.
                if char.exp_to_next <= 0 then print(char.name .. " has enough EXP to level up.") end
            end
            -- AP goes to the materia equipped in the weapon and armor.
            for _, equip in ipairs({char.weapon, char.armor}) do
                if equip ~= nil and equip.materia ~= nil then
                    for m = 0, 7 do
                        local materia = equip.materia[m]
                        if materia ~= nil and Game.Materia[materia.id] ~= nil then
                            local levels = Game.Materia[materia.id].levels_ap
                            local max_ap = levels ~= nil and levels[#levels] or nil
                            materia.ap = (materia.ap or 0) + ap
                            -- Mastered materia stops gaining AP.
                            if max_ap ~= nil and materia.ap > max_ap then materia.ap = max_ap end
                        end
                    end
                end
            end
        end
        Inventory.add_money(gil)

        local pages = {}
        table.insert(pages, "EXP " .. tostring(exp_each) .. "  AP " .. tostring(ap))
        table.insert(pages, tostring(gil) .. " Gil")
        for _, id in ipairs(items) do
            if Game.Items[id] ~= nil then
                Inventory.add_item(id, 1)
                table.insert(pages, "Got " .. Game.Items[id].name)
            end
        end
        print("Battle rewards: " .. tostring(exp_each) .. " EXP, " .. tostring(ap) .. " AP, " .. tostring(gil) .. " gil, " .. tostring(#items) .. " items.")
        return pages
    end,

    --- Shows the next result page, or leaves the battle after the last one.
    show_next_result = function(self)
        self.result_page = self.result_page + 1
        if self.result_pages[self.result_page] == nil then
            self.window_state = "wait"
            self:finish_battle()
            return
        end
        ui_manager:get_widget("BattleUi.Container.Message.Name"):set_text(self.result_pages[self.result_page])
    end,

    --- Leaves the battle, and starts the next one if the formation has any.
    finish_battle = function(self)
        local next_battle = self.pending_next_battle
        self.pending_next_battle = -1
        Battle.finish()
        if next_battle >= 0 then
            print("Next battle: " .. tostring(next_battle))
            Battle.chain(next_battle)
        end
    end,

    --- Checks if the battle is over, and finishes it.
    --
    -- @return True if the battle ended.
    check_battle_end = function(self)
        -- Timed battles end without results when the field timer runs out.
        if Battle.has_mode(Battle.Mode.TIMED) then
            if self.timed_started == nil then self.timed_started = timer:get_timer() > 0 end
            if self.timed_started and timer:get_timer() <= 0 then
                print("Battle timer expired.")
                self.ending = true
                self.action_queue = {}
                self.command_queue = {}
                self.pending_next_battle = -1
                self:finish_battle()
                return true
            end
        end
        local enemies_alive = false
        for i = 1, #self.enemies do if self.enemies[i].dead == false then enemies_alive = true end end
        -- Some scripts end the battle (a boss that leaves, for example).
        if self.ai_end_battle == true then enemies_alive = false end
        local party_alive = false
        -- Petrified members can't fight, so they count as down.
        for i = 1, #self.party do
            if self.party[i].dead == false and not self:has_status(Battle.TargetGroup.PARTY, i, BattleStatus.PETRIFY) then party_alive = true end
        end
        if enemies_alive and party_alive then return false end
        self.ending = true
        self.action_queue = {}
        self.command_queue = {}
        if not enemies_alive and party_alive then
            print("Battle won.")
            if not Battle.has_mode(Battle.Mode.NO_VICTORY_MUSIC) then
                audio_manager:play_music_once(BattleMusic.victory)
            end
            -- Some battles are followed by another one instead of returning to the field.
            self.pending_next_battle = battle_manager:get_next_formation_id()
            local pages = self:apply_rewards()
            if battle_manager:show_victory_pose() then
                for i = 1, #self.party do
                    if self.party[i].dead == false then
                        self:play_script(Battle.TargetGroup.PARTY, i, self:get_script_number(Battle.TargetGroup.PARTY, i, "victory"), true)
                    end
                end
            end
            if battle_manager:show_spoils() and not Battle.has_mode(Battle.Mode.NO_RESULTS) then
                self.result_pages = pages
                self.result_page = 0
                self.window_state = "result"
                ui_manager:get_widget("BattleUi.Container.Bottom.Commands"):set_visible(false)
                ui_manager:get_widget("BattleUi.Container.Bottom.Target"):set_visible(false)
                ui_manager:get_widget("BattleUi.Container.Bottom.List"):set_visible(false)
                self:show_next_result()
                return true
            end
        else
            print("Battle lost.")
            if Battle.has_mode(Battle.Mode.NO_GAME_OVER) then
                self.pending_next_battle = -1
                self:finish_battle()
                return true
            end
            -- The game over screen leaves the battle when it's dismissed.
            self.window_state = "gameover"
            script:request(Script.UI, "GameOver", "show", 0)
            return true
        end
        self:finish_battle()
        return true
    end,

    --- Gets the entity of a combatant.
    --
    -- @param side {@see Battle.TargetGroup.PARTY} or {@see Battle.TargetGroup.ENEMIES}.
    -- @param index Index in the party or enemy list (1 based).
    -- @return The entity, nil if it can't be found.
    get_entity = function(self, side, index)
        local name
        if side == Battle.TargetGroup.PARTY then name = battle_manager:get_party_entity_name(index - 1)
        else name = battle_manager:get_enemy_entity_name(index - 1)
        end
        if name == nil or name == "" then return nil end
        return entity_manager:get_entity(name)
    end,

    --- Gets the animation name for a number in a combatant model.
    get_animation_name = function(self, side, index, number)
        local prefix
        if side == Battle.TargetGroup.PARTY then prefix = battle_manager:get_party_animation_prefix(index - 1)
        else prefix = battle_manager:get_enemy(index - 1):get_model_prefix()
        end
        if prefix == nil or prefix == "" then return nil end
        return string.format("%s_%02d", prefix, number)
    end,

    --- Gets the name of the model of a combatant, as in "aq_mp".
    get_model_name = function(self, side, index)
        if side == Battle.TargetGroup.PARTY then return battle_manager:get_party_model_name(index - 1) end
        return battle_manager:get_enemy(index - 1):get_model()
    end,

    --- Gets the animation script number for a reaction or attack of a combatant.
    --
    -- @param type "attack", "hurt" or "death".
    -- @return The script number, nil if there is none.
    get_script_number = function(self, side, index, type)
        if side == Battle.TargetGroup.ENEMIES then
            if type == "attack" then
                local source = battle_manager:get_enemy(index - 1)
                -- TODO: Use the animation of the attack actually used.
                local count = source:get_animation_count()
                if count > 0 then return source:get_animation(math.random(0, count - 1)) end
                return nil
            end
            return BattleAnimations.enemy[type .. "_script"]
        end
        local mapped = BattleAnimations.models[self:get_model_name(side, index)]
        if mapped ~= nil and mapped[type .. "_script"] ~= nil then return mapped[type .. "_script"] end
        return BattleAnimations.player[type .. "_script"]
    end,

    --- Starts playing an animation script on a combatant.
    --
    -- Animations are played one after the other. If the script marks the moment an attack hits,
    -- on_hit is called then. The queue is advanced by {@see update_animations}.
    -- @param side {@see Battle.TargetGroup.PARTY} or {@see Battle.TargetGroup.ENEMIES}.
    -- @param index Index of the combatant.
    -- @param script Script number.
    -- @param stay True to keep the last frame when finished (deaths).
    -- @param on_hit Function to call when the attack hits, or nil.
    -- @param on_done Function to call when the script is over, or nil.
    -- @param target Combatant to run to, as {side = ..., index = ...}, or nil.
    -- @return True if there was something to play.
    play_script = function(self, side, index, script, stay, on_hit, on_done, target)
        if script == nil then return false end
        -- Scripts can be given as a list of steps too, for animations that are not in the files.
        local steps = script
        if type(script) ~= "table" then steps = BattleAnimations.get_scripts(self:get_model_name(side, index))[script] end
        if steps == nil or #steps == 0 then return false end
        -- A new script replaces the previous one of the combatant.
        local key = tostring(side) .. "_" .. tostring(index)
        local previous = self.animation_queues[key]
        if previous ~= nil and previous.on_done ~= nil then previous.on_done() end
        local entity = self:get_entity(side, index)
        local home = nil
        if entity ~= nil then
            local x, y, z = entity:get_position()
            home = {x = x, y = y, z = z}
            -- If a previous script left the combatant away, its home is the one to go back to.
            if previous ~= nil and previous.home ~= nil then home = previous.home end
        end
        self.animation_queues[key] = {
            side = side, index = index, steps = steps, step = 1, time = timer:get_game_time_total(),
            stay = stay, on_hit = on_hit, on_done = on_done, target = target, home = home,
        }
        return true
    end,

    --- Moves a combatant in a straight line, for the movement steps of a script.
    --
    -- @param q The animation queue of the combatant.
    -- @param to Destination, {x, y, z}.
    start_move = function(self, q, to)
        local entity = self:get_entity(q.side, q.index)
        if entity == nil then do return end end
        local x, y, z = entity:get_position()
        local distance = math.sqrt((to.x - x) ^ 2 + (to.y - y) ^ 2 + (to.z - z) ^ 2)
        local duration = math.max(0.2, distance / BattleAnimations.RUN_SPEED)
        q.move = {from = {x = x, y = y, z = z}, to = to, start = q.time, finish = q.time + duration}
        q.time = q.time + duration
    end,

    --- Advances the animation scripts being played.
    update_animations = function(self, now)
        -- Callbacks can start other scripts, so iterate over a copy of the keys.
        local keys = {}
        for key, _ in pairs(self.animation_queues) do table.insert(keys, key) end
        for _, key in ipairs(keys) do
            local q = self.animation_queues[key]
            if q ~= nil then
                local entity = self:get_entity(q.side, q.index)
                -- A step ends when the time it was scheduled to end arrives.
                while q.step <= #q.steps and now >= q.time do
                    -- A movement that just ended leaves the combatant exactly at its destination.
                    if q.move ~= nil and entity ~= nil then
                        entity:set_position(q.move.to.x, q.move.to.y, q.move.to.z)
                        q.move = nil
                    end
                    local step = q.steps[q.step]
                    if step.type == "anim" then
                        local name = self:get_animation_name(q.side, q.index, step.number)
                        local length = 0
                        if entity ~= nil and name ~= nil then length = entity:get_animation_length(name) end
                        local last = true
                        for i = q.step + 1, #q.steps do if q.steps[i].type == "anim" then last = false end end
                        if entity ~= nil and entity:get_animation_count() > 0 then
                            if q.stay and last then entity:play_animation_stop(name)
                            else entity:play_animation(name)
                            end
                        end
                        -- Real duration, considering the playback speed. Single frame poses last no time.
                        local duration = math.max(length / BattleAnimations.SPEED, BattleAnimations.MIN_FRAME_TIME)
                        q.last_name = name
                        q.last_duration = duration
                        q.time = q.time + duration
                    elseif step.type == "hit" then
                        if q.on_hit ~= nil then q.on_hit() end
                    elseif step.type == "move" then
                        -- Run to a point in front of the target, stopping some distance before it.
                        local target_entity = q.target ~= nil and self:get_entity(q.target.side, q.target.index) or nil
                        if entity ~= nil and target_entity ~= nil then
                            local x, y, z = entity:get_position()
                            local tx, ty, tz = target_entity:get_position()
                            local dx, dy = x - tx, y - ty
                            local length = math.sqrt(dx * dx + dy * dy)
                            if length > 0 then
                                local stop = step.distance / BattleAnimations.DISTANCE_DIVISOR
                                self:start_move(q, {x = tx + dx / length * stop, y = ty + dy / length * stop, z = z})
                            end
                        end
                    elseif step.type == "return" then
                        if q.home ~= nil then self:start_move(q, q.home) end
                    elseif step.type == "face_away" then
                        -- Face away from the enemies.
                        if entity ~= nil then self:face_away(q.index, entity) end
                    elseif step.type == "run_away" then
                        -- Run in a straight line away from the enemies.
                        if entity ~= nil then
                            local x, y, z = entity:get_position()
                            local cx, cy = self:get_enemy_center()
                            local dx, dy = x - cx, y - cy
                            local length = math.sqrt(dx * dx + dy * dy)
                            if length == 0 then dx, dy, length = 0, 1, 1 end
                            self:start_move(q, {x = x + dx / length * step.distance, y = y + dy / length * step.distance, z = z})
                            -- Running away is a one way trip.
                            q.home = nil
                        end
                    end
                    q.step = q.step + 1
                end
                -- While running, move the combatant and keep the run animation going.
                if q.move ~= nil and entity ~= nil and now < q.move.finish then
                    local t = (now - q.move.start) / (q.move.finish - q.move.start)
                    t = math.max(0, math.min(1, t))
                    entity:set_position(
                      q.move.from.x + (q.move.to.x - q.move.from.x) * t,
                      q.move.from.y + (q.move.to.y - q.move.from.y) * t,
                      q.move.from.z + (q.move.to.z - q.move.from.z) * t
                    )
                    if q.last_name ~= nil and (q.loop_next == nil or now >= q.loop_next) then
                        entity:play_animation(q.last_name)
                        q.loop_next = now + math.max(q.last_duration or 0, 0.1)
                    end
                end
                if q.step > #q.steps and now >= q.time and self.animation_queues[key] == q then
                    -- Make sure the combatant ends up where it started.
                    if q.move ~= nil and entity ~= nil then
                        entity:set_position(q.move.to.x, q.move.to.y, q.move.to.z)
                        q.move = nil
                    end
                    if entity ~= nil and q.home ~= nil and not q.stay then
                        entity:set_position(q.home.x, q.home.y, q.home.z)
                    end
                    self.animation_queues[key] = nil
                    if q.on_done ~= nil then q.on_done() end
                end
            end
        end
    end,

    --- Turns an entity to face a direction.
    --
    -- Rotation 0 faces towards -Y, 180 towards +Y and 90 towards +X.
    -- @param dx X component of the direction.
    -- @param dy Y component of the direction.
    face_direction = function(self, entity, dx, dy)
        if dx == 0 and dy == 0 then do return end end
        entity:set_rotation(math.deg(math.atan2(dx, -dy)) % 360)
    end,

    --- Sets the rotation of a party member for the escape run.
    --
    -- The run animation already includes a half turn (the hips are rotated 180 degrees around the
    -- vertical axis), so a character that faces the enemies must not be rotated: the animation
    -- makes it face away. One that already faces away (back attacks) has to be turned around first,
    -- or the animation would make it face the enemies.
    -- @param index Index of the party member.
    -- @param entity Entity of the party member.
    -- @return The rotation set.
    face_away = function(self, index, entity)
        local pose = self.escape_pose[index]
        local original = pose ~= nil and pose.rotation or entity:get_rotation()
        local x, y = entity:get_position()
        local cx, cy = self:get_enemy_center()
        local rotation = original
        if x ~= cx or y ~= cy then
            -- Direction away from the enemies, and how far the original facing is from it.
            local away = math.deg(math.atan2(x - cx, -(y - cy))) % 360
            local difference = ((original - away + 540) % 360) - 180
            if math.abs(difference) <= 90 then rotation = (original + 180) % 360 end
        end
        entity:set_rotation(rotation)
        return rotation
    end,

    --- Makes the party turn their back to the enemies and run in place, while L1 and R1 are held.
    begin_escape_pose = function(self)
        self.escape_pose = {}
        for i = 1, #self.party do
            local key = tostring(Battle.TargetGroup.PARTY) .. "_" .. tostring(i)
            -- Members acting or reacting keep doing it.
            if self.party[i].dead == false and self.animation_queues[key] == nil
              and not (self.current_action ~= nil and self.current_action.side == Battle.TargetGroup.PARTY and self.current_action.action.actor == i)
            then
                local entity = self:get_entity(Battle.TargetGroup.PARTY, i)
                local script = self:get_script_number(Battle.TargetGroup.PARTY, i, "escape")
                local steps = type(script) == "number" and BattleAnimations.get_scripts(self:get_model_name(Battle.TargetGroup.PARTY, i))[script] or nil
                local run = nil
                if steps ~= nil then
                    for _, step in ipairs(steps) do if step.type == "anim" then run = step.number break end end
                end
                if entity ~= nil and run ~= nil then
                    local name = self:get_animation_name(Battle.TargetGroup.PARTY, i, run)
                    self.escape_pose[i] = {rotation = entity:get_rotation(), animation = name, next_time = 0}
                    local facing = self:face_away(i, entity)
                    print(string.format("Escape: %s turns from %d to %d", entity:get_name(), self.escape_pose[i].rotation, facing))
                end
            end
        end
    end,

    --- Keeps the party running in place while the escape pose lasts.
    update_escape_pose = function(self, now)
        for i, pose in pairs(self.escape_pose) do
            local entity = self:get_entity(Battle.TargetGroup.PARTY, i)
            local key = tostring(Battle.TargetGroup.PARTY) .. "_" .. tostring(i)
            -- A member that is acting or reacting at the moment is left alone.
            if self.animation_queues[key] ~= nil then entity = nil end
            if entity ~= nil then self:face_away(i, entity) end
            if entity ~= nil and now >= pose.next_time then
                entity:play_animation(pose.animation)
                pose.next_time = now + math.max(entity:get_animation_length(pose.animation) / BattleAnimations.SPEED, 0.1)
            end
        end
    end,

    --- Makes the party go back to their original position after trying to escape.
    end_escape_pose = function(self)
        for i, pose in pairs(self.escape_pose) do
            local entity = self:get_entity(Battle.TargetGroup.PARTY, i)
            if entity ~= nil then
                entity:set_rotation(pose.rotation)
                -- Back to the idle animation (the first one of the model).
                local idle = self:get_animation_name(Battle.TargetGroup.PARTY, i, 0)
                if idle ~= nil then entity:play_animation(idle) end
            end
        end
        self.escape_pose = {}
    end,

    --- Gets the center of the enemies (X and Y), living or not.
    --
    -- @return X, Y. 0, 0 if there are no enemies.
    get_enemy_center = function(self)
        local sx, sy, count = 0, 0, 0
        for i = 1, #self.enemies do
            local entity = self:get_entity(Battle.TargetGroup.ENEMIES, i)
            if entity ~= nil then
                local x, y, z = entity:get_position()
                sx, sy, count = sx + x, sy + y, count + 1
            end
        end
        if count == 0 then return 0, 0 end
        return sx / count, sy / count
    end,

    --- Escapes from the battle.
    --
    -- Living party members turn around and run away, then the battle ends without rewards.
    start_escape = function(self)
        self.ending = true
        self.action_queue = {}
        self.command_queue = {}
        self.current_action = nil
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.Target"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.List"):set_visible(false)
        self.window_state = "wait"
        local running = 0
        for i = 1, #self.party do
            if self.party[i].dead == false then
                local script = self:get_script_number(Battle.TargetGroup.PARTY, i, "escape")
                local model_steps = type(script) == "number" and BattleAnimations.get_scripts(self:get_model_name(Battle.TargetGroup.PARTY, i))[script] or nil
                if model_steps ~= nil then
                    local steps = {{type = "face_away"}}
                    for _, step in ipairs(model_steps) do table.insert(steps, step) end
                    table.insert(steps, {type = "run_away", distance = BattleAnimations.ESCAPE_DISTANCE})
                    running = running + 1
                    self:play_script(Battle.TargetGroup.PARTY, i, steps, true, nil, function()
                        running = running - 1
                        if running <= 0 then self:finish_battle() end
                    end)
                end
            end
        end
        -- Nobody has an escape animation: leave right away.
        if running == 0 then self:finish_battle() end
    end,

    --- Applies the animation playback speed to every combatant.
    apply_animation_speed = function(self)
        for i = 1, #self.party do
            local entity = self:get_entity(Battle.TargetGroup.PARTY, i)
            if entity ~= nil then entity:set_animation_speed(BattleAnimations.SPEED) end
        end
        for i = 1, #self.enemies do
            local entity = self:get_entity(Battle.TargetGroup.ENEMIES, i)
            if entity ~= nil then entity:set_animation_speed(BattleAnimations.SPEED) end
        end
    end,

    --- Changes the animation speed, for tuning.
    --
    -- @param key "Z" to slow down, "X" to speed up.
    change_animation_speed = function(self, key)
        local speed = BattleAnimations.SPEED + (key == "X" and 0.1 or -0.1)
        BattleAnimations.SPEED = math.max(0.1, math.min(3, speed))
        self:apply_animation_speed()
        ui_manager:get_widget("BattleUi.Container.Message.Name"):set_text(string.format("Animation speed: x%.2f", BattleAnimations.SPEED))
        print("Animation speed: " .. tostring(BattleAnimations.SPEED))
    end,

    --- Starts executing the first action in the queue.
    --
    -- The attacker plays its attack script. The damage is applied when the script marks the hit,
    -- (or at the end, if it doesn't) and the action is over when every animation is done.
    execute_action = function(self)
        if self.action_executing == true or #self.action_queue == 0 then do return 0 end end
        local action = table.remove(self.action_queue, 1)
        self.action_executing = true
        -- Once someone acts, held ATBs start to fill.
        self:release_hold()

        local actor_side = action.actor_side or Battle.TargetGroup.PARTY
        local actors = actor_side == Battle.TargetGroup.PARTY and self.party or self.enemies
        local actor = actors[action.actor]
        local current = {action = action, side = actor_side, hit_done = false, targets = {}, pending = 0}
        self.current_action = current
        -- The actor may have died while waiting.
        if actor == nil or (actor.dead == true and not action.allow_dead) then
            current.hit_done = true
            self:update_action(timer:get_game_time_total())
            return
        end
        local attack = self:get_action_attack(action)
        local revives = self:attack_revives(attack)
        -- Attacks that revive are for the dead, the other ones for the living.
        local function valid(combatant) return combatant ~= nil and (combatant.dead == revives) end
        local target_list = action.target_group == Battle.TargetGroup.PARTY and self.party or self.enemies
        if action.target_list ~= nil then
            -- Targets chosen by an AI script.
            for _, target in ipairs(action.target_list) do
                if valid(target_list[target]) then table.insert(current.targets, target) end
            end
        elseif action.target_index ~= nil then
            if valid(target_list[action.target_index]) then
                table.insert(current.targets, action.target_index)
            elseif not revives then
                -- The target died: retarget to the first living one of the same group.
                for i = 1, #target_list do
                    if valid(target_list[i]) then table.insert(current.targets, i) break end
                end
            end
        else
            for i = 1, #target_list do
                if valid(target_list[i]) then table.insert(current.targets, i) end
            end
        end
        local script = action.script
        if script == nil and actor_side == Battle.TargetGroup.PARTY then
            script = self:get_script_number(actor_side, action.actor, (action.attack_id ~= nil or action.item_id ~= nil) and "magic" or "attack")
        end
        -- Using things costs something. If it can't be paid anymore, the action fizzles.
        local function fizzle(message)
            print(message)
            current.hit_done = true
            current.targets = {}
            current.pending = 0
            self:update_action(timer:get_game_time_total())
        end
        if actor_side == Battle.TargetGroup.PARTY then
            local stats = Characters[self.party[action.actor].character].stats
            if action.item_id ~= nil then
                -- Items are used up.
                local owned = 0
                for slot = 0, MAX_INVENTORY_SLOTS do
                    if Inventory[slot] ~= nil and Inventory[slot].item == action.item_id then owned = owned + Inventory[slot].quantity end
                end
                if owned < 1 then fizzle("No more " .. attack.name) return end
                Inventory.remove_item(action.item_id, 1)
            elseif action.attack_id ~= nil then
                if attack.mp > 0 then
                    if stats.mp.current < attack.mp then fizzle("Not enough MP for " .. attack.name) return end
                    self:apply_damage(actor_side, action.actor, attack.mp, true)
                end
                if action.summon ~= nil then
                    local used = self.party[action.actor].summon_used[action.attack_id] or 0
                    if (action.summon.times or 1) < 255 then
                        if used >= (action.summon.times or 1) then fizzle("No summons left of " .. attack.name) return end
                        self.party[action.actor].summon_used[action.attack_id] = used + 1
                    end
                end
            end
        end
        -- Spells, summons, items and enemy attacks show their names.
        if action.attack_id ~= nil or action.item_id ~= nil then
            ui_manager:get_widget("BattleUi.Container.Message.Name"):set_text(attack.name)
            UiContainer.BattleFx:cast(attack, actor_side, action.actor)
        end
        current.pending = 1
        local played = self:play_script(
          actor_side, action.actor, script, false,
          function() self:apply_action_hit() end,
          function() current.pending = current.pending - 1 end,
          current.targets[1] ~= nil and {side = action.target_group, index = current.targets[1]} or nil
        )
        if not played then
            -- Nothing to play for the attacker: the hit lands right away.
            current.pending = 0
        end
        self:update_action(timer:get_game_time_total())
    end,

    --- Applies the damage of the current action, and starts the reactions of the targets.
    --- Brings a dead combatant back to life.
    --
    -- It comes back with 1 HP at least, the attack heals it afterwards.
    revive = function(self, side, index)
        local combatant = self:get_combatant(side, index)
        if combatant == nil or combatant.dead == false then do return end end
        combatant.dead = false
        combatant.atb = 0
        combatant.status = {}
        combatant.waiting = false
        combatant.acting = false
        if side == Battle.TargetGroup.PARTY then
            Characters[combatant.character].stats.hp.current = 1
            self:update_party_bars(index)
            self:update_name_colours()
        else
            combatant.stats.hp = 1
            battle_manager:set_enemy_visible(index - 1, true)
        end
        -- Back to idle, it was lying on the floor.
        local entity = self:get_entity(side, index)
        local name = self:get_animation_name(side, index, 0)
        if entity ~= nil and name ~= nil then entity:play_animation(name) end
        print(self:get_combatant_name(side, index) .. " is back")
    end,

    --- Picks a random living combatant of a side, nil if there is none.
    random_living = function(self, side)
        local list = side == Battle.TargetGroup.PARTY and self.party or self.enemies
        local alive = {}
        for i = 1, #list do if list[i].dead == false then table.insert(alive, i) end end
        if #alive == 0 then return nil end
        return alive[math.random(1, #alive)]
    end,

    apply_action_hit = function(self)
        local current = self.current_action
        if current == nil or current.hit_done == true then do return end end
        current.hit_done = true
        local action = current.action
        local attack = self:get_action_attack(action)
        local revives = self:attack_revives(attack)
        for _, t0 in ipairs(current.targets) do
            local target_group = action.target_group
            local t = t0
            -- Reflect bounces spells to the other side.
            if attack.reflectable and self:has_status(target_group, t, BattleStatus.REFLECT) then
                local other = target_group == Battle.TargetGroup.PARTY and Battle.TargetGroup.ENEMIES or Battle.TargetGroup.PARTY
                local bounce = self:random_living(other)
                if bounce ~= nil then
                    print(attack.name .. " is reflected")
                    UiContainer.BattleFx:text(target_group, t, "Reflect", {0.7, 0.9, 1})
                    target_group, t = other, bounce
                end
            end
            if revives then self:revive(target_group, t) end
            local result = self:calculate_attack(attack, current.side, action.actor, target_group, t)
            if not result.hit then
                print("Miss")
                UiContainer.BattleFx:text(target_group, t, "Miss", UiContainer.BattleFx.COLOUR_MISS)
            else
                UiContainer.BattleFx:hit(attack, target_group, t, result)
                -- Status effects of the attack.
                self:apply_attack_effects(attack, target_group, t)
            end
            if result.hit and result.amount ~= 0 then
                self:apply_damage(target_group, t, result.amount, result.mp)
                -- Damage wakes up sleepers.
                if result.amount > 0 and not result.mp then self:clear_status(target_group, t, BattleStatus.SLEEP) end
                local data = attack.damage
                if result.amount > 0 and (data.drain_hp or 0) == 1 and not result.mp then
                    self:apply_damage(current.side, action.actor, -result.amount, false)
                end
            end
            -- Elixirs restore MP too.
            if result.hit and (result.extra_mp or 0) ~= 0 then self:apply_damage(target_group, t, result.extra_mp, true) end
            local dead = (target_group == Battle.TargetGroup.PARTY and self.party[t].dead)
              or (target_group == Battle.TargetGroup.ENEMIES and self.enemies[t].dead)
            -- The AI of enemies can counterattack.
            if target_group == Battle.TargetGroup.ENEMIES and result.hit and attack.id ~= nil then
                self:ai_after_hit(attack, current.side, action.actor, t, dead)
            end
            -- Only damage makes the target react.
            local reaction = nil
            if dead then reaction = "death"
            elseif result.hit and result.amount > 0 and not result.mp then reaction = "hurt"
            end
            if reaction ~= nil then
                current.pending = current.pending + 1
                local played = false
                if not (dead and target_group == Battle.TargetGroup.ENEMIES) then
                    played = self:play_script(
                      target_group, t, self:get_script_number(target_group, t, reaction), dead, nil,
                      function() current.pending = current.pending - 1 end
                    )
                end
                if not played then current.pending = current.pending - 1 end
            end
            if dead and target_group == Battle.TargetGroup.ENEMIES then
                -- Enemies have no death animation: they disappear a moment after dying.
                table.insert(self.pending_hides, {side = target_group, index = t, time = timer:get_game_time_total() + BattleAnimations.ENEMY_DEATH_TIME})
                current.pending = current.pending + 1
                self.pending_hides[#self.pending_hides].on_done = function() current.pending = current.pending - 1 end
            end
        end
    end,

    --- Finishes the action being executed once all its animations are over.
    update_action = function(self, now)
        local current = self.current_action
        if current == nil then do return end end
        -- If the script didn't mark a hit, it lands when the attacker finishes.
        if current.hit_done == false and current.pending <= 0 then self:apply_action_hit() end
        if current.hit_done == false or current.pending > 0 then do return end end

        local action = current.action
        -- Reset the actor ATB so it starts filling again.
        local actors = current.side == Battle.TargetGroup.PARTY and self.party or self.enemies
        local actor = actors[action.actor]
        -- Counterattacks happen outside the turn of the enemy, so they don't change its ATB.
        if actor ~= nil and not action.is_counter then
            actor.atb = 0
            actor.waiting = false
            actor.acting = false
            if current.side == Battle.TargetGroup.PARTY then
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(action.actor) .. ".Right.AtbFill"):set_width(0, 0)
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(action.actor) .. ".Right.AtbFill"):set_colour(1, 1, 0)
            end
        end
        self.current_action = nil
        self.action_executing = false
        self:check_battle_end()
    end,

    --- Hides the enemies whose death time is over.
    update_hides = function(self, now)
        for i = #self.pending_hides, 1, -1 do
            local hide = self.pending_hides[i]
            if now >= hide.time then
                battle_manager:set_enemy_visible(hide.index - 1, false)
                table.remove(self.pending_hides, i)
                if hide.on_done ~= nil then hide.on_done() end
            end
        end
    end,

    --- Rotates the combatant selected in the animation viewer 45 degrees, to find out which
    -- angle faces which direction. Shows the new angle and position.
    viewer_rotate = function(self)
        local list = {}
        for i = 1, #self.party do table.insert(list, {side = Battle.TargetGroup.PARTY, index = i}) end
        for i = 1, #self.enemies do table.insert(list, {side = Battle.TargetGroup.ENEMIES, index = i}) end
        if #list == 0 then do return end end
        if self.viewer.combatant > #list then self.viewer.combatant = 1 end
        local view = list[self.viewer.combatant]
        local entity = self:get_entity(view.side, view.index)
        if entity == nil then do return end end
        self.viewer.rotation = ((self.viewer.rotation or entity:get_rotation()) + 45) % 360
        entity:set_rotation(self.viewer.rotation)
        local x, y, z = entity:get_position()
        local text = string.format("%s rotation %d at (%.1f, %.1f)", entity:get_name(), self.viewer.rotation, x, y)
        ui_manager:get_widget("BattleUi.Container.Message.Name"):set_text(text)
        print("Rotation viewer: " .. text)
    end,

    --- Animation viewer, to find out what each animation of a model is.
    --
    -- V selects the next combatant (party, then enemies), N and B play the next and previous
    -- animations. The animation name and number are shown in the top window.
    -- @param key "V", "N" or "B".
    viewer_input = function(self, key)
        local list = {}
        for i = 1, #self.party do table.insert(list, {side = Battle.TargetGroup.PARTY, index = i}) end
        for i = 1, #self.enemies do table.insert(list, {side = Battle.TargetGroup.ENEMIES, index = i}) end
        if #list == 0 then do return end end
        if self.viewer.combatant > #list then self.viewer.combatant = 1 end
        local view = list[self.viewer.combatant]
        if key == "V" then
            self.viewer.combatant = self.viewer.combatant % #list + 1
            view = list[self.viewer.combatant]
            self.viewer.animation = 0
            self.viewer.rotation = nil
        end
        local entity = self:get_entity(view.side, view.index)
        if entity == nil then do return end end
        local total = entity:get_animation_count()
        if total == 0 then do return end end
        if key == "N" then self.viewer.animation = (self.viewer.animation + 1) % total
        elseif key == "B" then self.viewer.animation = (self.viewer.animation - 1) % total
        end
        local name = self:get_animation_name(view.side, view.index, self.viewer.animation)
        local length = entity:get_animation_length(name)
        entity:play_animation(name)
        ui_manager:get_widget("BattleUi.Container.Message.Name"):set_text(
          name .. " " .. tostring(self.viewer.animation + 1) .. "/" .. tostring(total) .. string.format(" %.1fs x%.2f", length / BattleAnimations.SPEED, BattleAnimations.SPEED)
        )
        print("Animation viewer: " .. name .. " (" .. tostring(self.viewer.animation) .. " of " .. tostring(total) .. "), " .. tostring(length) .. "s")
    end,

    --- Shows a list of actions.
    --
    -- Can be used for the windows of magic, summons, items...
    -- @param list The list to open, as in {@see Battle.Menu}
    show_list = function(self, list)
        local cursor = ui_manager:get_widget("BattleUi.Container.Bottom.List.Cursor")
        local kind = nil
        if list == Battle.Menu.MAGIC then kind = "magic"
        elseif list == Battle.Menu.SUMMON then kind = "summon"
        elseif list == Battle.Menu.ITEM then kind = "item"
        end
        if kind ~= nil then
            audio_manager:play_sound("Cursor")
            self.cursor_list = 1
            self.first_list = 1
            self.current_list = kind
            self.list_entries = self:build_list_entries(kind)
            -- Start where the list was left, if the cursor memory is on.
            if UiContainer.cursor_memory() then
                local remembered = (self.memory_list[self.current_character_window] or {})[kind]
                if remembered ~= nil and remembered.first + remembered.cursor - 1 <= #self.list_entries then
                    self.first_list = remembered.first
                    self.cursor_list = remembered.cursor
                end
            end
            self:populate_list()
            self.window_state = "list"
            ui_manager:get_widget("BattleUi.Container.Bottom.List"):set_visible(true)
            cursor:set_default_animation("Position" .. tostring(self.cursor_list))
            cursor:set_visible(true)
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

    --- Remembers the position of the cursor in the current list, for the cursor memory.
    remember_list = function(self)
        local slot = self.current_character_window
        if self.current_list == "none" or slot == nil then return end
        self.memory_list[slot] = self.memory_list[slot] or {}
        self.memory_list[slot][self.current_list] = { first = self.first_list, cursor = self.cursor_list }
    end,

    --- Closes the currently opened list
    close_list = function(self)
        self:remember_list()
        self.current_list = "none"
        self.window_state = "command"
        ui_manager:get_widget("BattleUi.Container.Bottom.List"):set_visible(false)
    end,

    --- Shows the target selection window.
    --
    -- The elements displayed in the window and the navegability will depend on what is set in
    -- {@see target_options}.
    select_target = function(self)
        local entry = nil
        if self.window_state == "list" then entry = self.list_entries[self.cursor_list + self.first_list - 1] end
        if entry == nil then do return end end
        self:remember_list()
        -- Spells, summons and items that can't be used (not enough MP, no summons left...).
        if not entry.enabled then
            audio_manager:play_sound("Error")
            do return end
        end
        local action = nil
        if entry.item_id ~= nil then action = Game.Items[entry.item_id]
        else action = Game.Attacks[entry.attack_id]
        end
        self.selected_attack = entry.attack_id
        self.selected_item = entry.item_id
        self.selected_summon = entry.summon
        self.target_options = action.target
        if self.target_options.default_multiple == 1 then self.target_type = Battle.TargetType.GROUP
        else self.target_type = Battle.TargetType.SINGLE
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
        self:refresh_target_enemies()
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
        self:populate_target_list_base()
        self:update_all_cursors()
    end,

    --- Shows one hand cursor for each target when all the targets of a side are selected.
    --
    -- The target window has four of them for each side, but there can be fewer targets.
    update_all_cursors = function(self)
        if self.target_options == nil then do return end end
        local left = #self.party
        -- The left column has the enemies, unless the attack is only for the party.
        if not (self.target_options.fixed_row == 1 and self.target_options.default_enemy == 0) then
            left = math.max(0, #self.target_enemies - self.first_target + 1)
        end
        local right = #self.party
        for i = 1, 4 do
            ui_manager:get_widget("BattleUi.Container.Bottom.Target.CursorAllLeft.CursorAllLeft" .. tostring(i)):set_visible(i <= left)
            ui_manager:get_widget("BattleUi.Container.Bottom.Target.CursorAllRight.CursorAllRight" .. tostring(i)):set_visible(i <= right)
        end
    end,

    --- Populates the target selection window (see {@see populate_target_list}).
    populate_target_list_base = function(self)
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
                if self.target_enemies[e + self.first_target - 1] ~= nil then
                    ui_manager:get_widget("BattleUi.Container.Bottom.Target.L" .. tostring(e)):set_text(self.enemies[self.target_enemies[e + self.first_target - 1]].name)
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
        self:populate_list()
    end,

    --- Builds the entries of the list window: spells, summons or items.
    --
    -- Each entry has a name, if it can be used (enabled), and what it refers to: attack_id for
    -- spells and summons (and summon, with its data), item_id and quantity for items.
    -- @param kind "magic", "summon" or "item".
    build_list_entries = function(self, kind)
        local entries = {}
        local member = self.party[self.current_character_window]
        local stats = Characters[member.character].stats
        if kind == "magic" then
            for _, spell in ipairs(member.magic) do
                local data = Game.Attacks[spell.id]
                table.insert(entries, {name = data.name, attack_id = spell.id, enabled = data.mp <= stats.mp.current})
            end
        elseif kind == "summon" then
            for _, summon in ipairs(member.summon) do
                local data = Game.Attacks[summon.id]
                -- Summons can be used a number of times per battle. 255 times is no limit.
                local used = member.summon_used[summon.id] or 0
                local left = (summon.times or 1) - used
                table.insert(entries, {
                    name = data.name, attack_id = summon.id, summon = summon,
                    enabled = data.mp <= stats.mp.current and (left > 0 or (summon.times or 0) >= 255),
                })
            end
        elseif kind == "item" then
            for slot = 0, MAX_INVENTORY_SLOTS do
                local entry = Inventory[slot]
                if entry ~= nil and entry.quantity > 0 then
                    local item = Game.Items[entry.item]
                    -- Only the items that can be used in battle (weapons and armor can't).
                    if item ~= nil and item.battle == 1 then
                        table.insert(entries, {name = item.name .. " " .. tostring(entry.quantity), item_id = entry.item, enabled = true})
                    end
                end
            end
        end
        return entries
    end,

    --- Shows the entries of the list window.
    populate_list = function(self)
        for pos = 1, 4 do
            local widget = ui_manager:get_widget("BattleUi.Container.Bottom.List.List" .. tostring(pos))
            local entry = self.list_entries[pos + self.first_list - 1]
            if entry == nil then
                widget:set_visible(false)
            else
                -- Entries that can't be used are greyed out. The colour has to be set before the
                -- text, as it's applied to it when it's set.
                if entry.enabled then widget:set_colour(1, 1, 1) else widget:set_colour(0.5, 0.5, 0.5) end
                widget:set_text(entry.name)
                widget:set_visible(true)
            end
        end
    end,

    --- Populates the summon selction window.
    --
    -- It uses the summons the current character can use.
    populate_summon_list = function(self)
        self:populate_list()
    end,

    --- Load enemies
    load_enemies = function(self)
        local total = battle_manager:get_enemy_count()
        for e = 0, total - 1 do
            local enemy_src = battle_manager:get_enemy(e)
            enemy = {}
            enemy.dead = false
            enemy.status = {}
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
            if self:has_character_data(Party[i]) then
                self.party[i] = {}
                self.party[i].character = Party[i]
                --math.randomseed(('' .. os.time()):reverse())
                -- TODO: Consider battle layouts.
                -- TODO: Check random seed
                self.party[i].dead = false
                self.party[i].status = {}
                self.party[i].acting = false
                self.party[i].summon_used = {}
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
        -- Reset the state of the previous battle.
        self.party = {}
        self.enemies = {}
        self.action_queue = {}
        self.command_queue = {}
        self.action_executing = false
        self.window_state = "wait"
        self.current_list = "none"
        self.ending = false
        self.timed_started = nil
        self.result_pages = {}
        self.result_page = 0
        self.pending_next_battle = -1
        self.current_action = nil
        self.animation_queues = {}
        self.pending_hides = {}
        self.attack_cache = {}
        self.viewer = {combatant = 1, animation = 0}
        self:populate()
        ui_manager:get_widget("BattleUi"):set_visible(true)
        self.active = true;

        -- Lopad combatant data
        self:load_party_members()
        self:load_enemies()
        self:ai_setup()
        self:init_atb()
        self:apply_animation_speed()
        UiContainer.BattleFx:show()
        -- The transition from the field ends in white.
        UiContainer.BattleFx:flash({1, 1, 1}, 1.0, 0.7)
        -- Battle music. The music of the field continues when the battle ends.
        -- The scripts of the field choose one of the music tracks of the field for the battles
        -- (the BMUSC opcode of the original game). Without one, the default battle music plays.
        local track = BattleMusic.next
        if track == nil then
            local field_track = audio_manager:get_battle_track_id()
            if field_track >= 0 then
                local music = audio_manager:get_track_id(field_track)
                if music ~= -1 then track = tostring(music) end
            end
        end
        BattleMusic.next = nil
        audio_manager:play_music(track or BattleMusic.battle)
        for i = 1, #self.party do self:update_party_bars(i) end
        ui_manager:get_widget("BattleUi.Container.Message.Name"):set_text("V: target  N/B: anim  F: rotate  C: camera  Z/X: speed  K: end")

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
            local widget = ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(c) .. ".Left.Name")
            widget:set_colour(self:get_name_colour(c, c == party_id))
            widget:set_text(Characters[Party[c]].name)
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
        -- Start at the command chosen last time, if the cursor memory is on.
        if UiContainer.cursor_memory() then
            local remembered = self.memory_command[party_id]
            if remembered ~= nil and remembered <= total_commands then self.cursor_command = remembered end
        end
        -- Set and show cursor
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_default_animation("Position" .. tostring(self.cursor_command))
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_visible(true)
        self.window_state = "command"

    end,

    --- Hides the battle UI.
    --
    -- It also sets this widget to inactive.
    hide = function(self)
        UiContainer.BattleFx:hide()
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
            if self:has_character_data(Party[c]) then
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
