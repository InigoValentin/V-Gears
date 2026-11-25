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
        magic_script = 29,
        -- Script 44 (45 is the same): ready, hit mark and the throwing animation 21, shared by all the party.
        throw_script = 44,
        -- Script 32 (33 is the same with animation 10 instead of 9): the pose of the commands that don't hit
        -- (Manipulate). The hit lands when it ends.
        manip_script = 32,
        -- Script 30 is the same as 29 with the pink aura of the enemy skills.
        eskill_script = 30},
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
    DEATH = 0, NEAR_DEATH = 1, SLEEP = 2, POISON = 3, SADNESS = 4, FURY = 5, CONFUSION = 6,
    SILENCE = 7, HASTE = 8, SLOW = 9, STOP = 10, FROG = 11, SMALL = 12, SLOW_NUMB = 13,
    PETRIFY = 14, REGEN = 15, BARRIER = 16, M_BARRIER = 17, REFLECT = 18, DUAL = 19, SHIELD = 20,
    D_SENTENCE = 21, MANIPULATE = 22, BERSERK = 23, PEERLESS = 24, PARALYSIS = 25, DARKNESS = 26,
}

BattleStatus.NAME = {
    [0] = "Death", "Near-death", "Sleep", "Poison", "Sadness", "Fury", "Confusion", "Silence",
    "Haste", "Slow", "Stop", "Frog", "Small", "Slow-numb", "Petrify", "Regen", "Barrier",
    "M.Barrier", "Reflect", "Dual", "Shield", "D.Sentence", "Manipulate", "Berserk", "Peerless",
    "Paralysis", "Darkness",
}

--- Time units each timed status lasts (the table of ff7.exe at 0x7ad3e0). The other statuses last until they
-- are cured, the character is KO'd or the battle ends. How long a unit is depends on the battle speed and on
-- haste and slow, see {@see get_status_speed}.
BattleStatus.UNITS = {
    [BattleStatus.STOP] = 30, [BattleStatus.PARALYSIS] = 20, [BattleStatus.D_SENTENCE] = 60,
    [BattleStatus.SLOW_NUMB] = 30, [BattleStatus.BARRIER] = 127, [BattleStatus.M_BARRIER] = 127,
    [BattleStatus.SLEEP] = 100, [BattleStatus.REGEN] = 127, [BattleStatus.SHIELD] = 64, [BattleStatus.PEERLESS] = 64,
}

--- Statuses that stop the ATB, so the combatant can't act.
BattleStatus.FROZEN = {
    [BattleStatus.SLEEP] = true, [BattleStatus.STOP] = true, [BattleStatus.PETRIFY] = true,
    [BattleStatus.PARALYSIS] = true,
}

--- Statuses that each status removes when it is inflicted (ff7.exe: the pair of the table at 0x7ad628 for Haste
-- and Slow, and the gain of Petrify at 0x433fe0). Fury and Sadness are handled with their items.
BattleStatus.REMOVES = {
    [BattleStatus.HASTE] = {BattleStatus.SLOW}, [BattleStatus.SLOW] = {BattleStatus.HASTE},
    [BattleStatus.PETRIFY] = {BattleStatus.SLEEP, BattleStatus.POISON, BattleStatus.CONFUSION, BattleStatus.REGEN},
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
                elseif (fn == "set_camera" or fn == "set_formation_camera") and tonumber(args) ~= nil
                  -- Some models have scripts that are not real ones (they don't end).
                  and body:find("return 0", 1, true) then
                    table.insert(steps, {type = fn == "set_camera" and "camera" or "view", number = tonumber(args)})
                end
                -- TODO: set_direction, effects, sounds...
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
    -- Every battle action gets added to the end of this queue, and the actions are executed in
    -- order. ff7.exe does not sort the queue (its ring buffer at 0x99c420 is only appended to), so
    -- there are no priorities. The exception is the counterattacks of enemies, which go to the
    -- front: see {@see ai_action}. Actions for enemies and allies are stored here.
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

    --- Rows of the list window, see {@see build_list_rows}: {entries = {...}, tag = text}.
    list_entries = {},

    --- Item, spell or summon selected to be used.
    selected_item = nil,
    selected_summon = nil,
    selected_links = nil,
    selected_limit = nil,
    selected_throw = nil,
    selected_coin = nil,

    --- Data of the basic physical attack ("Attack" command).
    --
    -- Party members use {@see get_weapon_attack} instead.
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

    --- Column of the cursor in the magic list, which has several spells in each row.
    cursor_col = 1,

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
    -- @param button Game event (the directions, OK, cancel...) or, for the debug keys, the name of
    -- the key.
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
                self:show_message("Camera " .. tostring(self.camera_index + 1) .. "/" .. tostring(count))
            end
        end
        -- Escape: L1 and R1 must be held together. Release events are not received, so
        -- the key repeat events are used to know if they are still held.
        if button == Config.ACTIONS.L1 or button == Config.ACTIONS.R1 then
            self.escape_seen[button] = timer:get_game_time_total()
        end
        -- Handle command cursors
        if self.window_state == "command" then
            local command_count = #self:get_command_list(self.current_character_window)
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
                if self.cursor_command > 4 then
                    self.cursor_command = self.cursor_command - 4
                    cursor:set_default_animation("Position" .. tostring(self.cursor_command))
                    audio_manager:play_sound("Cursor")
                else
                    -- Left of the first column is the Change window.
                    self:open_side_window("change")
                end
            elseif button == Config.CONTROLS.RIGHT then
                if self.cursor_command + 4 <= command_count then
                    self.cursor_command = self.cursor_command + 4
                    cursor:set_default_animation("Position" .. tostring(self.cursor_command))
                    audio_manager:play_sound("Cursor")
                else
                    -- Right of the last column is the Defend window.
                    self:open_side_window("defend")
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
            elseif button == Config.ACTIONS.CONFIRM and self.party[self.current_character_window].manip_commands ~= nil
              and self:has_status(Battle.TargetGroup.PARTY, self.current_character_window, BattleStatus.FROG) then
                audio_manager:play_sound("Error")
            elseif button == Config.ACTIONS.CONFIRM and self.party[self.current_character_window].manip_commands ~= nil then
                -- The commands are the attacks of the manipulated enemy.
                self:select_manip_attack(self.cursor_command)
            elseif button == Config.ACTIONS.CONFIRM then
                self.memory_command[self.current_character_window] = self.cursor_command
                -- Check if a menu must be shown
                local cmd = Game.Commands[self.party[self.current_character_window].command[self.cursor_command]]
                if cmd == nil then
                    print("DEBUG: no command at the cursor " .. tostring(self.cursor_command) .. " of " .. tostring(self.current_character_window))
                    return 0
                end
                local command_id = self.party[self.current_character_window].command[self.cursor_command]
                if self:is_frog_blocked(self.current_character_window, command_id) then
                    audio_manager:play_sound("Error")
                    return 0
                end
                if command_id == self.MANIP_COMMAND and self.party[self.current_character_window].manipulating ~= nil then
                    -- Nobody can manipulate an enemy while manipulating another one.
                    audio_manager:play_sound("Error")
                    return 0
                end
                if cmd.menu == Battle.Menu.NONE and not self.ENEMY_COMMANDS[command_id] then
                    if not self:queue_direct_command(self.party[self.current_character_window].command[self.cursor_command]) then
                        print("DIRECT COMMAND")
                    end
                elseif cmd.menu == Battle.Menu.COIN then
                    self:open_coin_window()
                elseif cmd.menu == Battle.Menu.TARGET or self.ENEMY_COMMANDS[command_id] then
                    audio_manager:play_sound("Cursor")
                    self:select_command_target(self.party[self.current_character_window].command[self.cursor_command])
                elseif self.SILENCED_COMMANDS[command_id] and self:has_status(Battle.TargetGroup.PARTY, self.current_character_window, BattleStatus.SILENCE) then
                    -- Silenced characters can't cast, summon nor use enemy skills.
                    audio_manager:play_sound("Error")
                else
                    self.selected_command = self.party[self.current_character_window].command[self.cursor_command]
                    self.w_first = nil
                    self:show_list(cmd.menu)
                end
            end
        -- Coin: the amount of gil is chosen with the directions.
        elseif self.window_state == "coin" then
            self:handle_coin_button(button)
        -- Change (on the left of the commands) and Defend (on the right) have a window with one option.
        elseif self.window_state == "change" or self.window_state == "defend" then
            local kind = self.window_state
            if button == Config.ACTIONS.CANCEL or (kind == "change" and button == Config.CONTROLS.RIGHT)
              or (kind == "defend" and button == Config.CONTROLS.LEFT) then
                audio_manager:play_sound("Cursor")
                self:close_side_window()
            elseif button == Config.ACTIONS.CONFIRM then
                audio_manager:play_sound("Cursor")
                self:queue_side_command(kind == "change" and 18 or 19)
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
                else
                    -- Scroll down
                    audio_manager:play_sound("Cursor")
                    self.first_list = self.first_list + 1
                    self:repopulate_list()
                end
                self:update_list_cursor()
            elseif button == Config.CONTROLS.UP then
                if self.cursor_list == 1 and self.first_list == 1 then do return 0 end end
                if self.cursor_list > 1 then
                    -- Move cursor up
                    audio_manager:play_sound("Cursor")
                    self.cursor_list = self.cursor_list - 1
                else
                    -- Scroll up
                    audio_manager:play_sound("Cursor")
                    self.first_list = self.first_list - 1
                    self:repopulate_list()
                end
                self:update_list_cursor()
            elseif button == Config.CONTROLS.LEFT or button == Config.CONTROLS.RIGHT then
                -- The rows of the magic list have several spells.
                local row = self.list_entries[self.cursor_list + self.first_list - 1]
                local column = self.cursor_col + (button == Config.CONTROLS.LEFT and -1 or 1)
                if row ~= nil and column >= 1 and column <= #row.entries then
                    audio_manager:play_sound("Cursor")
                    self.cursor_col = column
                    self:update_list_cursor()
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

    --- Distance a character moves to change the row, in world units (about the size of the character).
    -- TODO: Estimated, tune it here.
    ROW_STEP = 3.0,

    --- Seconds a row change lasts.
    ROW_MOVE_TIME = 0.4,

    --- Distance in front of the covered character where the one that covers stands, and seconds it takes
    -- to get there and back.
    COVER_DISTANCE = 2.5,
    COVER_MOVE_TIME = 0.25,

    --- Slides a party member to a position, replacing the slide it was doing.
    --
    -- @param index Index in the party.
    -- @param to Destination, {x, y, z}.
    -- @param duration Seconds it takes.
    slide_party = function(self, index, to, duration)
        local entity = self:get_entity(Battle.TargetGroup.PARTY, index)
        if entity == nil then return end
        for i = #self.row_moves, 1, -1 do
            if self.row_moves[i].index == index then table.remove(self.row_moves, i) end
        end
        local x, y, z = entity:get_position()
        local now = timer:get_game_time_total()
        table.insert(self.row_moves, {index = index, from = {x = x, y = y, z = z}, to = to, start = now, finish = now + duration})
    end,

    --- Slides a party member back to its place once its animations are over (the end of an animation puts
    -- the character where the animation started, which would undo the slide).
    --
    -- @param index Index in the party.
    -- @param home Place to go back to, {x, y, z}.
    return_to_place = function(self, index, home, since)
        local now = timer:get_game_time_total()
        since = since or now
        local key = tostring(Battle.TargetGroup.PARTY) .. "_" .. tostring(index)
        -- It doesn't wait more than two seconds.
        if self.animation_queues[key] ~= nil and now - since < 2 then
            table.insert(self.delayed_calls, {
                time = now + 0.05, call = function() self:return_to_place(index, home, since) end,
            })
            return
        end
        self:slide_party(index, home, self.COVER_MOVE_TIME)
    end,

    --- Animation of the defensive stance of the party. TODO: Unverified: check it in the animation viewer.
    DEFEND_ANIMATION = 3,

    --- Opens the window of the Change command (on the left of the commands) or of the Defend command
    -- (on the right).
    --
    -- @param kind "change" or "defend".
    open_side_window = function(self, kind)
        local name = kind == "change" and "Change" or "Defend"
        local window = ui_manager:get_widget("BattleUi.Container.Bottom." .. name)
        ui_manager:get_widget("BattleUi.Container.Bottom." .. name .. ".Text"):set_text(Game.Commands[kind == "change" and 18 or 19].name)
        -- Defend goes right after the commands.
        if kind == "defend" then window:set_x(22, self.commands_width or 120) end
        window:set_visible(true)
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_visible(false)
        audio_manager:play_sound("Cursor")
        self.window_state = kind
    end,

    --- Closes the window of Change or Defend, and goes back to the commands.
    close_side_window = function(self)
        ui_manager:get_widget("BattleUi.Container.Bottom.Change"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.Defend"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cursor"):set_visible(true)
        self.window_state = "command"
    end,

    --- Queues Change or Defend for the character that is choosing.
    --
    -- @param command_id 18 for Change, 19 for Defend.
    queue_side_command = function(self, command_id)
        if self:is_frog_blocked(self.current_character_window, command_id) then
            audio_manager:play_sound("Error")
            return
        end
        self.selected_command = command_id
        self.current_list = "none"
        self.selected_attack = nil
        self.selected_item = nil
        self.selected_summon = nil
        self.selected_limit = nil
        self.selected_throw = nil
        self.selected_coin = nil
        self.selected_links = nil
        self.target_type = Battle.TargetType.SINGLE
        self.target_group = Battle.TargetGroup.PARTY
        self.target_index = self.current_character_window
        self:queue_character_action()
    end,

    --- Gets the direction a party member faces to look at the enemies, as a unit vector in the ground.
    get_row_direction = function(self, x, y)
        local cx, cy, count = 0, 0, 0
        for e = 0, battle_manager:get_enemy_count() - 1 do
            local ex, ey = battle_manager:get_enemy(e):get_position()
            cx, cy, count = cx + ex, cy + ey, count + 1
        end
        if count > 0 then
            local dx, dy = cx / count - x, cy / count - y
            local length = math.sqrt(dx * dx + dy * dy)
            if length > 0.001 then return dx / length, dy / length end
        end
        -- The party is on the positive side of the ground, facing the negative one.
        return 0, y > 0 and -1 or 1
    end,

    --- Moves a party member forward or back, as the rows do.
    --
    -- @param index Index in the party.
    -- @param back True to move away from the enemies (back row), false to go towards them.
    -- @param instant True to move at once, otherwise it slides (see {@see update_row_moves}).
    -- @param on_done Function to call at the end of a slide, or nil.
    move_row = function(self, index, back, instant, on_done)
        local entity = self:get_entity(Battle.TargetGroup.PARTY, index)
        if entity == nil then
            if on_done ~= nil then on_done() end
            return
        end
        local x, y, z = entity:get_position()
        local dx, dy = self:get_row_direction(x, y)
        local sign = back and -1 or 1
        local to = {x = x + dx * self.ROW_STEP * sign, y = y + dy * self.ROW_STEP * sign, z = z}
        -- The new row is the place of the character.
        if self.party[index] ~= nil then self.party[index].home = {x = to.x, y = to.y, z = to.z} end
        if instant then
            entity:set_position(to.x, to.y, to.z)
            if on_done ~= nil then on_done() end
        else
            local now = timer:get_game_time_total()
            table.insert(self.row_moves, {
                index = index, from = {x = x, y = y, z = z}, to = to, start = now, finish = now + self.ROW_MOVE_TIME, on_done = on_done,
            })
        end
    end,

    --- Advances the slides of the row changes.
    update_row_moves = function(self, now)
        for i = #self.row_moves, 1, -1 do
            local move = self.row_moves[i]
            local entity = self:get_entity(Battle.TargetGroup.PARTY, move.index)
            local t = math.min(1, (now - move.start) / (move.finish - move.start))
            if entity ~= nil then
                entity:set_position(
                    move.from.x + (move.to.x - move.from.x) * t, move.from.y + (move.to.y - move.from.y) * t,
                    move.from.z + (move.to.z - move.from.z) * t
                )
            end
            if t >= 1 then
                table.remove(self.row_moves, i)
                if move.on_done ~= nil then move.on_done() end
            end
        end
    end,

    --- Slides that are in progress, see {@see update_row_moves}.
    row_moves = {},

    --- Checks if a combatant is in a defensive stance.
    is_defending = function(self, side, index)
        return side == Battle.TargetGroup.PARTY and self.party[index] ~= nil and self.party[index].defending == true
    end,

    --- Puts a party member in the defensive stance or takes it out of it.
    set_defending = function(self, index, defending)
        self.party[index].defending = defending
        local entity = self:get_entity(Battle.TargetGroup.PARTY, index)
        local name = self:get_animation_name(Battle.TargetGroup.PARTY, index, defending and self.DEFEND_ANIMATION or 0)
        if entity ~= nil and name ~= nil and entity:get_animation_count() > 0 then
            if defending then entity:play_animation_stop(name) else entity:play_animation(name) end
        end
    end,

    --- Goes back to the defensive stance after an animation that interrupted it (a hit).
    restore_stance = function(self, side, index)
        if self:is_defending(side, index) and not self.party[index].dead then self:set_defending(index, true) end
    end,

    --- Runs Change or Defend for the party member that chose it.
    --
    -- Change moves the character to the other row for good (it stays after the battle), and Defend puts it
    -- in a defensive stance until its ATB is full again: the physical damage it takes is halved.
    run_side_command = function(self, current, action)
        local index = action.actor
        local member = self.party[index]
        local char = Characters[member.character]
        self:show_message(Game.Commands[action.command].name)
        current.hit_done = true
        current.pending = 0
        if action.command == 18 then
            char.back_row = not (char.back_row == true)
            if member.defending then self:set_defending(index, false) end
            current.pending = 1
            self:move_row(index, char.back_row, false, function()
                current.pending = current.pending - 1
            end)
        else
            self:set_defending(index, true)
        end
        self:update_action(timer:get_game_time_total())
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
        self.selected_limit = nil
        self.selected_throw = nil
        self.selected_coin = nil
        self.selected_links = nil
        self.target_options = {
            fixed_row = 0, default_enemy = 1, default_multiple = 0, toggle_multiple = 0,
            all_rows = 0, random = 0, selectable = 1
        }
        -- Mega All lets the attack hit all the enemies.
        if self:mega_all_left(self.party[self.current_character_window]) > 0 then self.target_options.toggle_multiple = 1 end
        self.target_type = Battle.TargetType.SINGLE
        self.cursor_target_h = 1
        self.cursor_target_v = 1
        self.first_target = 1
        self:refresh_target_enemies()
        self:populate_target_list()
        ui_manager:get_widget("BattleUi.Container.Bottom.Target"):set_visible(true)
        self.window_state = "target"
    end,

    --- Attack commands that are done: the number of attacks they make (the others make one), and the ones
    -- that aim each attack at a random enemy.
    COMMAND_STRIKES = {[25] = 2, [27] = 4},
    COMMAND_RANDOM = {[27] = true},

    --- Chance of the instant death of Flash on an enemy that is hit (and not immune to it).
    FLASH_CHANCE = 1,

    --- Queues a command that has no target window: Slash-All and Flash hit all the enemies, and 4x-Cut
    -- picks a random enemy for each hit.
    --
    -- @param command_id ID of the command in Game.Commands.
    -- @return True if the command is one of them and was queued.
    --- Gets the attacks of an enemy that a party member can use while it manipulates it.
    --
    -- Builds the commands of the window (virtual commands that have the names of the attacks), or removes them
    -- if the member is not manipulating an enemy.
    update_manip_commands = function(self, index)
        local member = self.party[index]
        member.manip_commands = nil
        self.manip_attack = nil
        local enemy = member.manipulating
        if enemy == nil then return end
        local source = battle_manager:get_enemy(enemy - 1)
        member.manip_commands = {}
        member.manip_attacks = {}
        for slot = 0, math.min(11, source:get_manip_count() - 1) do
            local id = source:get_manip_attack(slot)
            local data = self:get_attack_data(id)
            Game.Commands[1000 + slot] = {name = data ~= nil and data.name or "?", menu = Battle.Menu.NONE, camera = {single = 65535, multiple = 65535}}
            table.insert(member.manip_commands, 1000 + slot)
            table.insert(member.manip_attacks, id)
        end
    end,

    --- Picks an attack of the manipulated enemy and asks for its targets.
    select_manip_attack = function(self, cursor)
        local member = self.party[self.current_character_window]
        local id = member.manip_attacks[cursor]
        if id == nil then return end
        audio_manager:play_sound("Cursor")
        local data = self:get_attack_data(id)
        local target = data.target or {}
        self.manip_attack = id
        self.selected_command = nil
        self.current_list = "none"
        self.selected_attack = nil
        self.selected_item = nil
        self.selected_summon = nil
        self.selected_limit = nil
        self.selected_throw = nil
        self.selected_coin = nil
        self.selected_links = nil
        -- The attacks of an enemy that aim at "enemies" are aimed at the party, so the sides are the other way round.
        self.target_options = {
            selectable = 1, default_enemy = target.default_enemy == 1 and 0 or 1, default_multiple = target.default_multiple or 0,
            toggle_multiple = target.toggle_multiple or 0, fixed_row = target.fixed_row or 0, all_rows = target.all_rows or 0,
            random = target.random or 0, short_range = target.short_range or 0,
        }
        self.target_type = self.target_options.default_multiple == 1 and Battle.TargetType.GROUP or Battle.TargetType.SINGLE
        self.cursor_target_h = self.target_options.default_enemy == 1 and 1 or 2
        self.cursor_target_v = 1
        self.first_target = 1
        self:refresh_target_enemies()
        self:populate_target_list()
        ui_manager:get_widget("BattleUi.Container.Bottom.Target"):set_visible(true)
        self.window_state = "target"
    end,

    --- Queues the attack of the manipulated enemy chosen by its controller. It takes the turn of the controller.
    queue_manip_action = function(self)
        local controller = self.current_character_window
        local member = self.party[controller]
        local enemy = member.manipulating
        local id = self.manip_attack
        local script = nil
        local source = battle_manager:get_enemy(enemy - 1)
        for slot = 0, source:get_attack_count() - 1 do
            if source:get_attack(slot) == id then
                script = source:get_attack_animation(slot)
                break
            end
        end
        if script ~= nil and script < 0 then script = nil end
        table.insert(self.action_queue, {
            actor = enemy, actor_side = Battle.TargetGroup.ENEMIES, type = Battle.ActionType.COMMAND, command = 1,
            attack_id = id, script = script, controller = controller,
            target_type = self.target_type, target_group = self.target_group, target_index = self.target_index,
        })
        member.acting = true
        member.manip_commands = nil
        self.manip_attack = nil
        for i = #self.command_queue, 1, -1 do
            if self.command_queue[i] == controller then table.remove(self.command_queue, i) end
        end
        ui_manager:get_widget("BattleUi.Container.Bottom.Target"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.List"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.Change"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.Defend"):set_visible(false)
        self.current_list = "none"
        self.window_state = "wait"
    end,

    --- Builds the action that a Mime command does: a copy of the last action of the party, done by the member that mimes.
    --
    -- Limits can only be mimed by their owner. The copy uses no MP, summon or All counters; items, thrown things
    -- and gil are paid again.
    --
    -- @return The action, or nil and the message of why it can't be done.
    build_mime_action = function(self, action)
        local last = self.last_party_action
        if last == nil then return nil, "Nothing to mime" end
        if last.limit ~= nil and last.actor ~= action.actor then return nil, "Can't mime that" end
        local copy = {}
        for key, value in pairs(last) do copy[key] = value end
        copy.actor = action.actor
        copy.actor_side = Battle.TargetGroup.PARTY
        copy.mimed = true
        copy.no_atb = nil
        return copy
    end,

    --- Makes an enemy a puppet of a party member (Manipulate). It fails with a message if the enemy has no attacks
    -- to be manipulated with, is immune, or the member is already controlling one.
    manipulate = function(self, actor, target)
        local member = self.party[actor]
        local enemy = self.enemies[target]
        local source = battle_manager:get_enemy(target - 1)
        if member.manipulating ~= nil or enemy == nil or enemy.manipulated_by ~= nil or source:get_manip_count() == 0 then
            self:show_message("Can't be manipulated")
            return
        end
        self:apply_attack_effects({effects = {{id = BattleStatus.MANIPULATE, chance = 1, mode = "inflict"}}}, Battle.TargetGroup.ENEMIES, target)
        if self:has_status(Battle.TargetGroup.ENEMIES, target, BattleStatus.MANIPULATE) then
            member.manipulating = target
            enemy.manipulated_by = actor
            -- The puppet turns around while it is controlled.
            local entity = self:get_entity(Battle.TargetGroup.ENEMIES, target)
            if entity ~= nil then
                enemy.manip_rotation = entity:get_rotation()
                entity:set_rotation((enemy.manip_rotation + 180) % 360)
            end
        else
            self:show_message("Can't be manipulated")
        end
    end,

    --- Frees an enemy that was manipulated: its controller dies, it dies, or it takes physical damage.
    release_manipulation = function(self, target)
        local enemy = self.enemies[target]
        if enemy == nil or enemy.manipulated_by == nil then return end
        local controller = enemy.manipulated_by
        enemy.manipulated_by = nil
        local entity = self:get_entity(Battle.TargetGroup.ENEMIES, target)
        if entity ~= nil and enemy.manip_rotation ~= nil then entity:set_rotation(enemy.manip_rotation) end
        enemy.manip_rotation = nil
        self.party[controller].manipulating = nil
        self:clear_status(Battle.TargetGroup.ENEMIES, target, BattleStatus.MANIPULATE)
        -- If its controller was choosing, it chooses its own commands again.
        if self.party[controller].manip_commands ~= nil and not self.party[controller].acting then
            self.manip_attack = nil
            ui_manager:get_widget("BattleUi.Container.Bottom.Target"):set_visible(false)
            if self.current_character_window == controller and (self.window_state == "command" or self.window_state == "target") then
                self:show_commands(controller)
            else
                self.party[controller].manip_commands = nil
            end
        end
    end,

    --- Gets the commands a member shows: its own, or the attacks of the enemy it manipulates.
    get_command_list = function(self, index)
        local member = self.party[index]
        return member.manip_commands or member.command
    end,

    queue_direct_command = function(self, command_id)
        -- Mime copies the last action when its turn comes, so it has no choices.
        if command_id == self.MIME_COMMAND then
            audio_manager:play_sound("Cursor")
            self.selected_command = command_id
            self.current_list = "none"
            self.selected_attack = nil
            self.selected_item = nil
            self.selected_summon = nil
            self.selected_limit = nil
            self.selected_throw = nil
            self.selected_coin = nil
            self.selected_links = nil
            self.target_group = Battle.TargetGroup.ENEMIES
            self.target_type = Battle.TargetType.GROUP
            self.target_index = nil
            self:queue_character_action()
            return true
        end
        if command_id ~= 24 and command_id ~= 26 and command_id ~= 27 then return false end
        local target = nil
        local all = command_id == 24 or command_id == 26
        if not all then
            target = self:random_living(Battle.TargetGroup.ENEMIES)
            if target == nil then return true end
        end
        audio_manager:play_sound("Cursor")
        self.selected_command = command_id
        self.current_list = "none"
        self.selected_attack = nil
        self.selected_item = nil
        self.selected_summon = nil
        self.selected_limit = nil
        self.selected_throw = nil
        self.selected_coin = nil
        self.selected_links = nil
        self.target_group = Battle.TargetGroup.ENEMIES
        self.target_type = all and Battle.TargetType.GROUP or Battle.TargetType.SINGLE
        self.target_index = target
        self:queue_character_action()
        return true
    end,

    --- Double commands, with the menu of each one.
    W_COMMANDS = {[21] = Battle.Menu.W_MAGIC, [22] = Battle.Menu.W_SUMMON, [23] = Battle.Menu.W_ITEM},

    --- Gets the name of what an action uses (the spell, summon or item).
    get_w_name = function(self, action)
        if action.item_id ~= nil then return Game.Items[action.item_id].name end
        if action.attack_id ~= nil then return Game.Attacks[action.attack_id].name end
        return ""
    end,

    --- Gets the MP that the first choice of a double command will use.
    get_w_reserved_mp = function(self)
        local first = self.w_first
        if first == nil or first.attack_id == nil then return 0 end
        return self:get_spell_cost(Game.Attacks[first.attack_id].mp, first.links)
    end,

    --- Adds a party character action to the queue
    --
    -- Closes the selection windows and removes the character from the command queue.
    queue_character_action = function(self)
        if self.manip_attack ~= nil then
            self:queue_manip_action()
            return
        end
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
            action.limit = self.selected_limit
            action.links = self.selected_links
            action.throw_item = self.selected_throw
        elseif self.selected_command == self.party[action.actor].command[1] then
            -- The Attack command carries the links of its materia. A frog that lost a replacement of the command
            -- loses its links too.
            local member = self.party[action.actor]
            if member.command[1] == member.attack_command or member.command[1] == 20 then action.links = member.attack_links end
        end
        if self.selected_command == self.COIN_COMMAND then action.coin_gil = self.selected_coin end
        action.target_type = self.target_type
        action.target_group = self.target_group
        action.target_index = self.target_index
        -- W-Magic, W-Summon and W-Item: the first choice waits, and the list opens again for the second one. Both
        -- are queued together, the first without resetting the ATB (that happens after the second).
        if self.W_COMMANDS[self.selected_command] ~= nil then
            if self.w_first == nil then
                action.no_atb = true
                self.w_first = action
                ui_manager:get_widget("BattleUi.Container.Bottom.Target"):set_visible(false)
                ui_manager:get_widget("BattleUi.Container.Bottom.List"):set_visible(false)
                self.current_list = "none"
                self.window_state = "command"
                self:show_message(self:get_w_name(action), false)
                self.message_until = nil
                self:show_list(self.W_COMMANDS[self.selected_command])
                return
            end
            table.insert(self.action_queue, self.w_first)
            self.w_first = nil
            self:show_message("")
        end
        table.insert(self.action_queue, action)
        -- The member has chosen: it doesn't choose again until the action is executed.
        self.party[action.actor].acting = true

        -- The character has chosen: remove it from the command queue and close the windows.
        for i = #self.command_queue, 1, -1 do
            if self.command_queue[i] == action.actor then table.remove(self.command_queue, i) end
        end
        ui_manager:get_widget("BattleUi.Container.Bottom.Target"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.List"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.Change"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.Defend"):set_visible(false)
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
            local ok, err = pcall(BattleCameraController.update, BattleCameraController, now - self.last_tick_time)
            if not ok then print("Battle camera: " .. tostring(err)) BattleCameraController:reset() end
            self.last_tick_time = now
            do return end
        end
        if self.intro == true then
            -- The battle waits for the camera of its start. The first tick can be late (loading), so
            -- the camera doesn't skip ahead.
            self:update_animations(now)
            UiContainer.BattleFx:update(now)
            local elapsed = math.min(now - self.last_tick_time, 0.2)
            self.last_tick_time = now
            local ok, going = pcall(BattleCameraController.update_intro, BattleCameraController, elapsed)
            if not ok then
                print("Battle camera: " .. tostring(going))
                BattleCameraController:reset()
                battle_manager:set_camera(0)
            end
            self.intro = ok and going
            -- The time that the ATBs are held for starts when the camera ends.
            if self.intro ~= true then self.hold_start = now end
            do return end
        end
        local delta = now - self.last_tick_time
        self.last_tick_time = now
        if self:update_escape(now, delta) then return end
        self:update_animations(now)
        UiContainer.BattleFx:update(now)
        self:update_hides(now)
        self:update_delayed_calls(now)
        self:update_row_moves(now)
        self:update_limit_flash(now)
        if self.message_until ~= nil and now >= self.message_until then self:show_message("") end
        self:update_action(now)
        -- The camera of the attacks.
        if self.action_executing ~= true and not BattleCameraController:is_script_running() then
            BattleCameraController:stop()
        end
        local ok, err = pcall(BattleCameraController.update, BattleCameraController, delta)
        if not ok then print("Battle camera: " .. tostring(err)) BattleCameraController:stop(false) end
        -- Held ATBs start to fill after some time.
        if (self.party_hold or self.enemy_hold) and now - self.hold_start > self.HOLD_TIME then
            self:release_hold()
        end
        self:update_statuses(now)
        -- A manipulated enemy is freed when it dies or its controller does.
        for i = 1, #self.enemies do
            local enemy = self.enemies[i]
            if enemy.manipulated_by ~= nil and (enemy.dead or self.party[enemy.manipulated_by].dead) then self:release_manipulation(i) end
        end
        -- Advance party ATBs
        for i = 1, #self.party do
            local member = self.party[i]
            if member.dead == false and self.party_hold == false then
                -- 0 if a status stops the ATB.
                local rate = self:get_atb_rate(Battle.TargetGroup.PARTY, i)
                if member.atb < 255 and rate > 0 then
                    member.atb = member.atb + self:get_atb_speed(Battle.TargetGroup.PARTY, i) * delta
                    -- Fill ATB UI bar
                    -- 0   -> 0%
                    -- 255 -> 18%-6
                    local width_percent = math.min(18, 18 * member.atb / 255)
                    local width_fixed = math.max(-6, -6 * width_percent / 18)
                    ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(i) .. ".Right.AtbFill"):set_width(width_percent, width_fixed)
                end
                -- The defensive stance lasts until the ATB is full again.
                if member.defending and member.atb >= 255 then self:set_defending(i, false) end
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
                        self.enemies[i].atb = self.enemies[i].atb + self:get_atb_speed(Battle.TargetGroup.ENEMIES, i) * delta
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
        -- Pre-Emptive materia can make a normal battle a pre-emptive one. TODO: The chance is a guess
        -- (its percent points).
        if layout == 0 then
            local points = 0
            for i = 1, #self.party do points = points + (self.party[i].independent ~= nil and self.party[i].independent.preemptive or 0) end
            if points > 0 and math.random() * 100 < points then
                print("Pre-Emptive materia: pre-emptive attack")
                layout = 1
            end
        end
        self.party_hold = false
        self.enemy_hold = false
        self.hold_start = timer:get_game_time_total()
        self.last_tick_time = self.hold_start
        self.escape_seen = {}
        self.escape_progress = 0
        self.escape_pose = {}
        -- Average dex of the enemies (rounded up) plus 50, the divisor of the ATB speeds.
        local sum = 0
        for i = 1, #self.enemies do sum = sum + (self.enemies[i].stats.dex or 0) end
        local count = math.max(1, #self.enemies)
        self.enemy_dex_average = math.floor((sum + count - 1) / count) + 50
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
            id = id, name = k.name, mp = k.mp or 0, reflectable = false,
            -- Limits with the additional effect 0 hit as many times as the modifier says (Omnislash 15, from the
            -- attack data of ff7.exe; not verified for the other attacks).
            hits = (k.additional ~= nil and k.additional.effect == 0 and k.additional.modifier > 1 and id >= 128) and k.additional.modifier or nil,
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

    --- Attack of the kernel with the data of Throw.
    THROW_ATTACK = 122,

    --- Gets the attack data of an action: an item, a spell or summon, or the basic attack.
    get_action_attack = function(self, action)
        if action.item_id ~= nil then return self:item_to_attack(action.item_id) or self.basic_attack end
        -- Coin: a tenth of the gil thrown as fixed damage, divided among the enemies that are alive (rounded up).
        if action.coin_gil ~= nil then
            if action.coin_damage == nil then
                local alive = 0
                for _, enemy in ipairs(self.enemies) do if not enemy.dead then alive = alive + 1 end end
                action.coin_damage = math.ceil(action.coin_gil / 10 / math.max(1, alive))
            end
            return {
                id = -3, name = "Coin", mp = 0, reflectable = false,
                damage = {formula = 1, modifier = 1, power = 0, accuracy = 1, fixed_damage = action.coin_damage},
                -- It always affects all the enemies, so it doesn't need All.
                target = {short_range = 0, default_multiple = 1}, effects = {}, elements = {}, animation = {},
            }
        end
        -- Throw: a physical attack of power 16 (attack 122 of the kernel) that never misses, with the attack of the
        -- thrown weapon added to the one of the thrower (0x5c8fbd in ff7.exe).
        if action.throw_item ~= nil then
            local item = Game.Items[action.throw_item]
            local data = self:get_attack_data(self.THROW_ATTACK)
            if item ~= nil and data ~= nil then
                local copy, damage = {}, {}
                for key, value in pairs(data) do copy[key] = value end
                for key, value in pairs(data.damage) do damage[key] = value end
                damage.attack_bonus = item.type == Inventory.ITEM_TYPE.WEAPON and (item.power or 0) or 0
                copy.damage = damage
                copy.name = item.name
                copy.elements = {}
                copy.target = {short_range = 0}
                return copy
            end
        end
        -- The data of the limit techniques (ids 128 to 183) comes from a table of ff7.exe that the installer
        -- adds to the kernel attacks. The physical ones (formulas 1 and 11) are used as they are. The other
        -- ones are not done in SPECIAL_LIMITS yet (statuses, MP damage, slots, transformations...) and are
        -- a strong normal attack. TODO: Those ones.
        if action.limit ~= nil and action.attack_id ~= nil and action.actor_side == Battle.TargetGroup.PARTY then
            local kernel = Game.Attacks[action.attack_id]
            -- Mindblow (formula 2) goes through the data too, and does its damage to MP.
            if kernel ~= nil and (kernel.dmg_formula == 1 or kernel.dmg_formula == 11
              or (kernel.dmg_formula == 2 and self.LIMIT_DATA_ATTACKS[action.attack_id])) then
                local data = self:get_attack_data(action.attack_id)
                if data ~= nil then
                    if self.LIMIT_MP_DAMAGE[action.attack_id] then data.damage.mp_damage = true end
                    return data
                end
            end
            if kernel ~= nil then
                local limit = self:get_weapon_attack(action.actor)
                local damage = {}
                for key, value in pairs(limit.damage) do damage[key] = value end
                damage.power = self.LIMIT_FALLBACK_POWER
                limit.damage = damage
                limit.name = kernel.name
                return limit
            end
        end
        if (action.attack_id == nil or action.attack_id < 0) and action.actor_side == Battle.TargetGroup.PARTY then
            if action.command == 26 then
                -- Flash: it can miss like a normal attack, with half of the accuracy of the weapon (ff7.exe,
                -- 0x5ca5c1), does no damage, and kills the enemies it hits unless they are immune to death.
                local flash = self:get_weapon_attack(action.actor)
                local damage = {}
                for key, value in pairs(flash.damage) do damage[key] = value end
                damage.power = 0
                damage.no_damage = true
                damage.critical = 0
                damage.accuracy = math.floor(math.floor((damage.accuracy or 1) * 255 + 0.5) / 2) / 255
                flash.damage = damage
                flash.name = "Flash"
                flash.elements = {}
                flash.effects = {{id = BattleStatus.DEATH, chance = self.FLASH_CHANCE, mode = "inflict"}}
                return flash
            end
            -- D.blow: an attack that is always a critical hit, or misses.
            if action.command == self.DBLOW_COMMAND then
                local attack = self:get_weapon_attack(action.actor)
                local damage = {}
                for key, value in pairs(attack.damage) do damage[key] = value end
                damage.always_critical = 1
                -- A third of the accuracy of the weapon (ff7.exe, 0x5ca610).
                damage.accuracy = math.floor(math.floor((damage.accuracy or 1) * 255 + 0.5) / 3) / 255
                attack.damage = damage
                attack.name = "D.blow"
                return attack
            end
            return self:get_weapon_attack(action.actor)
        end
        return self:get_attack_data(action.attack_id) or self.basic_attack
    end,

    --- Builds the basic attack of a party member from the weapon equipped: its accuracy, critical
    -- rate, elements and range.
    --
    -- @param index Index in the party (1 based).
    get_weapon_attack = function(self, index)
        local member = self.party[index]
        local char = member ~= nil and Characters[member.character] or nil
        local weapon = (char ~= nil and char.weapon ~= nil) and Game.Items[char.weapon.id] or nil
        if weapon == nil then return self.basic_attack end
        local elements = {}
        for element, _ in pairs(member.effects.elements.attack) do table.insert(elements, element) end
        table.sort(elements)
        -- The added statuses have the chance 5 of 63 (ff7.exe, 0x5c9c5f).
        local effects = {}
        for status, _ in pairs(member.effects.status.attack) do
            table.insert(effects, {id = status, chance = self.ADDED_EFFECT_CHANCE, mode = "inflict"})
        end
        return {
            id = -1, name = "Attack", mp = 0, weapon = true,
            -- The accuracy of weapons is a percentage.
            damage = {formula = 1, modifier = 1, power = 16, accuracy = (weapon.accuracy or 100) / 255, critical = weapon.critical or 0},
            target = {short_range = weapon.target ~= nil and weapon.target.short_range or 0},
            -- Elemental materia linked in the weapon adds elements, and Added Effect adds statuses.
            effects = effects, elements = elements, animation = {},
        }
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

    --- Picks the random target of a confused combatant in a side: a living one (a dead one for attacks that revive),
    -- and not itself while there is another one.
    --
    -- @return The index, nil if there is nobody to target.
    get_confused_target = function(self, group, side, index, revives)
        local list = group == Battle.TargetGroup.PARTY and self.party or self.enemies
        local candidates = {}
        for i = 1, #list do
            if list[i].dead == revives and not (group == side and i == index) then table.insert(candidates, i) end
        end
        if #candidates == 0 then
            if group == side and list[index] ~= nil and list[index].dead == revives then return index end
            return nil
        end
        return candidates[math.random(1, #candidates)]
    end,

    --- Aims an action of a confused combatant: at the side opposite to the usual one of what it uses (an ally for what
    -- is meant for enemies, and the other way round), at one random combatant or at the whole side.
    --
    -- @param multiple True to aim at the whole side.
    -- @return False if there is nobody to target.
    aim_confused = function(self, action, group, multiple, revives)
        action.target_group = group
        action.target_list = nil
        action.target_index = nil
        if multiple then
            action.target_type = Battle.TargetType.GROUP
            return true
        end
        action.target_type = Battle.TargetType.SINGLE
        action.target_index = self:get_confused_target(group, action.actor_side or Battle.TargetGroup.PARTY, action.actor, revives)
        return action.target_index ~= nil
    end,

    --- Turns around the targets of an action that was chosen before its actor got confused.
    --
    -- @param attack Attack data of the action.
    -- @return False if there is nobody to target.
    apply_confusion = function(self, action, attack)
        local side = action.actor_side or Battle.TargetGroup.PARTY
        if action.automatic or action.target_group == nil or not self:has_status(side, action.actor, BattleStatus.CONFUSION) then
            return true
        end
        local group = action.target_group == Battle.TargetGroup.PARTY and Battle.TargetGroup.ENEMIES or Battle.TargetGroup.PARTY
        local multiple = action.target_index == nil and (action.target_list == nil or #action.target_list > 1)
        return self:aim_confused(action, group, multiple, self:attack_revives(attack))
    end,

    --- Gets the spells that a confused combatant can cast: the ones it can pay and isn't stopped from casting. For an
    -- enemy, its attacks that cost MP.
    --
    -- @return A list of actions (attack_id, command and script).
    get_confused_spells = function(self, side, index)
        local spells = {}
        if self:has_status(side, index, BattleStatus.SILENCE) then return spells end
        local frog = self:has_status(side, index, BattleStatus.FROG)
        if side == Battle.TargetGroup.PARTY then
            local member = self.party[index]
            local mp = Characters[member.character].stats.mp.current
            for _, spell in ipairs(member.magic or {}) do
                if self:get_spell_cost(Game.Attacks[spell.id].mp, nil) <= mp and (not frog or spell.id == self.FROG_SPELL) then
                    table.insert(spells, {attack_id = spell.id, command = 2})
                end
            end
        elseif not frog then
            local source = battle_manager:get_enemy(index - 1)
            for slot = 0, source:get_attack_count() - 1 do
                local id = source:get_attack(slot)
                local data = self:get_attack_data(id)
                if data ~= nil and (data.mp or 0) > 0 and data.mp <= self.enemies[index].stats.mp then
                    local script = source:get_attack_animation(slot)
                    table.insert(spells, {attack_id = id, command = 1, script = script >= 0 and script or nil})
                end
            end
        end
        return spells
    end,

    --- Queues the action of a combatant that doesn't choose what to do. A berserk one attacks a random enemy. A
    -- confused one attacks or casts a random spell (half of the times each, if it can cast any), at the side opposite
    -- to the usual one. Berserk goes first.
    queue_auto_action = function(self, side, index)
        local other = side == Battle.TargetGroup.PARTY and Battle.TargetGroup.ENEMIES or Battle.TargetGroup.PARTY
        local berserk = self:has_status(side, index, BattleStatus.BERSERK)
        local action = {
            actor = index, actor_side = side, type = Battle.ActionType.COMMAND, command = 1, automatic = true,
        }
        -- The attack of an enemy is the first of the ones it has when manipulated (ff7.exe, 0x435a32 for Berserk).
        if side == Battle.TargetGroup.ENEMIES then
            local source = battle_manager:get_enemy(index - 1)
            if source:get_manip_count() > 0 then
                action.attack_id = source:get_manip_attack(0)
                for slot = 0, source:get_attack_count() - 1 do
                    if source:get_attack(slot) == action.attack_id and source:get_attack_animation(slot) >= 0 then
                        action.script = source:get_attack_animation(slot)
                        break
                    end
                end
            end
        end
        local aimed = false
        if not berserk then
            local spells = self:get_confused_spells(side, index)
            if #spells > 0 and math.random() < 0.5 then
                local spell = spells[math.random(1, #spells)]
                local cast = {
                    actor = index, actor_side = side, automatic = true, command = spell.command,
                    attack_id = spell.attack_id, script = spell.script,
                    type = side == Battle.TargetGroup.PARTY and Battle.ActionType.ATTACK or Battle.ActionType.COMMAND,
                }
                -- A spell without anybody to target (like Life with everybody alive) is left for an attack.
                if self:aim_confused_default(cast) then action, aimed = cast, true end
            end
            if not aimed then aimed = self:aim_confused_default(action) end
        else
            action.target_type = Battle.TargetType.SINGLE
            action.target_group = other
            action.target_index = self:random_living(other)
            aimed = action.target_index ~= nil
        end
        if not aimed then return end
        table.insert(self.action_queue, action)
        local combatant = self:get_combatant(side, index)
        if side == Battle.TargetGroup.PARTY then combatant.acting = true
        else
            combatant.atb = 0
            combatant.waiting = true
        end
    end,

    --- Aims an action that a confused combatant chooses itself, from the usual targets of what it uses.
    aim_confused_default = function(self, action)
        local side = action.actor_side
        local attack = self:get_action_attack(action)
        local target = attack.target or {}
        local other = side == Battle.TargetGroup.PARTY and Battle.TargetGroup.ENEMIES or Battle.TargetGroup.PARTY
        local group = target.default_enemy ~= 0 and side or other
        -- Spells that All materia can widen have both flags, and start with one target.
        local multiple = target.default_multiple == 1 and target.toggle_multiple ~= 1
        return self:aim_confused(action, group, multiple, self:attack_revives(attack))
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
            ui:show_message(text)
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
            allow_dead = self.ai_counter == true, is_counter = self.ai_counter == true,
        }
        if self.ai_counter then
            -- Counterattacks go before the other actions, but keep the order they were requested in.
            local position = self.ai_counter_count + 1
            table.insert(self.action_queue, math.min(position, #self.action_queue + 1), action)
            self.ai_counter_count = position
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
        self.ai_counter_count = 0
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
        })
        -- Stop the enemy ATB from refilling until its action is done.
        self.enemies[index].atb = 0
        self.enemies[index].waiting = true
    end,

    --- Statuses that make a target be hit by the physical attacks (0x2404445 in ff7.exe), and by the magical ones
    -- (0x2004445, without Manipulate).
    PHYSICAL_AUTO_HIT = {BattleStatus.DEATH, BattleStatus.SLEEP, BattleStatus.CONFUSION, BattleStatus.STOP,
        BattleStatus.PETRIFY, BattleStatus.MANIPULATE, BattleStatus.PARALYSIS},
    MAGICAL_AUTO_HIT = {BattleStatus.DEATH, BattleStatus.SLEEP, BattleStatus.CONFUSION, BattleStatus.STOP,
        BattleStatus.PETRIFY, BattleStatus.PARALYSIS},

    --- Rolls if an attack hits, with the formulas of ff7.exe (0x5ddb20 physical, 0x5dddce magical).
    --
    -- Physical: the chance is the accuracy of the attack + dexterity / 4 of the attacker + the evade of the attacker
    -- - the evade of the target (see get_evade), minus 30% if the attacker has Fury, and it is rolled from 1 to
    -- 100 (it hits if the roll is lower). Darkness halves the accuracy of the attack. Luck lets attackers always
    -- hit (luck / 4 percent) and, for the attacks of enemies, lets party members always evade. Sleep, Stop,
    -- Confusion, Petrify, Paralysis, Manipulate and Death are always hit (before the luck rolls).
    -- Magical: the chance is the accuracy (minus 30% with Fury) + level of the attacker - level of the target / 2,
    -- after a roll against the magic evasion of the target (a percent). Reflect, and the statuses of
    -- the physical ones except Manipulate, make the target be hit. 255 or more of accuracy always hits.
    -- Elements: a target that nullifies or absorbs the attack, or dies by its element, is hit like the ones with
    -- those statuses. For attacks without power (the ones that only add statuses), a weakness doubles the
    -- accuracy and a resistance halves it.
    --
    -- @param formula Formula of the attack.
    -- @param accuracy Accuracy byte of the attack.
    -- @param element Element factor of the target against the attack (see get_element_factor), 1 if nil.
    -- @param power Power of the attack.
    -- @return True if the attack hits.
    roll_hit = function(self, formula, accuracy, attacker_side, attacker_index, target_side, target_index, element, power)
        local physical = formula == 1 or formula == 6 or formula == 10
        local magical = formula == 2 or formula == 7
        if not physical and not magical then return true end
        local a = self:get_combat_stats(attacker_side, attacker_index)
        local t = self:get_combat_stats(target_side, target_index)
        local fury = self:has_status(attacker_side, attacker_index, BattleStatus.FURY)
        local function any(list)
            for _, id in ipairs(list) do
                if self:has_status(target_side, target_index, id) then return true end
            end
            return false
        end
        element = element or 1
        local element_hit = element >= 10 or element == 0 or (element < 0 and element > -10)
        if physical and self:has_status(attacker_side, attacker_index, BattleStatus.DARKNESS) then
            accuracy = math.floor(accuracy / 2)
        end
        if power == 0 then
            if element == 2 then accuracy = accuracy * 2
            elseif element == 0.5 then accuracy = math.floor(accuracy / 2)
            end
        end
        if magical then
            if accuracy >= 255 or element_hit then return true end
            if self:has_status(target_side, target_index, BattleStatus.REFLECT) or any(self.MAGICAL_AUTO_HIT) then return true end
            if fury then accuracy = accuracy - math.floor(accuracy * 3 / 10) end
            if math.floor(math.random() * 100) < t.meva then return false end
            return math.random(1, 100) < accuracy + a.level - math.floor(t.level / 2)
        end
        local chance
        if element_hit or any(self.PHYSICAL_AUTO_HIT) then chance = 255
        else
            chance = accuracy + math.floor(a.dex / 4) + self:get_evade(attacker_side, attacker_index)
              - self:get_evade(target_side, target_index)
            if chance < 255 and fury then chance = chance - math.floor(chance * 3 / 10) end
            if chance <= 0 then chance = 1 end
        end
        local roll = math.floor(math.random() * 100)
        if roll < math.floor(self:get_luck(attacker_side, attacker_index) / 4) then chance = 255
        elseif attacker_side == Battle.TargetGroup.ENEMIES and target_side == Battle.TargetGroup.PARTY
          and roll < math.floor(self:get_luck(target_side, target_index) / 4) then chance = 0
        end
        return math.random(1, 100) < chance
    end,

    --- Gets the physical evade of a combatant: dexterity / 4 plus the evasion of the armor for a party member,
    -- its evade for an enemy. The evade modifier applies to the whole sum, the dexterity one doesn't count.
    get_evade = function(self, side, index)
        local stats = self:get_base_combat_stats(side, index)
        local evade = stats.eva
        if side == Battle.TargetGroup.PARTY then evade = evade + math.floor(stats.dex / 4) end
        local mods = self:get_combatant(side, index).stat_mods
        if mods ~= nil and mods.eva ~= nil then evade = evade + math.floor(evade * mods.eva / 100) end
        return evade
    end,

    --- Gets the stats a damage formula needs from a combatant.
    --
    -- @param side {@see Battle.TargetGroup.PARTY} or {@see Battle.TargetGroup.ENEMIES}.
    -- @param index Index in the party or enemy list.
    -- @return A table with level, str (attack), def, mag, mdef, hp, hp_max, mp.
    get_combat_stats = function(self, side, index)
        local stats = self:get_base_combat_stats(side, index)
        local mods = self:get_combatant(side, index).stat_mods
        if mods ~= nil then
            for kind, percent in pairs(mods) do
                if stats[kind] ~= nil then stats[kind] = stats[kind] + math.floor(stats[kind] * percent / 100) end
            end
        end
        return stats
    end,

    --- Changes a stat modifier of a combatant, a percentage (-100 to 100) of its stat as in ff7.exe (0x5c7e55).
    -- The modifiers last until the end of the battle.
    --
    -- @param kind Stat: str, mag, def, mdef, eva or dex.
    -- @param delta Percentage added to the modifier.
    change_stat_mod = function(self, side, index, kind, delta)
        local combatant = self:get_combatant(side, index)
        combatant.stat_mods = combatant.stat_mods or {}
        combatant.stat_mods[kind] = math.max(-100, math.min(100, (combatant.stat_mods[kind] or 0) + delta))
    end,

    --- Gets the stats of a combatant without the modifiers of the limits. See {@see get_combat_stats}.
    get_base_combat_stats = function(self, side, index)
        if side == Battle.TargetGroup.PARTY then
            local char = Characters[self.party[index].character]
            local stats = char.stats
            local str = stats.str.total or stats.str.base or 1
            if char.weapon ~= nil and char.weapon.attack ~= nil then str = str + char.weapon.attack end
            -- Defense, magic defense and the evasions include the armor (Characters.recalculate_stats).
            return {
                level = char.level or 1, str = str,
                def = stats.def or stats.vit.total or stats.vit.base or 0,
                mag = stats.mag.total or stats.mag.base or 1,
                mdef = stats.mdef or stats.spr.total or stats.spr.base or 0,
                dex = stats.dex.total or stats.dex.base or 0, eva = stats.eva or 0, meva = stats.meva or 0,
                hp = stats.hp.current, hp_max = self:get_stat_max(stats.hp),
                mp = stats.mp.current, mp_max = self:get_stat_max(stats.mp),
            }
        end
        local enemy = self.enemies[index]
        return {
            level = enemy.level, str = enemy.stats.str, def = enemy.stats.def, mag = enemy.stats.mag,
            mdef = enemy.stats.spr, hp = enemy.stats.hp, hp_max = enemy.stats.hp_max, mp = enemy.stats.mp,
            mp_max = enemy.stats.mp_max, dex = enemy.stats.dex or 0, eva = enemy.stats.eva or 0, meva = enemy.stats.meva or 0,
        }
    end,

    --- Gets the luck of a combatant.
    get_luck = function(self, side, index)
        if side == Battle.TargetGroup.PARTY then
            local stats = Characters[self.party[index].character].stats
            return stats.lck.total or stats.lck.base or 0
        end
        return self.enemies[index].stats.lck or 0
    end,

    --- Checks if a combatant has Long Range materia: its attacks don't lose power from the rows.
    has_long_range = function(self, side, index)
        return side == Battle.TargetGroup.PARTY and self.party[index].independent ~= nil and self.party[index].independent.long_range
    end,

    --- Takes the use of All or Mega All of an action that was widened to all the targets.
    --
    -- @return False if it can't be paid anymore.
    spend_all = function(self, action, attack)
        if action.target_type ~= Battle.TargetType.GROUP or action.is_counter or action.sneak_attack then return true end
        -- Actions that already hit all the targets.
        if attack.target.default_multiple == 1 or (action.attack_id == nil and (action.command == 24 or action.command == 26)) then return true end
        local member = self.party[action.actor]
        if action.links ~= nil and (action.links.all or 0) > 0 and attack.target.toggle_multiple == 1 then
            local used = member.all_used[action.attack_id] or 0
            if used < action.links.all then
                member.all_used[action.attack_id] = used + 1
                return true
            end
        end
        if self:mega_all_left(member) > 0 then
            member.mega_all_used = member.mega_all_used + 1
            return true
        end
        return false
    end,

    --- Gets the times a character can still use Mega All (All on every action) in this battle.
    mega_all_left = function(self, member)
        if member == nil or member.independent == nil then return 0 end
        return member.independent.mega_all - (member.mega_all_used or 0)
    end,

    --- Looks for a party member that covers another one that was attacked: an ally with Cover materia
    -- takes the attack, by chance, for a member with critical HP (a quarter of the maximum or less).
    --
    -- @return The index of the member that covers it, nil if nobody does.
    find_cover = function(self, target)
        local stats = self:get_combat_stats(Battle.TargetGroup.PARTY, target)
        if self.party[target].dead or stats.hp > stats.hp_max / 4 then return nil end
        for index = 1, #self.party do
            local member = self.party[index]
            if index ~= target and not member.dead and member.independent ~= nil and member.independent.cover > 0
              and not self:is_frozen(Battle.TargetGroup.PARTY, index) and math.random() * 100 < member.independent.cover then
                return index
            end
        end
        return nil
    end,

    --- Checks if a combatant is in the back row.
    is_back_row = function(self, side, index)
        if side == Battle.TargetGroup.PARTY then return Characters[self.party[index].character].back_row == true end
        return not battle_manager:get_enemy(index - 1):is_front()
    end,

    --- Gets how an enemy reacts to the elements of an attack.
    --
    -- @return The damage factor: 1 normal, 2 double, 0.5 half, 0 nullified, negative absorbed (it
    -- heals), 10 or more kills, -10 or less heals everything. Party members react as their armor,
    -- accessory and Elemental materia say (half, nullified or absorbed).
    get_element_factor = function(self, attack, side, index)
        if side == Battle.TargetGroup.PARTY then
            local defense = self.party[index].effects.elements.defense
            local factor = 1
            for _, element in ipairs(attack.elements or {}) do
                local mode = defense[element]
                if mode == Characters.ELEMENT_MODE.ABSORB then factor = factor * -1
                elseif mode == Characters.ELEMENT_MODE.NULLIFY then factor = 0
                elseif mode == Characters.ELEMENT_MODE.HALF then factor = factor * 0.5
                end
            end
            return factor
        end
        local source = battle_manager:get_enemy(index - 1)
        local factor = 1
        for _, element in ipairs(attack.elements or {}) do
            for e = 0, source:get_element_count() - 1 do
                if source:get_element(e) == element then factor = factor * source:get_element_factor(e) end
            end
        end
        return factor
    end,

    --- Calculates the result of an attack on a target.
    --
    -- Formulas as in the original game: 1 is physical, 2 is magical (modifier 5 heals, 3 takes a
    -- fraction of the current HP). Level attacks (8) only apply their effects, and some of formula 6 are done. The rest do no damage.
    -- Hit chance, criticals, rows and defending are handled.
    -- @param attack Attack data, as in {@see get_attack_data}.
    -- @param target_count Number of targets of the action (several of them reduce the damage).
    -- @return A table: hit (false if the attack misses), amount (positive damages, negative heals),
    -- mp (true if the amount is for MP).
    calculate_attack = function(self, attack, attacker_side, attacker_index, target_side, target_index, target_count)
        local data = attack.damage
        local formula, modifier, power = math.floor(data.formula or 0), math.floor(data.modifier or 0), data.power or 0
        local result = {hit = true, amount = 0, mp = (data.damage_mp or 0) == 1, extra_mp = 0}
        -- Fixed damage (Coin): it always hits and nothing changes it.
        if data.fixed_damage ~= nil then
            result.amount = data.fixed_damage
            return result
        end
        local is_item = attack.item == true
        -- Limit breaks (11) are physical attacks that check the hit chance but never are critical.
        local no_critical = false
        if formula == 11 then formula = 1; no_critical = true end
        -- Level attacks (L4 Suicide, L5 Death...): the accuracy holds the divisor, and they only affect
        -- the targets whose level is a multiple of it (a divisor of 0 affects all). Then they do the
        -- damage of their modifier (L4 Suicide: 31/32 of the current HP) and apply their effects.
        local level_attack = formula == 8
        if level_attack then
            local divisor = math.floor((data.accuracy or 0) * 255 + 0.5)
            local level = self:get_combat_stats(target_side, target_index).level
            if divisor > 0 and level % divisor ~= 0 then
                result.hit = false
                return result
            end
            formula = 2
            data = {formula = 2, modifier = modifier, power = power, accuracy = 1}
        end
        -- Accuracy is stored as the original byte divided by 255 (older installs stored 0 or 1, so 0 is treated
        -- as 1). Only the physical (1, 6, 10) and the magical (2, 7) formulas roll to hit, as in ff7.exe.
        local byte = (data.accuracy or 1) <= 0 and 255 or math.floor((data.accuracy or 1) * 255 + 0.5)
        local element = self:get_element_factor(attack, target_side, target_index)
        if not self:roll_hit(formula, byte, attacker_side, attacker_index, target_side, target_index, element, data.no_damage and 1 or power) then
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
                -- Original: power * (512 - defense) / 32.
                amount = power * (512 - math.min(t.def, 511)) / 32
            elseif formula == 2 and modifier == 3 then
                amount = t.hp * power / 32
            end
        elseif formula == 1 then
            local str = a.str + (data.attack_bonus or 0)
            local base = str + ((a.level + str) / 32) * ((a.level * str) / 32)
            amount = base * (512 - math.min(t.def, 511)) * power / (16 * 512)
            -- Small characters hit for almost nothing.
            -- Original: Berserk x1.5, Frog /4, Small 0 (so 1).
            if self:has_status(attacker_side, attacker_index, BattleStatus.SMALL) then amount = 1
            elseif self:has_status(attacker_side, attacker_index, BattleStatus.FROG) then amount = amount / 4
            end
            if self:has_status(attacker_side, attacker_index, BattleStatus.BERSERK) then amount = amount * 1.5 end
        elseif formula == 6 then
            -- Special attacks, as in the original game. TODO: Chocobuckle (6, 9) and Time Damage
            -- (6, 11) read counters of the savegame (escapes and play time), and do no damage yet.
            local target_char = target_side == Battle.TargetGroup.PARTY and Characters[self.party[target_index].character] or nil
            if modifier == 0 then amount = -a.hp -- White Wind: the HP of the caster.
            elseif modifier == 1 then amount = a.hp_max - a.hp -- ????: the HP the caster is missing.
            elseif modifier == 10 then amount = t.hp - 1 -- Heartless Angel: leaves 1 HP.
            elseif modifier == 12 then -- Everyone's Grudge: 10 per kill of a party member.
                amount = target_char ~= nil and (target_char.kills or 0) * 10 or 0
            elseif modifier == 13 then -- Aire Tam Storm: 1111 per materia the target has equipped.
                local count = 0
                if target_char ~= nil then
                    for _, equip in ipairs({target_char.weapon, target_char.armor}) do
                        for m = 0, 7 do
                            if equip ~= nil and equip.materia ~= nil and equip.materia[m] ~= nil then count = count + 1 end
                        end
                    end
                end
                amount = count * 1111
            end
        elseif formula == 2 then
            -- Magical attack stat (mag + level) * 6, not the physical one.
            if data.mp_damage then result.mp = true end
            local sum = (a.mag + a.level) * 6
            local restores = (data.restore or 0) == 1
            if revives then
                -- Life and Life2: a fraction of the maximum HP, a quarter if the power is unknown.
                amount = -t.hp_max * (power > 0 and power or 8) / 32
            elseif modifier == 5 then amount = -(sum + power * 22) -- Cure: (mag + level) * 6 + power * 22.
            elseif modifier == 3 then amount = (result.mp and t.mp or t.hp) * power / 32
            elseif modifier == 4 then
                amount = (result.mp and t.mp_max or t.hp_max) * power / 32
                if restores then amount = -amount end
            elseif modifier == 6 then
                amount = power * 20
                if restores then amount = -amount end
            elseif modifier == 7 then amount = power * (512 - math.min(t.def, 511)) / 32
            else amount = sum * (512 - math.min(t.mdef, 511)) * power / (16 * 512)
            end
        end
        -- MP Turbo adds a percent of power to the damage and the healing.
        if amount ~= 0 and (data.power_bonus or 0) > 0 then amount = amount * (100 + data.power_bonus) / 100 end
        -- Spread: attacks that hit several targets (but not the ones that always do) do 2/3 of the
        -- damage. Original: physical, Cure and magic (unless it can only be aimed at all).
        if amount ~= 0 and (target_count or 1) >= 2 and (formula == 1 or (formula == 2 and (modifier == 1 or modifier == 2 or modifier == 5)))
          and (attack.target.random or 0) ~= 1
          and not (formula == 2 and modifier ~= 5 and (attack.target.default_multiple or 0) == 1 and (attack.target.toggle_multiple or 0) == 0) then
            amount = amount * 2 / 3
        end
        if amount ~= 0 and formula ~= 6 and (modifier == 1 or modifier == 2 or modifier == 5 or modifier == 7 or modifier == 9) and not (is_item and formula ~= 3) then amount = amount * (15 + math.random()) / 16 end
        if amount > 0 and formula == 1 and not is_item then
            -- Criticals double the damage. The chance is (luck + level - target level) / 4 percent,
            -- plus the one of the weapon.
            local chance = math.max(0, (self:get_luck(attacker_side, attacker_index) + a.level - t.level) / 4) + (data.critical or 0)
            if not no_critical and ((data.always_critical or 0) == 1 or math.random() * 100 < chance) then
                amount = amount * 2
                result.critical = true
            end
            -- A defending target takes half of the physical damage (the exe halves only the physical one).
            if self:is_defending(target_side, target_index) then amount = amount / 2 end
            -- Short range attacks do half the damage from or to the back row.
            if (attack.target.short_range or 0) == 1 and not self:has_long_range(attacker_side, attacker_index)
              and (self:is_back_row(attacker_side, attacker_index) or self:is_back_row(target_side, target_index)) then
                amount = amount / 2
            end
        end
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
        if amount > 0 then result.amount = math.min(9999, math.max(1, math.floor(amount)))
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
        -- A manipulated enemy only acts on the turns of its controller.
        if side == Battle.TargetGroup.ENEMIES and self.enemies[index] ~= nil and self.enemies[index].manipulated_by ~= nil then return 0 end
        local rate = 1
        if self:has_status(side, index, BattleStatus.HASTE) then rate = rate * 2 end
        if self:has_status(side, index, BattleStatus.SLOW) then rate = rate * 0.5 end
        return rate
    end,

    --- Gets the base step of the battle clock, from the battle speed of the configuration (0 fast to 10 slow).
    -- It is the B of {@see get_atb_speed}.
    get_battle_base = function(self)
        local speed = 5
        if MenuSettings ~= nil and MenuSettings.config ~= nil then speed = MenuSettings.config.battle_speed or 5 end
        return 65536 / (2 * (120 + (speed * 255 / 10) * 480 / 256))
    end,

    --- Gets how many time units per second a timed status of a combatant counts down.
    --
    -- As in ff7.exe (0x4351da and 0x433932): Stop uses the base step B, Paralysis, Barrier, Sleep, Regen,
    -- Shield and the others the step of the combatant (B x 2, x 4 with haste, x 1 with slow or stop), and
    -- Slow-numb and Death Sentence use 34 x the same factor, with units of 4096 instead of 8192.
    get_status_speed = function(self, side, index, id)
        local base = self:get_battle_base()
        local factor = 2
        if self:has_status(side, index, BattleStatus.SLOW) then factor = 1
        elseif self:has_status(side, index, BattleStatus.HASTE) then factor = 4
        end
        if self:has_status(side, index, BattleStatus.STOP) then factor = 1 end
        local steps = self.ATB_STEPS_PER_SECOND
        if id == BattleStatus.STOP then return steps * base / 8192 end
        if id == BattleStatus.D_SENTENCE or id == BattleStatus.SLOW_NUMB then return steps * (34 * factor) / 4096 end
        return steps * (base * factor) / 8192
    end,

    --- ATB steps per second (the speed formula of the original game is per step). Tuned by feel, not
    -- read from the exe: the old battle.lua used 30, which was too slow in game, and 72 (30 * 2.4)
    -- was confirmed as the right speed on 2026-11-17.
    ATB_STEPS_PER_SECOND = 72,

    --- Gets how much the ATB of a combatant fills per second, in the scale used here (0 to 255).
    --
    -- As in the original game (ff7.exe 0x433932 and 0x43853c). The ATB has 65535 steps, and each
    -- step adds (dex + 50 for the party, dex for enemies) * 2 * rate * base / average, where base is
    -- 65536 / (2 * (120 + battle speed * 480 / 256)) and the average is the average dex of the
    -- enemies plus 50 (so strong enemies slow the party down). The rate is 1 normally, 2 with haste
    -- and 0.5 with slow.
    get_atb_speed = function(self, side, index)
        local rate = self:get_atb_rate(side, index)
        if rate <= 0 then return 0 end
        local base = self:get_battle_base()
        local dex
        if side == Battle.TargetGroup.PARTY then
            dex = Characters[self.party[index].character].stats.dex.total + 50
        else
            dex = self.enemies[index].stats.dex
        end
        local average = self.enemy_dex_average or 50
        local per_step = dex * 2 * rate * base / average
        return per_step * self.ATB_STEPS_PER_SECOND / 257
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
        local units = BattleStatus.UNITS[id]
        if combatant.status[id] == nil then
            print(self:get_combatant_name(side, index) .. " gets " .. (BattleStatus.NAME[id] or tostring(id)))
            -- Good statuses are shown in a different colour than bad ones.
            local good = id == BattleStatus.HASTE or id == BattleStatus.REGEN or id == BattleStatus.BARRIER
              or id == BattleStatus.M_BARRIER or id == BattleStatus.REFLECT or id == BattleStatus.SHIELD
              or id == BattleStatus.DUAL or id == BattleStatus.PEERLESS
            UiContainer.BattleFx:text(side, index, BattleStatus.NAME[id] or tostring(id), good and {0.5, 1, 0.8} or {0.9, 0.5, 1})
        end
        combatant.status[id] = {remaining = units, last = now, next_tick = now + BattleStatus.TICK}
        self:save_lasting_status(side, index, id, true)
        for _, other in ipairs(BattleStatus.REMOVES[id] or {}) do self:clear_status(side, index, other) end
        -- Those that can't act lose the control of who manipulates them.
        if side == Battle.TargetGroup.ENEMIES and BattleStatus.FROZEN[id] then self:release_manipulation(index) end
        self:status_changed(side, index)
    end,

    --- Keeps Fury and Sadness in the status list of the character, as they remain after the battle.
    save_lasting_status = function(self, side, index, id, present)
        if side ~= Battle.TargetGroup.PARTY or (id ~= BattleStatus.FURY and id ~= BattleStatus.SADNESS) then do return end end
        local char = Characters[self.party[index].character]
        if char.status == nil then char.status = {} end
        for key = #char.status, 1, -1 do
            if char.status[key] == id then table.remove(char.status, key) end
        end
        if present then table.insert(char.status, id) end
    end,

    --- Kernel attacks that hold the data of limits, by the technique that the character refers to. Tifa's
    -- limits (149 to 155) are the attacks 98 to 104 of the kernel. TODO: The other characters' limit data
    -- is not in the kernel (ff7.exe loads it from somewhere else when the battle starts).
    LIMIT_ATTACK_DATA = {[149] = 98, [150] = 99, [151] = 100, [152] = 101, [153] = 102, [154] = 103, [155] = 104},

    --- Animations of the party models that are empty in battle.lgp (the limit ones come from magic.lgp).
    LIMIT_MISSING_FIRST = 42,
    LIMIT_MISSING_LAST = 51,

    --- Files of magic.lgp with the animations of the limits, by the prefix of the model and the position of the
    -- technique. The installer converts them into the animations "<prefix>_<file>_00" to "_07", that the scripts
    -- of the limits refer to as the animations 44 to 51. Same table as LimitAnimationFiles in BattleDataInstaller.cpp.
    LIMIT_ANIMATION_FILES = (function()
        local cloud = {"blaver", "kyou", "limcl2", "limcl6", "limcl3", "limcl7", "limcl4"}
        local barret = {"hvshot", "limbr5", "limbr2", "limbr6", "limbr3", "limbr7", "limbr4"}
        return {
            rt = cloud, si = cloud,
            ru = {"limfast"},
            rv = {"iyash", "limea5", "limea2", "limea6", "limea3", "kodo", "limea4"},
            rw = {"limsled", "limrd5", "limsled", "limrd6", "limrd3", "limrd7", "limrd4"},
            rx = {"limyf1", "limyf5", "limyf2", "limyf6", "limyf3", "limyf7", "limyf4"},
            ry = {"dice"},
            rz = {"limcd1", "limcd5", "limcd2", "limcd6", "limcd3", "limcd7", "limcd4"},
            sb = barret, sc = barret, sd = barret, se = barret,
        }
    end)(),

    --- Number of the first animation script of the limits in the models of the party (60 in ff7.exe).
    LIMIT_FIRST_SCRIPT = 60,

    --- Gets the animation script of a limit technique.
    --
    -- As in ff7.exe (0x5c9524), it is 60 plus the position of the technique among the ones of its character,
    -- in the order of the limit levels. The scripts only exist in the models installed with the fixed
    -- installer (before, the characters had 32 scripts and the limits are in 60 and up).
    --
    -- @param index Index in the party.
    -- @param attack_id ID of the limit technique.
    -- @return The script number, nil if the model has no script for it.
    get_limit_script = function(self, index, attack_id)
        local definition = Game.Characters[self.party[index].character]
        if definition == nil then return nil end
        local position = 0
        for level = 1, 4 do
            local limit = definition.limits[level]
            for _, technique in ipairs(limit ~= nil and limit.techniques or {}) do
                if technique.command == attack_id then
                    local number = self.LIMIT_FIRST_SCRIPT + position
                    local scripts = BattleAnimations.get_scripts(self:get_model_name(Battle.TargetGroup.PARTY, index))
                    local steps = scripts ~= nil and scripts[number] or nil
                    if steps == nil or #steps == 0 then return nil end
                    local prefix = battle_manager:get_party_animation_prefix(index - 1)
                    local files = self.LIMIT_ANIMATION_FILES[prefix]
                    local tag = files ~= nil and files[position + 1] or nil
                    if tag ~= nil then return number, tag end
                    -- The animations 42 to 51 of the models are empty: the limits of most characters keep theirs in
                    -- files of magic.lgp (limcl2.a00...) that are not installed yet, so those limits use the normal
                    -- attack animation (the missing animations would only print errors).
                    -- TODO: Install the limit animation files (see the memory notes).
                    for _, step in ipairs(steps) do
                        if step.type == "anim" and step.number >= self.LIMIT_MISSING_FIRST and step.number <= self.LIMIT_MISSING_LAST then
                            return nil
                        end
                    end
                    return number
                end
                position = position + 1
            end
        end
        return nil
    end,

    --- Limits of formula 2 that are attacks, and use their data.
    LIMIT_DATA_ATTACKS = {[137] = true}, -- Mindblow (Grenade Bomb, 136, is physical)

    --- Limits whose damage goes to the MP of the targets (Mindblow).
    LIMIT_MP_DAMAGE = {[137] = true},

    --- Power of the limits that have no data and are not in SPECIAL_LIMITS (a normal attack of that power).
    LIMIT_FALLBACK_POWER = 40,

    --- Limits that are not attacks and are not in the kernel data, by the ID of the technique (the Aerith
    -- ones). They always affect a whole side. TODO: The amounts are estimates, and they have no visual effects.
    --  * target: "allies" or "enemies".
    --  * heal: power of the healing, (magic + level) * 6 + power * 22 HP for each ally.
    --  * full: HP and MP of the allies are filled.
    --  * cure: statuses of the allies that are cured.
    --  * statuses: statuses given to the targets (the enemies can be immune).
    --  * limit_bar: the limit bars of the allies, except the user, are filled.
    SPECIAL_LIMITS = {
        [145] = {target = "allies", limit_bar = true}, -- Fury Brand
    },

    --- Gets the special limit (one that is not an attack) of a technique, nil if it is an attack.
    --
    -- The limits of the kernel data that only heal, restore, or give and cure statuses are built from their
    -- data (ff7.exe, ids 128 to 183): formula 2 with modifier 2 is statuses only, 2 with modifier 4 heals a
    -- fraction (power / 32) of the maximum HP, and 0 with modifier 8 restores everything. The ones in
    -- SPECIAL_LIMITS are done by hand. Additional effects 22 and 23 (Red XIII) change stat modifiers of the user.
    --
    -- @param id Attack ID of the technique.
    get_special_limit = function(self, id)
        if self.SPECIAL_LIMITS[id] ~= nil then return self.SPECIAL_LIMITS[id] end
        local k = id ~= nil and id >= 128 and Game.Attacks[id] or nil
        if k == nil then return nil end
        local f, m = k.dmg_formula, k.dmg_modifier
        if not ((f == 2 and (m == 2 or m == 4)) or (f == 0 and m == 8)) then return nil end
        if k.target == nil then return nil end
        local special = {
            target = k.target.default_enemy == 1 and "enemies" or "allies",
            self_only = k.target.selectable == 0, statuses = {}, cures = {},
        }
        if f == 2 and m == 4 then special.heal_fraction = k.power end
        if f == 0 and m == 8 then special.full = true end
        -- Additional effects 22 (evade) and 23 (strength) change a stat modifier of the user by (modifier - 100) percent.
        local additional = k.additional
        if additional ~= nil and additional.modifier ~= nil then
            local kinds = {[22] = "eva", [23] = "str"}
            if kinds[additional.effect] ~= nil then
                special.user_mods = {{kind = kinds[additional.effect], delta = additional.modifier - 100}}
            end
        end
        for _, entry in ipairs(k.status or {}) do
            if entry.mode == 1 then table.insert(special.cures, entry.status)
            else table.insert(special.statuses, entry.status) end
        end
        return special
    end,

    --- Statuses that Breath of the Earth and Pulse of Life don't cure (the good ones, death and Near Death).
    KEPT_STATUSES = {
        [BattleStatus.DEATH] = true, [BattleStatus.NEAR_DEATH] = true, [BattleStatus.HASTE] = true,
        [BattleStatus.REGEN] = true, [BattleStatus.BARRIER] = true, [BattleStatus.M_BARRIER] = true,
        [BattleStatus.REFLECT] = true, [BattleStatus.DUAL] = true, [BattleStatus.SHIELD] = true,
        [BattleStatus.PEERLESS] = true,
    },

    --- Runs one of the limits of SPECIAL_LIMITS: the character casts, and the effect lands at the end.
    run_special_limit = function(self, current, action)
        local special = self:get_special_limit(action.attack_id)
        local index = action.actor
        self:use_limit(action)
        self:show_message(Game.Attacks[action.attack_id].name, true)
        -- The effect is not a normal hit.
        current.hit_done = true
        current.pending = 1
        local applied = false
        local function apply()
            if applied then return end
            applied = true
            -- The side of the limit, or the opposite one if the caster is confused.
            local group = action.target_group
            -- Curing Death (Pulse of Life) reaches the dead allies too.
            for _, mod in ipairs(special.user_mods or {}) do
                self:change_stat_mod(Battle.TargetGroup.PARTY, index, mod.kind, mod.delta)
            end
            local targets = current.targets
            for _, id in ipairs(special.cures or {}) do
                if id == BattleStatus.DEATH and group == Battle.TargetGroup.PARTY then
                    targets = {}
                    for i = 1, #self.party do table.insert(targets, i) end
                end
            end
            for _, t in ipairs(targets) do
                if special.heal ~= nil then
                    local stats = self:get_combat_stats(Battle.TargetGroup.PARTY, index)
                    local amount = (stats.mag + stats.level) * 6 + special.heal * 22
                    self:apply_damage(group, t, -amount, false)
                end
                if special.heal_fraction ~= nil then
                    local stats = self:get_combat_stats(group, t)
                    self:apply_damage(group, t, -math.floor(stats.hp_max * special.heal_fraction / 32), false)
                end
                -- Curing Death (Pulse of Life) brings the dead back.
                for _, id in ipairs(special.cures or {}) do
                    if id == BattleStatus.DEATH and self:get_combatant(group, t).dead then self:revive(group, t) end
                end
                if special.full then
                    local stats = self:get_combat_stats(group, t)
                    self:apply_damage(group, t, -(stats.hp_max - stats.hp), false)
                    self:apply_damage(group, t, -(stats.mp_max - stats.mp), true)
                end
                for _, id in ipairs(special.cures or {}) do
                    if id ~= BattleStatus.DEATH then self:clear_status(group, t, id) end
                end
                if special.cure then
                    local ids = {}
                    for id, _ in pairs(self:get_combatant(group, t).status) do table.insert(ids, id) end
                    for _, id in ipairs(ids) do
                        if not self.KEPT_STATUSES[id] then self:clear_status(group, t, id) end
                    end
                end
                if special.statuses ~= nil and #special.statuses > 0 then
                    local effects = {}
                    for _, id in ipairs(special.statuses) do table.insert(effects, {id = id, chance = 1, mode = "inflict"}) end
                    self:apply_attack_effects({effects = effects}, group, t)
                end
                if special.limit_bar and t ~= index then
                    local char = Characters[self.party[t].character]
                    if char.limit ~= nil and not self.party[t].dead then
                        char.limit.bar = 255
                        self:update_limit(t)
                    end
                end
            end
        end
        local played = self:play_script(
          Battle.TargetGroup.PARTY, index, self:get_script_number(Battle.TargetGroup.PARTY, index, "magic"), false,
          apply, function() apply() current.pending = current.pending - 1 end, nil
        )
        if not played then
            apply()
            current.pending = 0
        end
        self:update_action(timer:get_game_time_total())
    end,

    --- Colours of the pink windows of the limits (top-left, top-right, bottom-right and bottom-left corner).
    LIMIT_WINDOW_COLOURS = {{1, 0.55, 0.8}, {1, 0.4, 0.7}, {0.6, 0.12, 0.4}, {0.8, 0.25, 0.55}},

    --- Sets the colours of a window of the battle: the pink ones of the limits, or the normal ones of the
    -- configuration.
    --
    -- @param path Path of the window widget.
    -- @param pink True for the limit colours.
    set_window_pink = function(self, path, pink)
        local colours = {}
        if pink then
            for _, corner in ipairs(self.LIMIT_WINDOW_COLOURS) do
                for channel = 1, 3 do colours[#colours + 1] = corner[channel] end
            end
        else
            local config = MenuSettings ~= nil and MenuSettings.config ~= nil and MenuSettings.config.window_colours or nil
            config = config or {{0, 0, 178}, {0, 0, 127}, {0, 0, 32}, {0, 0, 79}}
            for _, corner in ipairs(config) do
                for channel = 1, 3 do colours[#colours + 1] = corner[channel] / 255 end
            end
        end
        ui_manager:get_widget(path .. ".Center"):set_colours(
            colours[1], colours[2], colours[3], colours[4], colours[5], colours[6],
            colours[7], colours[8], colours[9], colours[10], colours[11], colours[12]
        )
    end,

    --- Shows a text in the message window of the top of the screen.
    --
    -- @param text The message.
    -- @param pink True for the pink window of the limits.
    show_message = function(self, text, pink)
        self:set_window_pink("BattleUi.Container.Message", pink == true)
        ui_manager:get_widget("BattleUi.Container.Message.Name"):set_text(text)
        -- The pink messages of the limit bars go away by themselves.
        self.message_until = (pink == true and self.message_timed) and timer:get_game_time_total() + 2.5 or nil
        self.message_timed = false
    end,

    --- Gets the statuses that a hit of an attack would cure on a target: the physical formulas wake up, cure
    -- Confusion and end the manipulation (ff7.exe, 0x5ddb20). Magic doesn't.
    --
    -- @return A list of status IDs, empty if the attack isn't physical.
    get_physical_cures = function(self, attack, side, index)
        local formula = attack.damage.formula
        local cures = {}
        if formula == 1 or formula == 6 or formula == 10 or formula == 11 then
            for _, id in ipairs({BattleStatus.SLEEP, BattleStatus.CONFUSION, BattleStatus.MANIPULATE}) do
                if self:has_status(side, index, id) then table.insert(cures, id) end
            end
        end
        return cures
    end,

    --- Cures the statuses that {@see get_physical_cures} gave.
    apply_physical_cures = function(self, cures, side, index)
        for _, id in ipairs(cures) do
            if id == BattleStatus.MANIPULATE and side == Battle.TargetGroup.ENEMIES then self:release_manipulation(index) end
            self:clear_status(side, index, id)
        end
    end,

    --- Queues again the actions that a combatant kept while it couldn't act, if it can now.
    release_held_actions = function(self, side, index)
        local combatant = self:get_combatant(side, index)
        if combatant == nil or combatant.held == nil or self:is_frozen(side, index) then return end
        for _, action in ipairs(combatant.held) do table.insert(self.action_queue, action) end
        combatant.held = nil
    end,

    --- Applies Berserk to the action of a party member when its turn comes: anything but a plain attack becomes
    -- one, on a random enemy.
    --
    -- @param action The action, that is changed if needed.
    -- @return False if the action is dropped (the first one of a double command).
    apply_berserk = function(self, action)
        if not self:has_status(Battle.TargetGroup.PARTY, action.actor, BattleStatus.BERSERK) then return true end
        if action.command == 1 and action.attack_id == nil and action.item_id == nil then
            action.automatic, action.links = true, nil
            return true
        end
        if action.no_atb then return false end
        action.target_index = self:random_living(Battle.TargetGroup.ENEMIES)
        if action.target_index == nil then return false end
        action.target_group = Battle.TargetGroup.ENEMIES
        action.target_type = Battle.TargetType.SINGLE
        action.type = Battle.ActionType.COMMAND
        action.command = 1
        action.automatic = true
        action.limit, action.attack_id, action.item_id, action.target_list, action.links = nil, nil, nil, nil, nil
        action.summon, action.throw_item, action.coin_gil = nil, nil, nil
        return true
    end,

    --- Commands that a silenced character can't use (ff7.exe, 0x5dbde0): Magic, Summon, E.Skill, W-Magic and
    -- W-Summon.
    SILENCED_COMMANDS = {[2] = true, [3] = true, [13] = true, [21] = true, [22] = true},

    --- Checks if Silence makes an action fail, as in ff7.exe: the commands of {@see SILENCED_COMMANDS} for a party
    -- member, and the attacks that cost MP for an enemy.
    --
    -- @param action The action, with actor, actor_side (the party if nil) and command.
    -- @param attack Attack data, as in {@see get_attack_data}.
    is_silenced_action = function(self, action, attack)
        local side = action.actor_side or Battle.TargetGroup.PARTY
        if not self:has_status(side, action.actor, BattleStatus.SILENCE) then return false end
        if side == Battle.TargetGroup.PARTY then return self.SILENCED_COMMANDS[action.command] == true end
        return attack ~= nil and (attack.mp or 0) > 0
    end,

    --- Commands that a frog can use: Attack and Item. Their replacements (Slash-All, 2x-Cut, Flash, 4x-Cut,
    -- Limit and W-Item) go back to them.
    FROG_COMMANDS = {[1] = true, [4] = true},

    --- The only spell a frog can cast (Toad, that cures it), and the commands to cast it with: Magic and
    -- W-Magic (ff7.exe, 0x5dbde0).
    FROG_SPELL = 10,
    FROG_MAGIC_COMMANDS = {[2] = true, [21] = true},

    --- Checks if a command can't be chosen because the character is a frog. Magic and W-Magic can if the
    -- character has the Toad spell.
    is_frog_blocked = function(self, index, command_id)
        if self.FROG_COMMANDS[command_id] == true or not self:has_status(Battle.TargetGroup.PARTY, index, BattleStatus.FROG) then
            return false
        end
        if self.FROG_MAGIC_COMMANDS[command_id] == true then
            for _, spell in ipairs(self.party[index].magic or {}) do
                if spell.id == self.FROG_SPELL then return false end
            end
        end
        return true
    end,

    --- Applies Frog to the action of a party member when its turn comes: attacks with a replacement of the Attack
    -- command and limits become plain attacks on one enemy (the limit bar is kept), W-Item uses only the second
    -- item, and any other command but Attack, Item and the casts of Toad fails.
    --
    -- @param action The action, that is changed if needed.
    -- @return False if the action fails.
    apply_frog = function(self, action)
        if not self:has_status(Battle.TargetGroup.PARTY, action.actor, BattleStatus.FROG) then return true end
        local command = action.command
        if command == 23 then
            -- The first of the two doesn't end the turn, so it is the one that is lost.
            if action.no_atb then return false end
            action.command = 4
            return true
        end
        if action.limit ~= nil or command == 20 or (command ~= nil and command >= 24 and command <= 27) then
            if action.target_group ~= Battle.TargetGroup.ENEMIES or action.target_index == nil then
                action.target_index = self:random_living(Battle.TargetGroup.ENEMIES)
                if action.target_index == nil then return false end
            end
            action.target_group = Battle.TargetGroup.ENEMIES
            action.target_type = Battle.TargetType.SINGLE
            action.type = Battle.ActionType.COMMAND
            action.command = 1
            action.limit, action.attack_id, action.target_list, action.links = nil, nil, nil, nil
            return true
        end
        if self.FROG_MAGIC_COMMANDS[command] == true then return action.attack_id == self.FROG_SPELL end
        return self.FROG_COMMANDS[command] == true
    end,

    --- Changes the commands of a party member that depend on its state: Limit takes the place of Attack when the
    -- limit bar is full, and a frog only has the plain Attack and Item ones.
    update_commands = function(self, index)
        local member = self.party[index]
        if member.attack_command == nil then return end
        local frog = self:has_status(Battle.TargetGroup.PARTY, index, BattleStatus.FROG)
        local limit = Characters[member.character].limit
        local first = member.attack_command
        if frog then first = 1
        elseif limit ~= nil and (limit.bar or 0) >= 255 and self:has_limit_techniques(index) then first = 20
        end
        member.command[1] = first
        if member.item_slot ~= nil then member.command[member.item_slot] = frog and 4 or member.item_command end
        -- If the commands of this character are on the screen, they change on the spot.
        if self.current_character_window == index and member.manip_commands == nil
          and (self.window_state == "command" or self.window_state == "change" or self.window_state == "defend") then
            for c = 1, 12 do
                if member.command[c] ~= nil then self:set_command_widget(index, c, member.command[c]) end
            end
        end
    end,

    --- Writes a command in its place of the command window, grey if it can't be used.
    set_command_widget = function(self, index, position, command_id)
        local widget = ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cmd" .. tostring(position))
        -- The colour goes before the text. Manipulate is not available while controlling an enemy.
        if self:is_frog_blocked(index, command_id)
          or (command_id == self.MANIP_COMMAND and self.party[index].manipulating ~= nil) then
            widget:set_colour(0.5, 0.5, 0.5)
        else
            widget:set_colour(1, 1, 1)
        end
        widget:set_text(Game.Commands[command_id].name)
    end,

    --- Message of ff7.exe (battle text 91) for a character that can't pay a spell.
    NOT_ENOUGH_MP = "Not enough MP!!",

    --- Shows a message that goes away by itself after a moment.
    show_timed_message = function(self, text)
        self:show_message(text)
        self.message_until = timer:get_game_time_total() + 2
    end,

    --- Takes from an enemy the MP of an attack, as ff7.exe does for every combatant (0x5dbde0).
    --
    -- @param index Index of the enemy.
    -- @param attack Attack data, as in {@see get_attack_data}.
    -- @return False if the enemy doesn't have enough MP: nothing is taken, and the attack fails.
    pay_enemy_mp = function(self, index, attack)
        local cost = attack.mp or 0
        if cost <= 0 then return true end
        local stats = self.enemies[index].stats
        if stats.mp < cost then return false end
        stats.mp = stats.mp - cost
        return true
    end,

    --- Damage divisor of the limit bar by limit level, only for installs without the ones of the characters
    -- (Game.Characters[id].limit_divisors). As in ff7.exe (0x5df1b0) the bar gets damage * 300 / maximum HP * 256 /
    -- divisor (x2 with Fury, half with Sadness).
    LIMIT_DIVISOR = {150, 200, 250, 300},

    --- Checks if a party member has a limit technique that it can use: one that was learned in its limit level.
    has_limit_techniques = function(self, index)
        local char = Characters[self.party[index].character]
        local learned = char.limit ~= nil and char.limit.learned ~= nil and char.limit.learned[char.limit.current] or nil
        if learned == nil then return false end
        for _, value in pairs(learned) do if value == true then return true end end
        return false
    end,

    --- Fills the limit bar of a party member with the damage it takes from the enemies.
    --
    -- @param index Index in the party.
    -- @param damage HP lost.
    add_limit = function(self, index, damage)
        local member = self.party[index]
        local char = Characters[member.character]
        if member.dead or char.limit == nil or damage <= 0 then return end
        local stats = self:get_combat_stats(Battle.TargetGroup.PARTY, index)
        local gain = math.floor(damage * 300 / math.max(1, stats.hp_max))
        local factor = 1
        if self:has_status(Battle.TargetGroup.PARTY, index, BattleStatus.FURY) then factor = 2
        elseif self:has_status(Battle.TargetGroup.PARTY, index, BattleStatus.SADNESS) then factor = 0.5
        end
        local definition = Game.Characters[member.character]
        local divisors = definition ~= nil and definition.limit_divisors or self.LIMIT_DIVISOR
        local divisor = divisors[char.limit.current] or divisors[1]
        char.limit.bar = math.min(255, (char.limit.bar or 0) + math.floor(gain * 256 * factor / divisor))
        self:update_limit(index)
    end,

    --- Shows the limit bar of a party member (blue with Sadness, red with Fury), and puts the Limit command
    -- in place of the Attack one when the bar is full and the character has a limit to use.
    update_limit = function(self, index)
        local member = self.party[index]
        local char = Characters[member.character]
        if char.limit == nil then
            self:update_commands(index)
            return
        end
        local bar = char.limit.bar or 0
        -- The message of a limit bar that has just got full (not at the start of the battle).
        local full = bar >= 255
        if member.limit_full == false and full then
            self.message_timed = true
            self:show_message(self:get_combatant_name(Battle.TargetGroup.PARTY, index) .. " limit break!", true)
        end
        member.limit_full = full
        local widget = ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(index) .. ".Right.LimitFill")
        local width_percent = math.min(18, 18 * bar / 255)
        widget:set_width(width_percent, math.max(-6, -6 * width_percent / 18))
        if self:has_status(Battle.TargetGroup.PARTY, index, BattleStatus.SADNESS) then widget:set_colour(0.4, 0.6, 1)
        elseif self:has_status(Battle.TargetGroup.PARTY, index, BattleStatus.FURY) then widget:set_colour(1, 0.3, 0.3)
        else widget:set_colour(1, 0, 1)
        end
        self:update_commands(index)
    end,

    --- Colours of the full limit bar, that flashes between them.
    LIMIT_FLASH_COLOURS = {{1, 0, 1}, {0.4, 0.6, 1}, {1, 0.3, 0.3}},

    --- Makes the full limit bars flash, and the "Limit" command change colours.
    --
    -- A full bar goes through its regular colour, blue and red (whatever status the character has). The
    -- "Limit" command goes through the colours of the rainbow.
    -- TODO: The fields have a rainbow with a colour for each letter in their dialogs (the installer writes it
    -- from the original text codes), but the engine can't draw it yet: here all the word has the same colour.
    update_limit_flash = function(self, now)
        local step = math.floor(now * 15)
        if step == self.limit_flash_step then return end
        self.limit_flash_step = step
        local phase = math.floor(now * 4) % 3 + 1
        for index = 1, #self.party do
            local char = Characters[self.party[index].character]
            if char.limit ~= nil and (char.limit.bar or 0) >= 255 and not self.party[index].dead then
                local colour = self.LIMIT_FLASH_COLOURS[phase]
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(index) .. ".Right.LimitFill"):set_colour(colour[1], colour[2], colour[3])
            end
        end
        -- The command Limit of the character that is choosing.
        local current = self.current_character_window
        if (self.window_state == "command" or self.window_state == "change" or self.window_state == "defend") and current ~= nil
          and self.party[current] ~= nil and self.party[current].manip_commands == nil then
            for c = 1, 12 do
                if self.party[current].command[c] == 20 then
                    local hue = (now * 1.5) % 1
                    local r = math.max(0, math.min(1, math.abs(hue * 6 - 3) - 1))
                    local g = math.max(0, math.min(1, 2 - math.abs(hue * 6 - 2)))
                    local b = math.max(0, math.min(1, 2 - math.abs(hue * 6 - 4)))
                    local widget = ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cmd" .. tostring(c))
                    widget:set_colour(r, g, b)
                    widget:set_text(Game.Commands[20].name)
                end
            end
        end
    end,

    --- Uses the limit bar: it is emptied, and the uses of the first technique of the level unlock the second one.
    use_limit = function(self, action)
        local member = self.party[action.actor]
        local char = Characters[member.character]
        local level, technique = action.limit.level, action.limit.index
        char.limit.bar = 0
        if technique == 1 then
            char.limit.uses[level] = (char.limit.uses[level] or 0) + 1
            local definition = Game.Characters[member.character]
            local second = definition ~= nil and definition.limits[level] ~= nil and definition.limits[level].techniques[2] or nil
            if second ~= nil and second.uses ~= nil and second.uses > 0 and char.limit.uses[level] >= second.uses
              and not char.limit.learned[level][2] then
                char.limit.learned[level][2] = true
                print(char.name .. " learns the second limit of level " .. tostring(level))
                self:announce_limit(member.character, second.command)
            end
        end
        self:update_limit(action.actor)
    end,

    --- Sets or clears Near Death in a combatant, depending on its HP: it has it with a quarter of the
    -- maximum HP or less (and some HP left), as in ff7.exe (0x5c7d37). It isn't announced, and it ends by
    -- itself when the HP is back. The HP of the party is shown in yellow while it lasts.
    update_near_death = function(self, side, index)
        local combatant = self:get_combatant(side, index)
        if combatant == nil or combatant.dead or combatant.status == nil then return end
        local stats = self:get_combat_stats(side, index)
        local critical = stats.hp > 0 and stats.hp <= stats.hp_max / 4
        local has = combatant.status[BattleStatus.NEAR_DEATH] ~= nil
        if critical and not has then
            combatant.status[BattleStatus.NEAR_DEATH] = {next_tick = timer:get_game_time_total() + BattleStatus.TICK}
            self:status_changed(side, index)
        elseif not critical and has then
            combatant.status[BattleStatus.NEAR_DEATH] = nil
            self:status_changed(side, index)
        end
    end,

    --- Cures a status of a combatant.
    clear_status = function(self, side, index, id)
        local combatant = self:get_combatant(side, index)
        if combatant.status[id] == nil then do return end end
        combatant.status[id] = nil
        self:save_lasting_status(side, index, id, false)
        print(self:get_combatant_name(side, index) .. " is cured of " .. (BattleStatus.NAME[id] or tostring(id)))
        self:status_changed(side, index)
    end,

    --- Updates what depends on the statuses of a combatant after they change.
    status_changed = function(self, side, index)
        self:release_held_actions(side, index)
        if side == Battle.TargetGroup.PARTY then
            -- Someone that can't act, or that acts by itself, can't choose a command.
            if self:is_frozen(side, index) or self:acts_by_itself(side, index) then self:remove_from_command_queue(index) end
            self:update_name_colours()
            self:update_limit(index)
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
            ui_manager:get_widget("BattleUi.Container.Bottom.Change"):set_visible(false)
            ui_manager:get_widget("BattleUi.Container.Bottom.Defend"):set_visible(false)
            self.current_list = "none"
            self.window_state = "wait"
        end
    end,

    --- Checks if a combatant is immune to a status. Party members are protected by their
    -- accessory and by Added Effect materia linked in the armor.
    is_immune = function(self, side, index, id)
        if side == Battle.TargetGroup.PARTY then return self.party[index].effects.status.defense[id] == true end
        local source = battle_manager:get_enemy(index - 1)
        for i = 0, source:get_immunity_count() - 1 do
            if source:get_immunity(i) == id and math.random() < source:get_immunity_rate(i) then return true end
        end
        return false
    end,

    --- Rolls if the statuses of an attack work on a target, as in ff7.exe (0x5dc099). The chance of the data is
    -- a byte from 0 to 63 and it is worth 4 percent points each: 63 always works. MP Turbo raises it, and attacks
    -- split between several targets lose a third of it. Removing Frog or Small from who has it always works, and so
    -- do Haste, Shield and Berserk on party members.
    --
    -- @param attack Attack data, as in {@see get_attack_data}, with at least one effect.
    -- @param target_count Number of targets of the action, one if nil.
    -- @return True if the statuses work.
    roll_status = function(self, attack, side, index, target_count)
        local effects = attack.effects
        local points = math.floor(math.abs(effects[1].chance) * 63 + 0.5) * 4
        if #effects == 1 and (effects[1].id == BattleStatus.FROG or effects[1].id == BattleStatus.SMALL)
          and self:has_status(side, index, effects[1].id) then
            return true
        end
        if side == Battle.TargetGroup.PARTY then
            for _, effect in ipairs(effects) do
                if effect.id == BattleStatus.HASTE or effect.id == BattleStatus.SHIELD or effect.id == BattleStatus.BERSERK then
                    return true
                end
            end
        end
        if points >= 252 then return true end
        local bonus = attack.damage ~= nil and attack.damage.power_bonus or 0
        points = points + math.floor(points * bonus / 100)
        local target = attack.target or {}
        if (target_count or 1) >= 2 and target.random ~= 1
          and not (target.default_multiple == 1 and target.toggle_multiple == 0) then
            points = math.floor(points * 2 / 3)
        end
        return math.random(1, 100) < points
    end,

    --- Applies the status effects of an attack to a target.
    --
    -- Effects have a mode (inflict, cure or toggle), and a chance between 0 and 1.
    apply_attack_effects = function(self, attack, side, index, target_count)
        -- The original rolls once for all the statuses of an attack.
        local roll = nil
        for _, effect in ipairs(attack.effects) do
            if roll == nil then roll = self:roll_status(attack, side, index, target_count) end
            local has = self:has_status(side, index, effect.id)
            local opposite = effect.id == BattleStatus.FURY and BattleStatus.SADNESS
              or (effect.id == BattleStatus.SADNESS and BattleStatus.FURY or nil)
            if opposite ~= nil and effect.mode ~= "cure" then
                -- Hyper and Tranquilizer: they cure the opposite status if the target has it, and
                -- otherwise induce their own (nothing happens if the target already has it).
                if roll and not has then
                    if self:has_status(side, index, opposite) then self:clear_status(side, index, opposite)
                    else self:set_status(side, index, effect.id)
                    end
                end
            elseif effect.mode == "cure" or (effect.mode == "toggle" and has) then
                if has and roll then self:clear_status(side, index, effect.id) end
            elseif not self:is_immune(side, index, effect.id) and roll then
                if effect.id == BattleStatus.DEATH then
                    -- Death takes all the HP.
                    local stats = self:get_combat_stats(side, index)
                    if stats.hp > 0 then self:apply_damage(side, index, stats.hp, false) end
                elseif effect.id == BattleStatus.NEAR_DEATH then
                    -- Near Death follows the HP, see update_near_death.
                else
                    self:set_status(side, index, effect.id)
                end
            end
        end
    end,

    --- Updates the timed statuses: expiration, and the damage of poison and the healing of regen.
    update_statuses = function(self, now)
        -- Final Attacks of the characters that die of a status go at the front of the queue.
        self.ai_counter_count = 0
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
                            -- Timed statuses count down their units, faster with haste and slower with slow.
                            local expired = false
                            if status.remaining ~= nil then
                                status.remaining = status.remaining - math.min(0.5, now - (status.last or now)) * self:get_status_speed(side, index, id)
                                status.last = now
                                expired = status.remaining <= 0
                            end
                            if expired then
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
        -- The HP are yellow with Near Death (the colour has to be set before the text).
        local hp_widget = ui_manager:get_widget(base .. "Hp.HpCurrent")
        if self:has_status(Battle.TargetGroup.PARTY, index, BattleStatus.NEAR_DEATH) then hp_widget:set_colour(1, 1, 0)
        else hp_widget:set_colour(1, 1, 1)
        end
        hp_widget:set_text(string.format("%5d", stats.hp.current))
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
            if char.stats.hp.current > 0 then self:update_near_death(side, index) end
            self:update_party_bars(index)
            if char.stats.hp.current == 0 then
                self.party[index].dead = true
                self.party[index].atb = 0
                self.party[index].status = {}
                self.party[index].held = nil
                -- A character that dies loses its limit bar.
                if char.limit ~= nil then char.limit.bar = 0 end
                self:update_limit(index)
                self:update_name_colours()
                for i = #self.command_queue, 1, -1 do
                    if self.command_queue[i] == index then table.remove(self.command_queue, i) end
                end
                self:queue_final_attack(index)
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
            if enemy.stats.hp > 0 then self:update_near_death(side, index) end
            if enemy.stats.hp == 0 then
                enemy.dead = true
                enemy.status = {}
                enemy.held = nil
                -- The model is hidden once the death animation ends, see update_action.
            end
        end
    end,

    --- Notes that a character learned a limit technique, to show it with the results of the battle.
    announce_limit = function(self, char_id, attack_id)
        local attack = Game.Attacks[attack_id]
        if attack == nil then return end
        self.limit_learned = self.limit_learned or {}
        local name = string.gsub(attack.name or "", "^[^%w]+", "")
        table.insert(self.limit_learned, Characters[char_id].name .. " learned " .. name .. "!")
    end,

    --- Gives the first limit technique of the next levels to the characters that have enough kills.
    --
    -- The levels need the kills of the kernel data (Game.Characters[].limits[level].kills, 0 for the ones that
    -- are not got with kills). The level that is being used does not change.
    update_limit_levels = function(self)
        for i = 1, #self.party do
            local char_id = self.party[i].character
            local char = Characters[char_id]
            local definition = Game.Characters[char_id]
            if char.limit ~= nil and definition ~= nil and definition.limits ~= nil then
                for level = 2, #definition.limits do
                    local limit = definition.limits[level]
                    local learned = char.limit.learned[level]
                    if limit ~= nil and (limit.kills or 0) > 0 and (char.kills or 0) >= limit.kills
                      and not (learned ~= nil and learned[1] == true) then
                        char.limit.learned[level] = learned or {}
                        char.limit.learned[level][1] = true
                        if limit.techniques[1] ~= nil then self:announce_limit(char_id, limit.techniques[1].command) end
                        print(char.name .. " learns the limit level " .. tostring(level))
                    end
                end
            end
        end
    end,

    --- Checks if an enemy was defeated: it is dead or petrified.
    is_defeated = function(self, index)
        return self.enemies[index].dead == true
          or self:has_status(Battle.TargetGroup.ENEMIES, index, BattleStatus.PETRIFY)
    end,

    --- Rolls the item an enemy drops, as in ff7.exe (0x5c7c1e): its items are tried in order, each with its chance
    -- out of 64, and the first one that passes is the drop.
    --
    -- @param source Enemy data.
    -- @return The item ID, nil if it drops nothing.
    roll_drop = function(self, source)
        for d = 0, source:get_drop_count() - 1 do
            local id = source:get_drop_id(d)
            if id >= 0 and math.random(0, 63) <= math.floor(source:get_drop_rate(d) * 63 + 0.5) then return id end
        end
        return nil
    end,

    --- Calculates the rewards for winning the battle and gives them to the party.
    --
    -- As in ff7.exe, each party member that isn't dead or petrified gets all the EXP and AP, and the
    -- available characters that aren't in the party get half of the EXP.
    -- Only the defeated enemies (dead or petrified) give gil, EXP, AP and items, one item at most each.
    -- @return A list of strings with the results, one per result page.
    apply_rewards = function(self)
        local gil, exp, ap = 0, 0, 0
        local items = {}
        for i = 1, #self.enemies do
            if self:is_defeated(i) then
                gil = gil + self.enemies[i].money
                exp = exp + self.enemies[i].exp
                ap = ap + self.enemies[i].ap
                local source = battle_manager:get_enemy(i - 1)
                -- A morphed enemy gives its morph item right away (Morph), and no drops.
                if source ~= nil and not self.enemies[i].morphed then
                    local id = self:roll_drop(source)
                    if id ~= nil then table.insert(items, id) end
                end
            end
        end
        local alive = {}
        for i = 1, #self.party do
            if self.party[i].dead == false and not self:has_status(Battle.TargetGroup.PARTY, i, BattleStatus.PETRIFY) then
                table.insert(alive, i)
            end
        end
        local level_ups = {}
        local function give_exp(char_id, amount)
            if Characters.add_exp(char_id, amount) > 0 then
                local text = Characters[char_id].name .. " Lv " .. tostring(Characters[char_id].level)
                table.insert(level_ups, text)
                print(text)
            end
        end
        -- Gil Plus (the best one of the party) gives more gil.
        local gil_plus = 1
        for i = 1, #self.party do
            if self.party[i].independent ~= nil then gil_plus = math.max(gil_plus, self.party[i].independent.gil_plus) end
        end
        gil = math.floor(gil * gil_plus)
        for _, i in ipairs(alive) do
            local char = Characters[self.party[i].character]
            -- EXP Plus gives more EXP to the character that has it.
            give_exp(self.party[i].character, math.floor(exp * ((self.party[i].independent or {}).exp_plus or 1)))
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
        -- The materia may have gained a level, and the ones that change the stats do it by level.
        for _, i in ipairs(alive) do Characters.recalculate_stats(self.party[i].character) end
        -- The reserve: everybody available that didn't fight.
        local fought = {}
        for i = 1, #self.party do fought[self.party[i].character] = true end
        for char_id = 0, 8 do
            local char = Characters[char_id]
            if not fought[char_id] and IFMEMBQ(char_id) then
                local hp_base, mp_base = char.stats.hp.base, char.stats.mp.base
                give_exp(char_id, math.floor(exp / 2))
                -- Unlike the ones that fought, they get the new HP and MP right away, if alive.
                if char.stats.hp.current > 0 then
                    char.stats.hp.current = char.stats.hp.current + math.max(0, char.stats.hp.base - hp_base)
                    char.stats.mp.current = char.stats.mp.current + math.max(0, char.stats.mp.base - mp_base)
                end
            end
        end
        Inventory.add_money(gil)

        local pages = {}
        table.insert(pages, "EXP " .. tostring(exp) .. "  AP " .. tostring(ap))
        for _, text in ipairs(level_ups) do table.insert(pages, text) end
        table.insert(pages, tostring(gil) .. " Gil")
        for _, id in ipairs(items) do
            if Game.Items[id] ~= nil then
                Inventory.add_item(id, 1)
                table.insert(pages, "Got " .. Game.Items[id].name)
            end
        end
        -- The limit techniques learned (kills and uses).
        self:update_limit_levels()
        for _, text in ipairs(self.limit_learned or {}) do table.insert(pages, text) end
        self.limit_learned = nil
        print("Battle rewards: " .. tostring(exp) .. " EXP, " .. tostring(ap) .. " AP, " .. tostring(gil) .. " gil, " .. tostring(#items) .. " items.")
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
        self:show_message(self.result_pages[self.result_page])
    end,

    --- Leaves the battle, and starts the next one if the formation has any.
    finish_battle = function(self)
        local next_battle = self.pending_next_battle
        self.pending_next_battle = -1
        -- The camera of the victory stays until here: it must not go on in the next battle.
        BattleCameraController:reset()
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
        for i = 1, #self.enemies do if not self:is_defeated(i) then enemies_alive = true end end
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
                local ok, err = pcall(BattleCameraController.start_victory, BattleCameraController)
                if not ok then print("Battle camera: " .. tostring(err)) BattleCameraController:reset() end
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
        local tag = self.next_animation_tag
        self.next_animation_tag = nil
        local entity = self:get_entity(side, index)
        local home = nil
        if entity ~= nil then
            local x, y, z = entity:get_position()
            home = {x = x, y = y, z = z}
            -- If a previous script left the combatant away, its home is the one to go back to.
            if previous ~= nil and previous.home ~= nil then home = previous.home end
            -- A party member always goes back to its place, even if it was covering someone.
            if side == Battle.TargetGroup.PARTY and self.party[index] ~= nil and self.party[index].home ~= nil then
                home = self.party[index].home
            end
        end
        self.animation_queues[key] = {
            side = side, index = index, steps = steps, step = 1, time = timer:get_game_time_total(),
            stay = stay, on_hit = on_hit, on_done = on_done, target = target, home = home, tag = tag,
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
                        -- The animations 44 to 51 of a limit are the ones of its own file.
                        if q.tag ~= nil and step.number >= 44 and step.number <= 51 and name ~= nil then
                            name = string.format("%s_%s_%02d", battle_manager:get_party_animation_prefix(q.index - 1), q.tag, step.number - 44)
                        end
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
                    elseif step.type == "camera" or step.type == "view" then
                        -- Nothing replaces the camera of the victory.
                        if self.ending ~= true then
                            local start = step.type == "camera" and BattleCameraController.start_script
                              or BattleCameraController.set_view
                            local ok, err = pcall(start, BattleCameraController, step.number)
                            if not ok then print("Battle camera: " .. tostring(err)) BattleCameraController:reset() end
                            if step.type == "camera" then print("Battle camera: animation, script " .. tostring(step.number)) end
                        end
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
        self:show_message(string.format("Animation speed: x%.2f", BattleAnimations.SPEED))
        print("Animation speed: " .. tostring(BattleAnimations.SPEED))
    end,

    --- Gets the camera IDs of an action: the ones of the item, of the limit, or of the attack, and for an enemy
    -- the one it has for that attack.
    --
    -- @return The ID for one target and the one for several, nil if the action has no camera.
    get_action_camera = function(self, side, action, attack)
        if action.item_id ~= nil then
            local item = Game.Items[action.item_id]
            if item == nil then return nil end
            return item.camera, item.camera
        end
        if action.attack_id == nil then return nil end
        -- A limit has its own, even if it takes its data from another attack of the kernel.
        local single, multiple
        local own = action.limit ~= nil and Game.Attacks[action.attack_id] or nil
        if own ~= nil and own.camera ~= nil then single, multiple = own.camera.single, own.camera.multiple
        else
            if attack.animation ~= nil then single, multiple = attack.animation.camera_single, attack.animation.camera_multiple end
            local kernel = attack.id ~= nil and Game.Attacks[attack.id] or nil
            if single == nil and kernel ~= nil and kernel.camera ~= nil then single, multiple = kernel.camera.single, kernel.camera.multiple end
        end
        -- The camera of the attack of an enemy overrides the one of the attack.
        if side == Battle.TargetGroup.ENEMIES then
            local source = battle_manager:get_enemy(action.actor - 1)
            for slot = 0, source:get_attack_count() - 1 do
                if source:get_attack(slot) == action.attack_id then
                    local camera = source:get_camera_for_attack(slot)
                    if camera ~= nil and camera >= 0 and camera < 0xffff then single, multiple = camera, camera end
                    break
                end
            end
        end
        return single, multiple
    end,

    --- Starts the camera of an action, if it has one.
    start_action_camera = function(self, side, action, attack, targets)
        local ok, err = pcall(function()
            local single, multiple = self:get_action_camera(side, action, attack)
            if single == nil and multiple == nil then return end
            BattleCameraController:start(side, action.actor, action.target_group, targets, single, multiple)
        end)
        if not ok then print("Battle camera: " .. tostring(err)) end
    end,

    --- Starts executing the first action in the queue.
    --
    -- The attacker plays its attack script. The damage is applied when the script marks the hit,
    -- (or at the end, if it doesn't) and the action is over when every animation is done.
    execute_action = function(self)
        if self.action_executing == true or #self.action_queue == 0 then do return 0 end end
        local action = table.remove(self.action_queue, 1)
        self.action_executing = true
        self.ai_counter_count = 0
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
        -- Those that can't act keep their actions for later. Counterattacks and the orders to manipulated enemies
        -- are lost.
        if self:is_frozen(actor_side, action.actor) and not action.final_attack then
            if not action.is_counter and action.controller == nil then
                actor.held = actor.held or {}
                table.insert(actor.held, action)
                self.current_action = nil
                self.action_executing = false
                return
            end
            current.hit_done = true
            current.targets = {}
            self:update_action(timer:get_game_time_total())
            return
        end
        -- A berserk member attacks, whatever it chose before.
        if actor_side == Battle.TargetGroup.PARTY and not self:apply_berserk(action) then
            current.hit_done = true
            current.targets = {}
            self:update_action(timer:get_game_time_total())
            return
        end
        -- A frog changes or loses the actions that it chose before.
        if actor_side == Battle.TargetGroup.PARTY and not self:apply_frog(action) then
            print(self:get_combatant_name(actor_side, action.actor) .. " can't do that as a frog")
            current.hit_done = true
            current.targets = {}
            self:update_action(timer:get_game_time_total())
            return
        end
        -- Mime repeats the last action of the party, as a copy that doesn't use MP (items are used up again).
        if actor_side == Battle.TargetGroup.PARTY and action.command == self.MIME_COMMAND and action.attack_id == nil then
            local copy, reason = self:build_mime_action(action)
            if copy == nil then
                self:show_message(reason)
                current.hit_done = true
                self:update_action(timer:get_game_time_total())
                return
            end
            action = copy
            current.action = copy
        end
        -- Change and Defend.
        if actor_side == Battle.TargetGroup.PARTY and (action.command == 18 or action.command == 19) then
            self:run_side_command(current, action)
            return
        end
        -- The last action of the party, for Mime. A mimed action leaves the one that was copied, so it can be mimed again.
        if actor_side == Battle.TargetGroup.PARTY and not action.mimed and not action.is_counter and not action.final_attack
          and not action.sneak_attack and action.controller == nil and not action.automatic then
            local last = {}
            for key, value in pairs(action) do last[key] = value end
            self.last_party_action = last
        end
        local attack = self:get_action_attack(action)
        local revives = self:attack_revives(attack)
        if not self:apply_confusion(action, attack) then
            current.hit_done = true
            self:update_action(timer:get_game_time_total())
            return
        end
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
            elseif not revives and not action.no_retarget then
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
        -- Allies with Cover materia run in front of a member with critical HP that is the target of an
        -- attack of an enemy, and take it. They go back when the action is over.
        if actor_side == Battle.TargetGroup.ENEMIES and not action.is_counter and action.target_group == Battle.TargetGroup.PARTY
          and #current.targets == 1 and (attack.damage.formula == 1 or attack.damage.formula == 2) then
            local covered = current.targets[1]
            local cover = self:find_cover(covered)
            if cover ~= nil then
                local cover_entity = self:get_entity(Battle.TargetGroup.PARTY, cover)
                local covered_entity = self:get_entity(Battle.TargetGroup.PARTY, covered)
                if cover_entity ~= nil and covered_entity ~= nil then
                    local home = self.party[cover].home
                    if home == nil then
                        local hx, hy, hz = cover_entity:get_position()
                        home = {x = hx, y = hy, z = hz}
                    end
                    local x, y, z = covered_entity:get_position()
                    local dx, dy = self:get_row_direction(x, y)
                    current.cover = {covered = covered, index = cover, home = home}
                    self:slide_party(cover, {x = x + dx * self.COVER_DISTANCE, y = y + dy * self.COVER_DISTANCE, z = z}, self.COVER_MOVE_TIME)
                else
                    current.cover = {covered = covered, index = cover}
                end
            end
        end
        -- Counterattacks that lost their target (it died before they happened) are cancelled.
        if action.no_retarget and #current.targets == 0 then
            print("The counterattack has no target anymore.")
            current.hit_done = true
            self:update_action(timer:get_game_time_total())
            return
        end
        -- Limits that are not attacks.
        if actor_side == Battle.TargetGroup.PARTY and action.limit ~= nil and self:get_special_limit(action.attack_id) ~= nil then
            self:start_action_camera(actor_side, action, attack, current.targets)
            self:run_special_limit(current, action)
            return
        end
        local script = action.script
        if script == nil and actor_side == Battle.TargetGroup.PARTY then
            local script_type = (action.attack_id ~= nil or action.item_id ~= nil or action.coin_gil ~= nil) and "magic" or "attack"
            if action.throw_item ~= nil or action.coin_gil ~= nil then script_type = "throw" end
            if action.command == self.MANIP_COMMAND and action.attack_id == nil then script_type = "manip" end
            -- The physical limits use the animation of the attack.
            if action.limit ~= nil and (attack.damage.formula == 1 or attack.damage.formula == 11) then script_type = "attack" end
            -- The animation of the limit itself, if the model has it.
            local limit_script, limit_tag = nil, nil
            if action.limit ~= nil then limit_script, limit_tag = self:get_limit_script(action.actor, action.attack_id) end
            self.next_animation_tag = limit_tag
            -- A limit that hits several times has no hit marks in its script: the hits start with the animations of
            -- the limit itself (44 to 51) and are spread over their duration.
            if limit_script ~= nil and limit_tag ~= nil and attack.hits ~= nil then
                local steps = BattleAnimations.get_scripts(self:get_model_name(Battle.TargetGroup.PARTY, action.actor))[limit_script]
                local entity = self:get_entity(Battle.TargetGroup.PARTY, action.actor)
                local prefix = battle_manager:get_party_animation_prefix(action.actor - 1)
                local marked, span = {}, 0
                for _, step in ipairs(steps) do
                    if step.type == "anim" and step.number >= 44 and step.number <= 51 then
                        if span == 0 then table.insert(marked, {type = "hit"}) end
                        local name = string.format("%s_%s_%02d", prefix, limit_tag, step.number - 44)
                        local length = entity ~= nil and entity:get_animation_length(name) or 0
                        span = span + math.max(length / BattleAnimations.SPEED, BattleAnimations.MIN_FRAME_TIME)
                    end
                    table.insert(marked, step)
                end
                current.limit_span = span
                limit_script = marked
            end
            if action.command == Characters.E_SKILL_COMMAND and action.attack_id ~= nil then script_type = "eskill" end
            script = limit_script or self:get_script_number(actor_side, action.actor, script_type)
        end
        -- Using things costs something. If it can't be paid anymore, the action fizzles.
        local function fizzle(message)
            print(message)
            current.hit_done = true
            current.targets = {}
            current.pending = 0
            self:update_action(timer:get_game_time_total())
        end
        -- Silence stops the actions that were chosen before it, too. Nothing is paid.
        if self:is_silenced_action(action, attack) then
            fizzle(self:get_combatant_name(actor_side, action.actor) .. " is silenced")
            return
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
            elseif action.coin_gil ~= nil then
                -- The gil is spent.
                if (Inventory.money or 0) < action.coin_gil then fizzle("Not enough gil") return end
                Inventory.money = Inventory.money - action.coin_gil
            elseif action.throw_item ~= nil then
                -- The thrown thing is gone.
                local owned = 0
                for slot = 0, MAX_INVENTORY_SLOTS do
                    if Inventory[slot] ~= nil and Inventory[slot].item == action.throw_item then owned = owned + Inventory[slot].quantity end
                end
                if owned < 1 then fizzle("No more " .. attack.name) return end
                Inventory.remove_item(action.throw_item, 1)
            elseif action.attack_id ~= nil and not action.mimed then
                local cost = self:get_spell_cost(attack.mp, action.links)
                local can_pay = stats.mp.current >= cost
                -- Quadra Magic casts the spell four times with half of its power, for the MP of one cast, a
                -- number of times per battle. TODO: Unverified: the targets of each cast.
                if action.links ~= nil and (action.links.quadra or 0) > 0 and can_pay then
                    local used = self.party[action.actor].quadra_used[action.attack_id] or 0
                    if used < action.links.quadra then
                        self.party[action.actor].quadra_used[action.attack_id] = used + 1
                        current.quadra = true
                    end
                end
                if cost > 0 then
                    if not can_pay then
                        self:show_timed_message(self.NOT_ENOUGH_MP)
                        fizzle("Not enough MP for " .. attack.name)
                        return
                    end
                    self:apply_damage(actor_side, action.actor, cost, true)
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
        -- Enemy frogs can't use the attacks that cost MP.
        if actor_side == Battle.TargetGroup.ENEMIES and (attack.mp or 0) > 0
          and self:has_status(actor_side, action.actor, BattleStatus.FROG) then
            fizzle(self:get_combatant_name(actor_side, action.actor) .. " can't use " .. attack.name .. " as a frog")
            return
        end
        if actor_side == Battle.TargetGroup.ENEMIES and not self:pay_enemy_mp(action.actor, attack) then
            -- The window only shows the name of the attack, and nothing happens.
            self:show_timed_message(attack.name)
            fizzle(self:get_combatant_name(actor_side, action.actor) .. " doesn't have enough MP for " .. attack.name)
            return
        end
        -- Limits empty the bar.
        if actor_side == Battle.TargetGroup.PARTY and action.limit ~= nil then self:use_limit(action) end
        -- Widening an action that has one target takes a use of All (spells with the materia linked) or of
        -- Mega All.
        if actor_side == Battle.TargetGroup.PARTY and not action.mimed and not self:spend_all(action, attack) then
            fizzle("No more All left for " .. attack.name)
            return
        end
        -- Spells, summons, items and enemy attacks show their names.
        if action.attack_id ~= nil or action.item_id ~= nil then
            self:show_message(attack.name, action.limit ~= nil)
        end
        -- Summons play their own sequence after the cast pose, and the hit lands during it.
        local summon = action.attack_id ~= nil and UiContainer.BattleFx:has_summon(attack.id)
        local summon_started = false
        local function start_summon()
            if summon_started then do return end end
            summon_started = true
            UiContainer.BattleFx:play_summon(
              attack, actor_side, action.actor, action.target_group, current.targets,
              function() self:apply_action_hit() end,
              function() current.pending = current.pending - 1 end
            )
        end
        -- Thrown things fly to the target, and the hit lands when they arrive.
        local throw = (action.throw_item ~= nil or action.coin_gil ~= nil) and actor_side == Battle.TargetGroup.PARTY
        local throw_started = false
        local function start_throw()
            if throw_started then do return end end
            throw_started = true
            -- The thing leaves the hand halfway through the throwing animation (21), which comes after the hit mark.
            local delay = 0
            local thrower = self:get_entity(actor_side, action.actor)
            local anim = self:get_animation_name(actor_side, action.actor, 21)
            if thrower ~= nil and anim ~= nil then
                delay = thrower:get_animation_length(anim) / BattleAnimations.SPEED / 2
            end
            local ok = pcall(function()
                UiContainer.BattleFx:play_throw(
                  actor_side, action.actor, action.target_group, current.targets, action.coin_gil ~= nil and "common_zeni" or self:get_throw_mesh(action.throw_item), delay,
                  function() self:apply_action_hit() end,
                  function() current.pending = current.pending - 1 end
                )
            end)
            if not ok then
                self:apply_action_hit()
                current.pending = current.pending - 1
            end
        end
        current.pending = (summon or throw) and 2 or 1
        -- The aura of the caster.
        if action.attack_id ~= nil and actor_side == Battle.TargetGroup.PARTY then
            local ok, err = pcall(function()
                local kind = action.limit ~= nil and 1 or UiContainer.BattleFx:aura_kind(action, summon)
                UiContainer.BattleFx:start_aura(kind, actor_side, action.actor)
            end)
            if not ok then print("Battle aura: " .. tostring(err)) end
        end
        -- The camera of what is used (summons have their own).
        if not summon then self:start_action_camera(actor_side, action, attack, current.targets) end
        local played = self:play_script(
          actor_side, action.actor, script, false,
          function()
              if summon then start_summon() elseif throw then start_throw() else self:apply_action_hit() end
          end,
          function()
              current.pending = current.pending - 1
              -- A script without a hit mark still ends with the summon.
              if summon then start_summon() end
              if throw then start_throw() end
          end,
          current.targets[1] ~= nil and {side = action.target_group, index = current.targets[1]} or nil
        )
        if not played then
            if summon then
                -- Nothing to play for the attacker: the summon starts right away.
                current.pending = 1
                start_summon()
            elseif throw then
                current.pending = 1
                start_throw()
            else
                -- Nothing to play for the attacker: the hit lands right away.
                current.pending = 0
            end
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
        combatant.held = nil
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
        self:update_near_death(side, index)
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

    --- Applies the hit of the action being executed. Quadra Magic repeats it three more times.
    apply_action_hit = function(self)
        local current = self.current_action
        if current == nil or current.hit_done == true then do return end end
        current.hit_done = true
        local action = current.action
        local attack = self:apply_links(self:get_action_attack(action), action.links)
        if current.quadra then attack = self:apply_quadra(attack) end
        -- Steal and Sense do no damage.
        if current.side == Battle.TargetGroup.PARTY and action.attack_id == nil and action.item_id == nil and action.limit == nil
          and action.target_group == Battle.TargetGroup.ENEMIES and current.targets[1] ~= nil then
            if action.command == self.STEAL_COMMAND then
                self:try_steal(action.actor, current.targets[1], false)
                return
            elseif action.command == self.SENSE_COMMAND then
                self:sense(action.actor, current.targets[1])
                return
            elseif action.command == self.MANIP_COMMAND then
                self:manipulate(action.actor, current.targets[1])
                return
            end
        end
        -- The counterattacks of this hit go in front of the queue, in order.
        self.ai_counter_count = 0
        local hits = (action.limit ~= nil and current.side == Battle.TargetGroup.PARTY and attack.hits ~= nil) and attack.hits or 1
        if hits > 1 then
            -- A limit that hits several times (Omnislash...): the attack data tells if each hit goes to a random
            -- enemy, or all of them go to the targets.
            local random = (attack.target.random or 0) == 1 and action.target_group == Battle.TargetGroup.ENEMIES
            local number = 0
            -- Spread over the animation of the limit, if it is known.
            local delay = (current.limit_span or 0) > 0 and current.limit_span / hits or self.LIMIT_HIT_DELAY
            current.pending = current.pending + 1
            local function one()
                number = number + 1
                local target = nil
                if random then target = self:random_living(Battle.TargetGroup.ENEMIES) end
                if number > hits or (random and target == nil) then
                    current.pending = current.pending - 1
                    return
                end
                local saved = current.targets
                if random then current.targets = {target} end
                self.ai_counter_count = 0
                self:apply_hit_pass(current, action, attack, number)
                current.targets = saved
                table.insert(self.delayed_calls, {time = timer:get_game_time_total() + delay, call = one})
            end
            one()
        else
            self:apply_hit_pass(current, action, attack, 1)
        end
        -- Morph: an enemy that is defeated by it turns into its morph item.
        if current.side == Battle.TargetGroup.PARTY and action.command == self.MORPH_COMMAND and action.attack_id == nil
          and action.limit == nil and current.targets[1] ~= nil and action.target_group == Battle.TargetGroup.ENEMIES then
            local target = current.targets[1]
            local enemy = self.enemies[target]
            local source = battle_manager:get_enemy(target - 1)
            if enemy ~= nil and enemy.dead and source ~= nil and source:get_morph() >= 0 then
                -- The item goes to the inventory right away, and the enemy drops nothing else.
                enemy.morphed = true
                local item_id = source:get_morph()
                Inventory.add_item(item_id, 1)
                local item = Game.Items[item_id]
                self:show_message("Morphed into " .. (item ~= nil and item.name or "an item"))
            end
        end
        -- Mug steals while it attacks (even if the attack kills the enemy).
        if current.side == Battle.TargetGroup.PARTY and action.command == self.MUG_COMMAND and action.attack_id == nil
          and action.limit == nil and current.targets[1] ~= nil and action.target_group == Battle.TargetGroup.ENEMIES then
            self:try_steal(action.actor, current.targets[1], true, true)
        end
        -- Added Cut (Cloud attacks with his weapon after the spell) and Steal as well (a steal attempt) go
        -- after the spell, and after all the casts of Quadra Magic.
        local links = action.links
        local added_cut = links ~= nil and (links.added_cut or 0) > 0 and action.attack_id ~= nil and current.side == Battle.TargetGroup.PARTY
        local steal = links ~= nil and (links.steal_as_well or 0) > 0 and current.side == Battle.TargetGroup.PARTY
          and action.target_group == Battle.TargetGroup.ENEMIES
        -- Steal as well happens with the spell, even if it kills the enemy.
        if steal and current.targets[1] ~= nil then
            self:try_steal(action.actor, current.targets[1], true)
        end
        if added_cut then
            current.pending = current.pending + 1
            -- The first target that is still alive.
            local function first_target()
                for _, t in ipairs(current.targets) do
                    if self.enemies[t] ~= nil and not self.enemies[t].dead then return t end
                end
                return nil
            end
            local function finish() current.pending = current.pending - 1 end
            -- The attack of Added Cut waits for the animation of the spell to end, as a new animation of
            -- the character would cut it.
            local added_cut_attack
            added_cut_attack = function()
                local key = tostring(Battle.TargetGroup.PARTY) .. "_" .. tostring(action.actor)
                if self.animation_queues[key] ~= nil then
                    table.insert(self.delayed_calls, {time = timer:get_game_time_total() + 0.1, call = added_cut_attack})
                    return
                end
                local target = first_target()
                local hit_done = false
                local function hit()
                    if hit_done then return end
                    hit_done = true
                    self.ai_counter_count = 0
                    self:apply_hit_pass(current, action, self:get_weapon_attack(action.actor), 2)
                end
                local played = target ~= nil and self:play_script(
                  Battle.TargetGroup.PARTY, action.actor, self:get_script_number(Battle.TargetGroup.PARTY, action.actor, "attack"), false,
                  hit, function() hit() finish() end, {side = Battle.TargetGroup.ENEMIES, index = target}
                )
                if not played then
                    hit()
                    finish()
                end
            end
            table.insert(self.delayed_calls, {
                time = timer:get_game_time_total() + (current.quadra and 4 or 1) * self.QUADRA_DELAY,
                call = added_cut_attack,
            })
        end
        -- 2x-Cut and 4x-Cut hit again and again, with the animation of the attack each time.
        local strikes = (action.attack_id == nil and current.side == Battle.TargetGroup.PARTY) and self.COMMAND_STRIKES[action.command] or nil
        if strikes ~= nil and strikes > 1 then
            current.pending = current.pending + 1
            local list = self.enemies
            local last_target = current.targets[1]
            local function finish() current.pending = current.pending - 1 end
            local strike
            strike = function(number)
                if number > strikes then finish() return end
                local key = tostring(Battle.TargetGroup.PARTY) .. "_" .. tostring(action.actor)
                if self.animation_queues[key] ~= nil then
                    table.insert(self.delayed_calls, {time = timer:get_game_time_total() + 0.1, call = function() strike(number) end})
                    return
                end
                -- The target of each hit: a random enemy for 4x-Cut, the same one for 2x-Cut (or another if it
                -- died, unless it is a counterattack, which only hits the one that attacked).
                local target = nil
                if self.COMMAND_RANDOM[action.command] and not action.no_retarget then target = self:random_living(Battle.TargetGroup.ENEMIES)
                elseif last_target ~= nil and list[last_target] ~= nil and not list[last_target].dead then target = last_target
                elseif not action.no_retarget then target = self:random_living(Battle.TargetGroup.ENEMIES)
                end
                if target == nil or current.side ~= Battle.TargetGroup.PARTY or self.party[action.actor] == nil then finish() return end
                last_target = target
                local done = false
                local function hit()
                    if done then return end
                    done = true
                    local targets = current.targets
                    current.targets = {target}
                    self.ai_counter_count = 0
                    self:apply_hit_pass(current, action, self:get_weapon_attack(action.actor), 2)
                    current.targets = targets
                end
                local played = self:play_script(
                  Battle.TargetGroup.PARTY, action.actor, self:get_script_number(Battle.TargetGroup.PARTY, action.actor, "attack"), false,
                  hit, function() hit() strike(number + 1) end, {side = Battle.TargetGroup.ENEMIES, index = target}
                )
                if not played then
                    hit()
                    strike(number + 1)
                end
            end
            table.insert(self.delayed_calls, {time = timer:get_game_time_total() + 0.1, call = function() strike(2) end})
        end
        if current.quadra then
            for pass = 2, 4 do
                current.pending = current.pending + 1
                table.insert(self.delayed_calls, {
                    time = timer:get_game_time_total() + (pass - 1) * self.QUADRA_DELAY,
                    call = function()
                        self.ai_counter_count = 0
                        self:apply_hit_pass(current, action, attack, pass)
                        current.pending = current.pending - 1
                    end,
                })
            end
        end
    end,

    --- Seconds between the hits of the limits that hit several times.
    LIMIT_HIT_DELAY = 0.15,

    --- Seconds between the casts of Quadra Magic.
    QUADRA_DELAY = 0.45,

    --- Calls waiting for their time, {time = game time, call = function}.
    delayed_calls = {},

    --- Runs the delayed calls that are due.
    update_delayed_calls = function(self, now)
        for i = #self.delayed_calls, 1, -1 do
            local delayed = self.delayed_calls[i]
            if now >= delayed.time then
                table.remove(self.delayed_calls, i)
                local ok, err = pcall(delayed.call)
                if not ok then print("Delayed call failed: " .. tostring(err)) end
            end
        end
    end,

    --- Applies an attack to the targets of the action once.
    --
    -- @param pass 1 for the first time, more for the repetitions of Quadra Magic, which skip the
    -- targets that died (or go to the first one alive if all of them did).
    apply_hit_pass = function(self, current, action, attack, pass)
        local revives = self:attack_revives(attack)
        local targets = current.targets
        if pass > 1 and not revives then
            local list = action.target_group == Battle.TargetGroup.PARTY and self.party or self.enemies
            targets = {}
            for _, t in ipairs(current.targets) do
                if list[t] ~= nil and not list[t].dead then table.insert(targets, t) end
            end
            if #targets == 0 then
                for i = 1, #list do
                    if not list[i].dead then table.insert(targets, i) break end
                end
            end
        end
        for _, t0 in ipairs(targets) do
            local target_group = action.target_group
            local t = t0
            -- The ally that covers the target takes the attack (decided when the action started).
            if current.cover ~= nil and target_group == Battle.TargetGroup.PARTY and t == current.cover.covered
              and not self.party[current.cover.index].dead then
                UiContainer.BattleFx:text(Battle.TargetGroup.PARTY, current.cover.index, "Cover", {1, 1, 0.6})
                t = current.cover.index
            end
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
            local was_dead = target_group == Battle.TargetGroup.ENEMIES and self.enemies[t].dead
            local result = self:calculate_attack(attack, current.side, action.actor, target_group, t, #targets)
            -- What a physical hit cures is decided before the effects of the attack.
            local physical_cures = self:get_physical_cures(attack, target_group, t)
            if not result.hit then
                print("Miss")
                UiContainer.BattleFx:text(target_group, t, "Miss", UiContainer.BattleFx.COLOUR_MISS)
            else
                UiContainer.BattleFx:hit(attack, target_group, t, result)
                -- Status effects of the attack.
                self:apply_attack_effects(attack, target_group, t, #targets)
            end
            -- Physical hits wake up, cure Confusion and free manipulated enemies.
            if result.hit then self:apply_physical_cures(physical_cures, target_group, t) end
            if result.hit and result.amount ~= 0 then
                -- What the target has left limits what can be absorbed.
                local hp_before = self:get_combat_stats(target_group, t).hp
                self:apply_damage(target_group, t, result.amount, result.mp)
                local data = attack.damage
                if result.amount > 0 and not result.mp then
                    local absorbed = math.min(result.amount, hp_before)
                    if (data.drain_hp or 0) == 1 then self:apply_damage(current.side, action.actor, -absorbed, false) end
                    if (data.drain_mp or 0) == 1 then self:apply_damage(current.side, action.actor, -absorbed, true) end
                end
            end
            -- Elixirs restore MP too.
            if result.hit and (result.extra_mp or 0) ~= 0 then self:apply_damage(target_group, t, result.extra_mp, true) end
            local dead = (target_group == Battle.TargetGroup.PARTY and self.party[t].dead)
              or (target_group == Battle.TargetGroup.ENEMIES and self.enemies[t].dead)
            -- The character whose action defeats an enemy gets the kill, that counts for the limit levels.
            if dead and not was_dead and target_group == Battle.TargetGroup.ENEMIES and current.side == Battle.TargetGroup.PARTY
              and self.party[action.actor] ~= nil then
                local killer = Characters[self.party[action.actor].character]
                killer.kills = (killer.kills or 0) + 1
            end
            -- The limit bar fills with the damage that the enemies do.
            if target_group == Battle.TargetGroup.PARTY and current.side == Battle.TargetGroup.ENEMIES and result.hit
              and result.amount > 0 and not result.mp then
                self:add_limit(t, result.amount)
            end
            -- Party members with counter materia attack back, but a counterattack never triggers another one.
            if target_group == Battle.TargetGroup.PARTY and result.hit and result.amount > 0 and not result.mp and not dead
              and current.side == Battle.TargetGroup.ENEMIES and not action.is_counter then
                local formula = attack.damage.formula
                self:party_counter(t, current.side, action.actor, formula == 1 or formula == 11)
            end
            -- The AI of enemies can counterattack, but a counterattack never triggers another one.
            if target_group == Battle.TargetGroup.ENEMIES and result.hit and attack.id ~= nil and not action.is_counter then
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
                      function()
                          current.pending = current.pending - 1
                          self:restore_stance(target_group, t)
                      end
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
        if actor ~= nil and not action.is_counter and not action.no_atb then
            actor.atb = 0
            actor.waiting = false
            actor.acting = false
            if current.side == Battle.TargetGroup.PARTY then
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(action.actor) .. ".Right.AtbFill"):set_width(0, 0)
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(action.actor) .. ".Right.AtbFill"):set_colour(1, 1, 0)
            end
        end
        -- The turn of a manipulated enemy is the one of its controller.
        if action.controller ~= nil and self.party[action.controller] ~= nil then
            local controller = self.party[action.controller]
            controller.atb = 0
            controller.acting = false
            ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(action.controller) .. ".Right.AtbFill"):set_width(0, 0)
            ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(action.controller) .. ".Right.AtbFill"):set_colour(1, 1, 0)
        end
        -- The one that covered goes back to its place.
        if current.cover ~= nil and current.cover.home ~= nil then
            self:return_to_place(current.cover.index, current.cover.home)
        end
        -- A dead character that attacked with Final Attack goes back to lying on the floor.
        if action.final_attack and current.side == Battle.TargetGroup.PARTY then
            self:play_script(current.side, action.actor, self:get_script_number(current.side, action.actor, "death"), true, nil, nil)
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
        self:show_message(text)
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
        self:show_message(
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
        elseif list == Battle.Menu.E_SKILL then kind = "eskill"
        elseif list == Battle.Menu.ITEM then kind = "item"
        elseif list == Battle.Menu.LIMIT then kind = "limit"
        elseif list == Battle.Menu.THROW then kind = "throw"
        -- The double commands use the lists of magic, summons and items (twice).
        elseif list == Battle.Menu.W_MAGIC then kind = "magic"
        elseif list == Battle.Menu.W_SUMMON then kind = "summon"
        elseif list == Battle.Menu.W_ITEM then kind = "item"
        end
        if kind ~= nil then
            audio_manager:play_sound("Cursor")
            self.cursor_list = 1
            self.first_list = 1
            self.cursor_col = 1
            self.current_list = kind
            -- The window of the limits is pink.
            self:set_window_pink("BattleUi.Container.Bottom.List", kind == "limit")
            self.list_entries = self:build_list_entries(kind)
            -- The magic list is wider, with a row for each materia.
            local window = ui_manager:get_widget("BattleUi.Container.Bottom.List")
            if kind == "magic" then
                window:set_x(20, 0)
                window:set_width(0, self.MAGIC_LIST_WIDTH)
            else
                window:set_x(32, 0)
                window:set_width(0, self.LIST_WIDTH)
            end
            -- Start where the list was left, if the cursor memory is on.
            if UiContainer.cursor_memory() then
                local remembered = (self.memory_list[self.current_character_window] or {})[kind]
                if remembered ~= nil and remembered.first + remembered.cursor - 1 <= #self.list_entries then
                    self.first_list = remembered.first
                    self.cursor_list = remembered.cursor
                    self.cursor_col = remembered.col or 1
                end
                local row = self.list_entries[self.first_list + self.cursor_list - 1]
                if row ~= nil and self.cursor_col > #row.entries then self.cursor_col = #row.entries end
            end
            self:populate_list()
            self.window_state = "list"
            ui_manager:get_widget("BattleUi.Container.Bottom.List"):set_visible(true)
            self:update_list_cursor()
            cursor:set_visible(true)
        elseif (list == Battle.Menu.COIN) then
            print("Show coin list: Not implemented")
        end
    end,

    --- Remembers the position of the cursor in the current list, for the cursor memory.
    remember_list = function(self)
        local slot = self.current_character_window
        if self.current_list == "none" or slot == nil then return end
        self.memory_list[slot] = self.memory_list[slot] or {}
        self.memory_list[slot][self.current_list] = { first = self.first_list, cursor = self.cursor_list, col = self.cursor_col }
    end,

    --- Closes the currently opened list
    close_list = function(self)
        self:remember_list()
        self.current_list = "none"
        self.window_state = "command"
        ui_manager:get_widget("BattleUi.Container.Bottom.List"):set_visible(false)
        -- Cancelling the second choice of a double command goes back to the first one.
        if self.w_first ~= nil then
            local menu = self.W_COMMANDS[self.selected_command]
            self.w_first = nil
            self:show_message("")
            if menu ~= nil then self:show_list(menu) end
        end
    end,

    --- Shows the target selection window.
    --
    -- The elements displayed in the window and the navegability will depend on what is set in
    -- {@see target_options}.
    select_target = function(self)
        local entry = nil
        if self.window_state == "list" then entry = self:get_list_entry() end
        if entry == nil then do return end end
        self:remember_list()
        -- Spells, summons and items that can't be used (not enough MP, no summons left...).
        if not entry.enabled then
            audio_manager:play_sound("Error")
            do return end
        end
        local action = nil
        if entry.item_id ~= nil then action = Game.Items[entry.item_id]
        elseif entry.throw_item ~= nil then action = Game.Attacks[self.THROW_ATTACK]
        else action = Game.Attacks[entry.attack_id]
        end
        self.selected_throw = entry.throw_item
        self.selected_attack = entry.attack_id
        self.selected_item = entry.item_id
        self.selected_summon = entry.summon
        self.selected_limit = entry.limit
        self.selected_links = entry.links
        -- The special limits always affect a whole side, so there is nothing to choose.
        local special = entry.limit ~= nil and self:get_special_limit(entry.attack_id) or nil
        if special ~= nil then
            self.target_group = special.target == "enemies" and Battle.TargetGroup.ENEMIES or Battle.TargetGroup.PARTY
            if special.self_only then
                self.target_type = Battle.TargetType.SINGLE
                self.target_index = self.current_character_window
            else
                self.target_type = Battle.TargetType.GROUP
                self.target_index = nil
            end
            self:queue_character_action()
            return
        end
        -- Some attacks (the limits that are not done) have no target data: one enemy.
        local target_data = action.target or {
            selectable = 1, default_enemy = 1, default_multiple = 0, toggle_multiple = 0, fixed_row = 0, short_range = 0,
            all_rows = 0, random = 0,
        }
        if entry.throw_item ~= nil then
            target_data = {
                selectable = 1, default_enemy = 1, default_multiple = 0, toggle_multiple = 0, fixed_row = 0, short_range = 0,
                all_rows = 0, random = 0,
            }
        end
        self.target_options = target_data
        -- In the kernel data, spells that All materia can turn into multiple target ones have both
        -- "default multiple" and "toggle multiple" set, and the game clears both if the spell has no All
        -- linked (items and the other attacks don't have them). With All (and uses left) the spell starts
        -- with one target and can toggle to all of them, a number of times per battle.
        if entry.item_id == nil and target_data.toggle_multiple == 1 then
            local member = self.party[self.current_character_window]
            local all = entry.links ~= nil and (entry.links.all or 0) > 0
              and (member.all_used[entry.attack_id] or 0) < entry.links.all
            local options = {}
            for key, value in pairs(target_data) do options[key] = value end
            options.default_multiple = 0
            options.toggle_multiple = all and 1 or 0
            self.target_options = options
        end
        -- Mega All lets any action that aims at one target hit all of them, a number of times per battle.
        if self:mega_all_left(self.party[self.current_character_window]) > 0 and self.target_options.default_multiple ~= 1
          and self.target_options.toggle_multiple ~= 1 then
            local options = {}
            for key, value in pairs(self.target_options) do options[key] = value end
            options.toggle_multiple = 1
            self.target_options = options
        end
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
            local frog = self:has_status(Battle.TargetGroup.PARTY, self.current_character_window, BattleStatus.FROG)
            for _, spell in ipairs(member.magic) do
                local data = Game.Attacks[spell.id]
                table.insert(entries, {
                    name = data.name, attack_id = spell.id, links = spell, group = spell.group,
                    enabled = self:get_spell_cost(data.mp, spell) <= stats.mp.current - self:get_w_reserved_mp()
                      and (not frog or spell.id == self.FROG_SPELL),
                })
            end
        elseif kind == "eskill" then
            -- The enemy skills learned with the equipped materia.
            local learned = Characters.get_abilities(member.character).e_skill
            for _, id in ipairs(learned) do
                local data = Game.Attacks[id]
                if data ~= nil then
                    table.insert(entries, {name = data.name, attack_id = id, enabled = data.mp <= stats.mp.current})
                end
            end
        elseif kind == "summon" then
            for _, summon in ipairs(member.summon) do
                local data = Game.Attacks[summon.id]
                -- Summons can be used a number of times per battle. 255 times is no limit.
                local used = member.summon_used[summon.id] or 0
                local left = (summon.times or 1) - used
                table.insert(entries, {
                    name = data.name, attack_id = summon.id, summon = summon, links = summon,
                    enabled = self:get_spell_cost(data.mp, summon) <= stats.mp.current - self:get_w_reserved_mp()
                      and (left - ((self.w_first ~= nil and self.w_first.attack_id == summon.id) and 1 or 0) > 0 or (summon.times or 0) >= 255),
                })
            end
        elseif kind == "limit" then
            -- The limits learned in the limit level of the character.
            local char = Characters[member.character]
            local level = char.limit.current
            local definition = Game.Characters[member.character]
            local techniques = definition ~= nil and definition.limits[level] ~= nil and definition.limits[level].techniques or {}
            for k, technique in ipairs(techniques) do
                local attack = Game.Attacks[technique.command]
                if attack ~= nil and char.limit.learned[level] ~= nil and char.limit.learned[level][k] == true then
                    -- Some names have bytes that are not letters at the start.
                    local name = string.gsub(attack.name, "^[^%w]+", "")
                    table.insert(entries, {name = name, attack_id = technique.command, enabled = true, limit = {level = level, index = k}})
                end
            end
        elseif kind == "throw" then
            -- Everything that can be thrown: items, weapons, armor and accessories.
            for slot = 0, MAX_INVENTORY_SLOTS do
                local entry = Inventory[slot]
                if entry ~= nil and entry.quantity > 0 then
                    local item = Game.Items[entry.item]
                    if item ~= nil and item.throw == 1 then
                        table.insert(entries, {name = item.name .. " " .. tostring(entry.quantity), throw_item = entry.item, enabled = true})
                    end
                end
            end
        elseif kind == "item" then
            for slot = 0, MAX_INVENTORY_SLOTS do
                local entry = Inventory[slot]
                if entry ~= nil and entry.quantity > 0 then
                    local item = Game.Items[entry.item]
                    -- Only the items that can be used in battle (weapons and armor can't).
                    if item ~= nil and item.battle == 1 then
                        -- The item of the first choice of a double command can't be used again if there is no more.
                        local left = entry.quantity - ((self.w_first ~= nil and self.w_first.item_id == entry.item) and 1 or 0)
                        table.insert(entries, {name = item.name .. " " .. tostring(entry.quantity), item_id = entry.item, enabled = left > 0})
                    end
                end
            end
        end
        return self:build_list_rows(kind, entries)
    end,

    --- Widths of the list window: the magic list has several spells in each row.
    LIST_WIDTH = 120,
    MAGIC_LIST_WIDTH = 302,

    --- Longest name shown in the magic list (the columns fit 10 characters).
    MAGIC_NAME_LENGTH = 10,

    --- Colour of the modifiers of the magic list.
    LIST_TAG_COLOUR = {0.8, 0.5, 1},

    --- Gets the entry under the cursor of the list window.
    get_list_entry = function(self)
        local row = self.list_entries[self.cursor_list + self.first_list - 1]
        if row == nil then return nil end
        return row.entries[math.min(self.cursor_col, #row.entries)]
    end,

    --- Moves the cursor of the list window to its row and column.
    update_list_cursor = function(self)
        local row = self.list_entries[self.cursor_list + self.first_list - 1]
        if row ~= nil and self.cursor_col > #row.entries then self.cursor_col = #row.entries end
        local name = "Position" .. tostring(self.cursor_list)
        if self.current_list == "magic" then name = name .. "_" .. tostring(self.cursor_col) end
        ui_manager:get_widget("BattleUi.Container.Bottom.List.Cursor"):set_default_animation(name)
    end,

    --- Gets the modifiers of the materia linked to a spell, as the three letter strings of the list.
    get_spell_tags = function(self, links)
        local tags = {}
        if links == nil then return tags end
        if (links.all or 0) > 0 then table.insert(tags, "ALL") end
        if (links.mp_turbo or 0) > 0 then table.insert(tags, "MPT") end
        if (links.hp_absorb or 0) > 0 then table.insert(tags, "HPA") end
        if (links.mp_absorb or 0) > 0 then table.insert(tags, "MPA") end
        if (links.quadra or 0) > 0 then table.insert(tags, " x4") end
        return tags
    end,

    --- Groups the entries of a list in rows. The spells of the same materia go in the same row, four
    -- at most (a materia with more, like Master Magic, continues in the next row), with the first modifier of the materia.
    -- The rows of the other lists have one entry.
    --
    -- @return Rows: {entries = {...}, tag = text of the modifiers}.
    build_list_rows = function(self, kind, entries)
        local rows = {}
        for _, entry in ipairs(entries) do
            local row = rows[#rows]
            if kind == "magic" and row ~= nil and row.group == entry.group and #row.entries < 4 then
                table.insert(row.entries, entry)
            else
                row = {entries = {entry}, group = entry.group}
                -- The first column only fits one modifier.
                row.tag = self:get_spell_tags(entry.links)[1] or ""
                table.insert(rows, row)
            end
        end
        return rows
    end,

    --- Populates the list window.
    --
    -- Uses the rows in {@see list_entries}, starting at {@see first_list}.
    populate_list = function(self)
        local magic = self.current_list == "magic"
        local columns = {"A", "B", "C", "D"}
        for pos = 1, 4 do
            local single = ui_manager:get_widget("BattleUi.Container.Bottom.List.List" .. tostring(pos))
            local tag = ui_manager:get_widget("BattleUi.Container.Bottom.List.ListTag" .. tostring(pos))
            local row = self.list_entries[pos + self.first_list - 1]
            -- The magic list uses the tag and spell widgets, the other ones the single widget.
            single:set_visible(false)
            tag:set_visible(false)
            for _, letter in ipairs(columns) do
                ui_manager:get_widget("BattleUi.Container.Bottom.List.List" .. letter .. tostring(pos)):set_visible(false)
            end
            if row ~= nil then
                -- Entries that can't be used are greyed out. The colour has to be set before the
                -- text, as it's applied to it when it's set.
                if magic then
                    if row.tag ~= "" then
                        tag:set_colour(self.LIST_TAG_COLOUR[1], self.LIST_TAG_COLOUR[2], self.LIST_TAG_COLOUR[3])
                        tag:set_text(row.tag)
                        tag:set_visible(true)
                    end
                    for index, entry in ipairs(row.entries) do
                        local widget = ui_manager:get_widget("BattleUi.Container.Bottom.List.List" .. columns[index] .. tostring(pos))
                        if entry.enabled then widget:set_colour(1, 1, 1) else widget:set_colour(0.5, 0.5, 0.5) end
                        widget:set_text(string.sub(entry.name, 1, self.MAGIC_NAME_LENGTH))
                        widget:set_visible(true)
                    end
                else
                    local entry = row.entries[1]
                    if entry.enabled then single:set_colour(1, 1, 1) else single:set_colour(0.5, 0.5, 0.5) end
                    single:set_text(entry.name)
                    single:set_visible(true)
                end
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
            -- Debug: Battle.debug_enemy_hp (set by the test menu) gives all the enemies that HP.
            if Battle.debug_enemy_hp ~= nil then
                enemy.stats.hp = Battle.debug_enemy_hp
                enemy.stats.hp_max = Battle.debug_enemy_hp
            end
            -- TODO: Get more data
            table.insert(self.enemies, enemy)
        end
    end,

    --- Bone of the skeleton that holds the weapon, by the two letter prefix of the battle model.
    --
    -- Derived from the skeleton of each model: the arm chains hang from the spine, the chains from the
    -- root are the legs. The hand of the second arm chain was chosen (the right one is a guess for all
    -- but Barret, whose gun arm is the right one). If a weapon is in the wrong hand, use the hand of the
    -- first arm chain instead: rt 7, ru 15, rv 12, rx 10, ry 19, rz 8, sb-se 7, sf-sh 41.
    WEAPON_BONES = {
        rt = 12, si = 12, ru = 22, rv = 17, rx = 18, ry = 24, rz = 16,
        sb = 11, sc = 11, sd = 11, se = 11, sf = 46, sg = 46, sh = 46,
    },

    --- Extra rotation {x, y, z} in degrees of the weapon of each battle model, around the axes of the
    -- weapon model itself, by the two letter prefix. Tune it here (no reinstall needed) until the weapons
    -- look right; models not listed use no rotation.
    WEAPON_ROTATIONS = {
        rt = {90, 0, 0},
    },

    --- Battle model of the owner of each character of the kernel, to find the model of a thrown weapon.
    THROW_MODELS = {
        [0] = "rt_cloud", [1] = "sb_barret", [2] = "ru_tifa", [3] = "rv_aeris", [5] = "rx_yuffie",
        [6] = "ry_cait_sith", [7] = "sf_vincent", [8] = "rz_cid",
    },

    --- Gets the mesh of a thrown thing: the model of the weapon, of the character that can use it.
    --
    -- @return Name of the mesh without extension, nil if it isn't a weapon with a model.
    get_throw_mesh = function(self, item_id)
        local item = Game.Items[item_id]
        if item == nil or item.type ~= Inventory.ITEM_TYPE.WEAPON or item.model == nil then return nil end
        for _, user in ipairs(item.users or {}) do
            local model = self.THROW_MODELS[user]
            if model ~= nil then return string.format("characters/%s_weapon_%02d", model, item.model) end
        end
        return nil
    end,

    --- Puts the weapon a party member has equipped in the hand of its battle model.
    --
    -- Weapon models are installed as <model>_weapon_NN.mesh, where NN is the model number of the weapon
    -- (see the `model` field of the weapons). Red XIII has none.
    --
    -- @param index Index in the party (1 based).
    attach_weapon = function(self, index)
        local entity = self:get_entity(Battle.TargetGroup.PARTY, index)
        local model = self:get_model_name(Battle.TargetGroup.PARTY, index)
        if entity == nil or model == nil or model == "" then return end
        local bone = self.WEAPON_BONES[string.sub(model, 1, 2)]
        if bone == nil then return end
        local character = Characters[self.party[index].character]
        local weapon = (character ~= nil and character.weapon ~= nil) and Game.Items[character.weapon.id] or nil
        if weapon == nil or weapon.model == nil then return end
        local rot = self.WEAPON_ROTATIONS[string.sub(model, 1, 2)] or {0, 0, 0}
        entity:attach_model(
            string.format("characters/%s_weapon_%02d.mesh", model, weapon.model), bone, rot[1], rot[2], rot[3]
        )
    end,

    --- Loads party character data
    --
    --- Gets the effects of the support materia linked to the materia of a slot.
    --
    -- @param equip The weapon or armor of the character ({@see Characters}).
    -- @param slot Slot of the materia (1-8).
    -- @return A table: all (times it can be used on all the targets, per battle), mp_turbo (percent
    -- of extra power), hp_absorb, mp_absorb, quadra (times per battle), added_cut and steal_as_well (0 or 1).
    get_materia_links = function(self, equip, slot)
        local links = {all = 0, mp_turbo = 0, hp_absorb = 0, mp_absorb = 0, quadra = 0, added_cut = 0, steal_as_well = 0}
        local item = Game.Items[equip.id]
        if item == nil or item.slots == nil then return links end
        for _, partner_slot in ipairs(Characters.get_linked_slots(item, slot)) do
            local partner = equip.materia[partner_slot - 1]
            local definition = partner ~= nil and Game.Materia[partner.id] or nil
            if definition ~= nil and definition.link ~= nil then
                local id = definition.link.id
                local level = Characters.get_materia_level(definition, partner.ap or 0)
                local param = definition.link.param[level] or definition.link.param[1] or 0
                if id == Materia.LINK.ALL then links.all = math.max(links.all, param)
                elseif id == Materia.LINK.MP_TURBO then links.mp_turbo = math.max(links.mp_turbo, param)
                elseif id == Materia.LINK.HP_ABSORB then links.hp_absorb = 1
                elseif id == Materia.LINK.MP_ABSORB then links.mp_absorb = 1
                elseif id == Materia.LINK.QUADRA_MAGIC then links.quadra = math.max(links.quadra, param)
                elseif id == Materia.LINK.ADDED_CUT then links.added_cut = 1
                elseif id == Materia.LINK.STEAL_AS_WELL then links.steal_as_well = 1
                end
            end
        end
        return links
    end,

    --- Gets the counterattacks of a character: Counter materia paired with a command materia, and
    -- Magic Counter materia paired with a magic materia.
    --
    -- Final Attack materia are in the list too, with final = true and the times they can be used per
    -- battle in uses, and Sneak Attack ones with sneak = true.
    --
    -- @return A list of {chance = 0 to 1, command = command ID} or {chance, spells = {attack IDs, from the
    -- lowest level to the highest}}.
    get_counters = function(self, char)
        local counters = {}
        for _, equip in ipairs({char.weapon, char.armor}) do
            local item = Game.Items[equip.id]
            if item ~= nil and item.slots ~= nil then
                for slot = 1, #item.slots do
                    local equipped = equip.materia[slot - 1]
                    local definition = equipped ~= nil and Game.Materia[equipped.id] or nil
                    local link = definition ~= nil and definition.link or nil
                    if link ~= nil and (link.id == Materia.LINK.COMMAND_COUNTER or link.id == Materia.LINK.MAGIC_COUNTER
                      or link.id == Materia.LINK.FINAL_ATTACK or link.id == Materia.LINK.SNEAK_ATTACK) then
                        local level = Characters.get_materia_level(definition, equipped.ap or 0)
                        local param = link.param[level] or link.param[1] or 0
                        local chance = param / 100
                        -- Final Attack: the level is the times it can be used per battle.
                        local final = link.id == Materia.LINK.FINAL_ATTACK
                        -- Sneak Attack: the level is the chance of using the paired materia at the start.
                        local sneak = link.id == Materia.LINK.SNEAK_ATTACK
                        for _, partner_slot in ipairs(Characters.get_linked_slots(item, slot)) do
                            local partner = equip.materia[partner_slot - 1]
                            local partner_def = partner ~= nil and Game.Materia[partner.id] or nil
                            if partner_def ~= nil then
                                local partner_level = Characters.get_materia_level(partner_def, partner.ap or 0)
                                if (link.id == Materia.LINK.COMMAND_COUNTER or final or sneak) and partner_def.type == 3 then
                                    -- The highest command of the materia.
                                    local command = nil
                                    for lv, id in ipairs(partner_def.command or {}) do
                                        if partner_level >= lv then command = id end
                                    end
                                    if command ~= nil then table.insert(counters, {chance = chance, command = command, final = final, sneak = sneak, uses = param}) end
                                elseif (link.id == Materia.LINK.MAGIC_COUNTER or final or sneak) and partner_def.type == 1 then
                                    local spells = {}
                                    for lv, id in ipairs(partner_def.magic or {}) do
                                        if partner_level >= lv then table.insert(spells, id) end
                                    end
                                    if #spells > 0 then table.insert(counters, {chance = chance, spells = spells, final = final, sneak = sneak, uses = param}) end
                                end
                            end
                        end
                    end
                end
            end
        end
        return counters
    end,

    --- Queues the counterattacks of a party member that was hurt by an enemy.
    --
    -- Counter materia attack back with the command they are paired with (the commands that change the
    -- attack, like Slash-All, aren't done and count as a normal attack), and Magic Counter materia cast
    -- the highest spell of the paired magic materia (nothing if the MP doesn't allow it). Counter Attack
    -- materia only counter physical attacks. They only
    -- counter the enemy that attacked, and are cancelled if it died. They go in front of the queue.
    -- TODO: Summons (Magic Counter with a summon materia) aren't done.
    party_counter = function(self, index, attacker_side, attacker_index, physical)
        local member = self.party[index]
        if member == nil or member.dead or self:is_frozen(Battle.TargetGroup.PARTY, index) or self:acts_by_itself(Battle.TargetGroup.PARTY, index) then
            do return end
        end
        -- Only the enemy that attacked is countered. If it's gone, there is no counterattack.
        local enemy = attacker_index
        if self.enemies[enemy] == nil or self.enemies[enemy].dead then do return end end
        for _, counter in ipairs(member.counters or {}) do
            if not counter.final and not counter.sneak and (not counter.physical or physical) and math.random() < counter.chance then
                local action = {
                    actor = index, actor_side = Battle.TargetGroup.PARTY, is_counter = true, no_retarget = true,
                    target_type = Battle.TargetType.SINGLE, target_group = Battle.TargetGroup.ENEMIES, target_index = enemy,
                }
                if counter.command ~= nil then
                    action.type = Battle.ActionType.COMMAND
                    -- The commands that change the attack are done, the other ones count as an attack.
                    action.command = (counter.command >= 24 and counter.command <= 27) and counter.command or 1
                    -- Slash-All counters hit all the enemies, the other ones only the attacker.
                    if counter.command == 24 or counter.command == 26 then
                        action.target_type = Battle.TargetType.GROUP
                        action.target_index = nil
                        action.no_retarget = nil
                    end
                else
                    -- Always the highest spell of the materia. Without the MP for it, there is no counterattack.
                    local mp = Characters[member.character].stats.mp.current
                    local spell = counter.spells[#counter.spells]
                    local cost = Game.Attacks[spell] ~= nil and Game.Attacks[spell].mp or nil
                    if cost ~= nil and cost > mp then
                        print("Magic Counter of " .. self:get_combatant_name(Battle.TargetGroup.PARTY, index) .. ": no MP for " .. Game.Attacks[spell].name)
                        UiContainer.BattleFx:text(Battle.TargetGroup.PARTY, index, "No MP", {1, 0.6, 0.6})
                        spell = nil
                    end
                    if spell ~= nil then
                        local data = Game.Attacks[spell]
                        action.type = Battle.ActionType.ATTACK
                        action.command = 2
                        action.attack_id = spell
                        if data.target.default_enemy == 0 then
                            -- Restoring spells go to the one that casts them.
                            action.target_group = Battle.TargetGroup.PARTY
                            action.target_index = index
                        elseif data.target.default_multiple == 1 and data.target.toggle_multiple ~= 1 then
                            action.target_type = Battle.TargetType.GROUP
                            action.target_index = nil
                        end
                    else
                        action = nil
                    end
                end
                if action ~= nil then
                    UiContainer.BattleFx:text(Battle.TargetGroup.PARTY, index, "Counter", {1, 1, 0.6})
                    local position = math.min((self.ai_counter_count or 0) + 1, #self.action_queue + 1)
                    table.insert(self.action_queue, position, action)
                    self.ai_counter_count = position
                end
            end
        end
    end,

    --- Queues the Sneak Attacks at the start of the battle: each Sneak Attack materia has a chance (its
    -- level) of making the character use the paired command (an Attack replacement) or the highest spell
    -- of the paired magic materia before anyone else acts, on a random enemy. They don't use the turn of
    -- the character. Nothing happens if the MP doesn't allow the spell.
    -- TODO: Summons aren't done, and it is unknown if the original also does it in back attacks.
    queue_sneak_attacks = function(self)
        local position = 0
        for index = 1, #self.party do
            local member = self.party[index]
            for _, counter in ipairs(member.counters or {}) do
                if counter.sneak and math.random() < counter.chance then
                    local target = self:random_living(Battle.TargetGroup.ENEMIES)
                    if target == nil then do return end end
                    local action = {
                        actor = index, actor_side = Battle.TargetGroup.PARTY, no_atb = true, sneak_attack = true,
                        target_type = Battle.TargetType.SINGLE, target_group = Battle.TargetGroup.ENEMIES, target_index = target,
                    }
                    if counter.command ~= nil then
                        action.type = Battle.ActionType.COMMAND
                        action.command = (counter.command >= 24 and counter.command <= 27) and counter.command or 1
                        if counter.command == 24 or counter.command == 26 then
                            action.target_type = Battle.TargetType.GROUP
                            action.target_index = nil
                        end
                    else
                        local mp = Characters[member.character].stats.mp.current
                        local spell = counter.spells[#counter.spells]
                        local data = Game.Attacks[spell]
                        if data == nil or data.mp > mp then
                            if data ~= nil then
                                print("Sneak Attack of " .. self:get_combatant_name(Battle.TargetGroup.PARTY, index) .. ": no MP for " .. data.name)
                            end
                            action = nil
                        else
                            action.type = Battle.ActionType.ATTACK
                            action.command = 2
                            action.attack_id = spell
                            if data.target.default_enemy == 0 then
                                -- Restoring spells go to a random ally.
                                action.target_group = Battle.TargetGroup.PARTY
                                action.target_index = self:random_living(Battle.TargetGroup.PARTY)
                            elseif data.target.default_multiple == 1 and data.target.toggle_multiple ~= 1 then
                                action.target_type = Battle.TargetType.GROUP
                                action.target_index = nil
                            end
                        end
                    end
                    if action ~= nil then
                        position = position + 1
                        table.insert(self.action_queue, math.min(position, #self.action_queue + 1), action)
                        UiContainer.BattleFx:text(Battle.TargetGroup.PARTY, index, "Sneak Attack", {1, 1, 0.6})
                    end
                end
            end
        end
    end,

    --- Queues the Final Attack of a party member that just died: it uses the paired command (a normal
    -- attack) or the highest spell of the paired magic materia once, on a random enemy, a number of times
    -- per battle. It does nothing if the MP doesn't allow the spell.
    -- TODO: Summons aren't done.
    queue_final_attack = function(self, index)
        local member = self.party[index]
        if member == nil or self.ending == true then do return end end
        member.final_used = member.final_used or {}
        for key, counter in ipairs(member.counters or {}) do
            if counter.final and (member.final_used[key] or 0) < counter.uses then
                local target = self:random_living(Battle.TargetGroup.ENEMIES)
                if target == nil then do return end end
                local action = {
                    actor = index, actor_side = Battle.TargetGroup.PARTY, is_counter = true, allow_dead = true, final_attack = true,
                    target_type = Battle.TargetType.SINGLE, target_group = Battle.TargetGroup.ENEMIES, target_index = target,
                }
                if counter.command ~= nil then
                    action.type = Battle.ActionType.COMMAND
                    action.command = (counter.command >= 24 and counter.command <= 27) and counter.command or 1
                    if counter.command == 24 or counter.command == 26 then
                        action.target_type = Battle.TargetType.GROUP
                        action.target_index = nil
                    end
                else
                    local mp = Characters[member.character].stats.mp.current
                    local spell = counter.spells[#counter.spells]
                    local data = Game.Attacks[spell]
                    if data ~= nil and data.mp > mp then
                        print("Final Attack of " .. self:get_combatant_name(Battle.TargetGroup.PARTY, index) .. ": no MP for " .. data.name)
                        UiContainer.BattleFx:text(Battle.TargetGroup.PARTY, index, "No MP", {1, 0.6, 0.6})
                        action = nil
                    elseif data ~= nil then
                        action.type = Battle.ActionType.ATTACK
                        action.command = 2
                        action.attack_id = spell
                        if data.target.default_enemy == 0 then
                            -- Restoring spells go to a random ally that is alive.
                            local ally = self:random_living(Battle.TargetGroup.PARTY)
                            if ally == nil then action = nil
                            else
                                action.target_group = Battle.TargetGroup.PARTY
                                action.target_index = ally
                            end
                        elseif data.target.default_multiple == 1 and data.target.toggle_multiple ~= 1 then
                            action.target_type = Battle.TargetType.GROUP
                            action.target_index = nil
                        end
                    else
                        action = nil
                    end
                end
                if action ~= nil then
                    member.final_used[key] = (member.final_used[key] or 0) + 1
                    UiContainer.BattleFx:text(Battle.TargetGroup.PARTY, index, "Final Attack", {1, 1, 0.6})
                    local position = math.min((self.ai_counter_count or 0) + 1, #self.action_queue + 1)
                    table.insert(self.action_queue, position, action)
                    self.ai_counter_count = position
                end
            end
        end
    end,

    --- Maximum gil that can be thrown with Coin: all the gil of the party, in tens.
    coin_limit = function(self) return math.floor((Inventory.money or 0) / 10) * 10 end,

    --- Opens the gil selection of Coin, in the message window.
    open_coin_window = function(self)
        if self:coin_limit() < 10 then
            audio_manager:play_sound("Error")
            do return end
        end
        audio_manager:play_sound("Cursor")
        self.coin = {amount = math.min(self:coin_limit(), 100), step = 10}
        self.window_state = "coin"
        self:show_coin_amount()
    end,

    --- Shows the gil chosen for Coin in its window.
    show_coin_amount = function(self)
        local base = "BattleUi.Container.Bottom.Coin."
        ui_manager:get_widget(base .. "Title"):set_text(Game.Commands[self.COIN_COMMAND].name)
        ui_manager:get_widget(base .. "Amount"):set_text(string.format("%d Gil", self.coin.amount))
        ui_manager:get_widget(base .. "Step"):set_text(string.format("+-%d", self.coin.step))
        ui_manager:get_widget("BattleUi.Container.Bottom.Coin"):set_visible(true)
    end,

    --- Closes the window of Coin.
    close_coin_window = function(self)
        ui_manager:get_widget("BattleUi.Container.Bottom.Coin"):set_visible(false)
    end,

    --- Handles a button in the gil selection of Coin: up and down change the amount by the step, left and right
    -- the step (10 to 100000). Confirm throws the gil at all the enemies.
    handle_coin_button = function(self, button)
        local coin = self.coin
        local limit = self:coin_limit()
        if button == Config.ACTIONS.CANCEL then
            audio_manager:play_sound("Cursor")
            self:close_coin_window()
            self.window_state = "command"
        elseif button == Config.ACTIONS.CONFIRM then
            audio_manager:play_sound("Cursor")
            self:close_coin_window()
            self.selected_command = self.COIN_COMMAND
            self.current_list = "none"
            self.selected_attack = nil
            self.selected_item = nil
            self.selected_summon = nil
            self.selected_limit = nil
            self.selected_links = nil
            self.selected_throw = nil
            self.selected_coin = math.min(coin.amount, limit)
            self.target_group = Battle.TargetGroup.ENEMIES
            self.target_type = Battle.TargetType.GROUP
            self.target_index = nil
            self:queue_character_action()
            return
        elseif button == Config.CONTROLS.UP then coin.amount = math.min(limit, coin.amount + coin.step)
        elseif button == Config.CONTROLS.DOWN then coin.amount = math.max(10, coin.amount - coin.step)
        elseif button == Config.CONTROLS.RIGHT then coin.step = math.min(100000, coin.step * 10)
        elseif button == Config.CONTROLS.LEFT then coin.step = math.max(10, coin.step / 10)
        else return end
        audio_manager:play_sound("Cursor")
        self:show_coin_amount()
    end,

    COIN_COMMAND = 7,

    --- IDs of the commands in Game.Commands.
    STEAL_COMMAND = 5,
    SENSE_COMMAND = 6,
    MUG_COMMAND = 17,
    MORPH_COMMAND = 9,
    --- Commands that the kernel marks as having no menu, but choose an enemy (Sense, Morph and D.blow).
    ENEMY_COMMANDS = {[6] = true, [9] = true, [10] = true, [11] = true},
    MANIP_COMMAND = 11,
    MIME_COMMAND = 12,
    DBLOW_COMMAND = 10,

    --- Chance of the statuses that Added Effect materia gives to the attacks, out of 1 (5 of 63).
    ADDED_EFFECT_CHANCE = 5 / 63,

    --- Names of the elements, by element id.
    ELEMENT_NAMES = {[0] = "Fire", "Ice", "Bolt", "Earth", "Poison", "Gravity", "Water", "Wind", "Holy",
      "Restorative", "Cut", "Hit", "Punch", "Shoot", "Shout"},

    --- Shows the information of an enemy (Sense): name, level, HP, MP, how it reacts to the elements
    -- (weak, half, nullified or absorbed) and the statuses it has now.
    sense = function(self, actor, target)
        local enemy = self.enemies[target]
        if enemy == nil then do return end end
        local source = battle_manager:get_enemy(target - 1)
        local text = string.format("%s Lv%d HP %d/%d MP %d/%d", enemy.name, source:get_level(), enemy.stats.hp,
          enemy.stats.hp_max, enemy.stats.mp, enemy.stats.mp_max)
        local groups = {weak = {}, half = {}, none = {}, absorb = {}}
        for e = 0, source:get_element_count() - 1 do
            local name = self.ELEMENT_NAMES[source:get_element(e)]
            local factor = source:get_element_factor(e)
            if name ~= nil then
                if factor < 0 then table.insert(groups.absorb, name)
                elseif factor == 0 then table.insert(groups.none, name)
                elseif factor < 1 then table.insert(groups.half, name)
                elseif factor > 1 then table.insert(groups.weak, name) end
            end
        end
        for _, entry in ipairs({{"weak", "Weak"}, {"half", "Resists"}, {"none", "Immune"}, {"absorb", "Absorbs"}}) do
            if #groups[entry[1]] > 0 then text = text .. " | " .. entry[2] .. ": " .. table.concat(groups[entry[1]], ", ") end
        end
        local statuses = {}
        for id = 0, 26 do
            if self:has_status(Battle.TargetGroup.ENEMIES, target, id) then table.insert(statuses, BattleStatus.NAME[id]) end
        end
        if #statuses > 0 then text = text .. " | Status: " .. table.concat(statuses, ", ") end
        self:show_message(text, false)
        self.message_until = timer:get_game_time_total() + 4
    end,

    --- Tries to steal an item from an enemy.
    --
    -- As in ff7.exe (0x5ca7d0 and the item roll 0x5c7c1e): each steal item (in order) has a rate of 0..63 that is
    -- scaled by (thief level + 40 - enemy level) percent (Mug never goes below 100%); the item is the first
    -- one with a roll of 0..63 that is less than or equal to the scaled rate. Only one item can be stolen from
    -- each enemy. With allow_dead, it can be stolen from an enemy that was just killed (Steal as well and Mug).
    try_steal = function(self, actor, target, allow_dead, mug)
        local enemy = self.enemies[target]
        if enemy == nil or (enemy.dead and not allow_dead) then do return end end
        local colour = {1, 1, 0.6}
        if enemy.stolen then
            UiContainer.BattleFx:text(Battle.TargetGroup.ENEMIES, target, "Nothing left", colour)
            do return end
        end
        local source = battle_manager:get_enemy(target - 1)
        local percent = self:get_combat_stats(Battle.TargetGroup.PARTY, actor).level + 40 - source:get_level()
        if mug and percent < 100 then percent = 100 end
        for i = 0, source:get_steal_count() - 1 do
            local rate = math.floor(source:get_steal_rate(i) * 63 + 0.5)
            local scaled = math.floor(rate * math.floor(percent * 512 / 100) / 256)
            if math.random(0, 63) <= scaled then
                local id = source:get_steal_id(i)
                local item = Game.Items[id]
                Inventory.add_item(id, 1)
                enemy.stolen = true
                print(self:get_combatant_name(Battle.TargetGroup.PARTY, actor) .. " steals " .. (item ~= nil and item.name or tostring(id)))
                UiContainer.BattleFx:text(Battle.TargetGroup.ENEMIES, target, "Stole " .. (item ~= nil and item.name or tostring(id)), colour)
                do return end
            end
        end
        UiContainer.BattleFx:text(Battle.TargetGroup.ENEMIES, target, "Couldn't steal", colour)
    end,

    --- Adds a spell or summon to a list, merging the links if it is already there.
    add_spell = function(self, list, spell)
        for _, other in ipairs(list) do
            if other.id == spell.id then
                for _, key in ipairs({"all", "mp_turbo", "hp_absorb", "mp_absorb", "quadra", "added_cut", "steal_as_well"}) do
                    other[key] = math.max(other[key] or 0, spell[key] or 0)
                end
                return
            end
        end
        table.insert(list, spell)
    end,

    --- Gets the MP an attack costs with the materia linked to it (MP Turbo adds its percent).
    get_spell_cost = function(self, mp, links)
        if links == nil or (links.mp_turbo or 0) == 0 then return mp end
        return math.ceil(mp * (100 + links.mp_turbo) / 100)
    end,

    --- Gets a copy of an attack changed by the materia linked to it: MP Turbo adds power, HP Absorb
    -- makes it heal the caster as much as it damages, and MP Absorb gives the caster that much MP.
    apply_links = function(self, attack, links)
        if links == nil then return attack end
        local damage = {}
        for key, value in pairs(attack.damage) do damage[key] = value end
        damage.power_bonus = links.mp_turbo or 0
        if (links.hp_absorb or 0) > 0 then damage.drain_hp = 1 end
        if (links.mp_absorb or 0) > 0 then damage.drain_mp = 1 end
        local copy = {}
        for key, value in pairs(attack) do copy[key] = value end
        copy.damage = damage
        return copy
    end,

    --- Gets a copy of an attack with half of its power, the one of each of the casts of Quadra Magic.
    apply_quadra = function(self, attack)
        local damage = {}
        for key, value in pairs(attack.damage) do damage[key] = value end
        damage.power = (damage.power or 0) / 2
        local copy = {}
        for key, value in pairs(attack) do copy[key] = value end
        copy.damage = damage
        return copy
    end,

    -- Reads materia, calculates stats, computes attacks...
    -- TODO: Move most of this code to Characters.recalculate_stats()
    load_party_members = function(self)
        for i = 1, Party.MAX do
            if self:has_character_data(Party[i]) then
                -- The stats depend on the equipment and the materia (HP<->MP swaps the HP and the MP).
                Characters.recalculate_stats(Party[i])
                self.party[i] = {}
                self.party[i].character = Party[i]
                self:attach_weapon(i)
                --math.randomseed(('' .. os.time()):reverse())
                -- TODO: Consider battle layouts.
                -- TODO: Check random seed
                self.party[i].dead = false
                self.party[i].status = {}
                -- Fury and Sadness remain after the battles.
                for _, id in ipairs(Characters[Party[i]].status or {}) do
                    if id == BattleStatus.FURY or id == BattleStatus.SADNESS then
                        self.party[i].status[id] = {next_tick = timer:get_game_time_total() + BattleStatus.TICK}
                    end
                end
                -- Elements and statuses from the equipment and the linked materia.
                self.party[i].effects = Characters.get_effects(Party[i])
                self.party[i].acting = false
                self.party[i].summon_used = {}
                self.party[i].all_used = {}
                self.party[i].quadra_used = {}
                self.party[i].atb = math.random(0, 255)
                self.party[i].command = {}
                -- Set default "Attack" command.
                self.party[i].command[1] = 1
                self.party[i].magic = {}
                self.party[i].summon = {}
                -- What the independent materia do: Long Range, Mega All, Counter Attack, Cover, Pre-Emptive,
                -- EXP Plus and Gil Plus.
                self.party[i].independent = Characters.get_independent(Party[i])
                self.party[i].defending = false
                -- Characters in the back row start a bit behind.
                local entity = self:get_entity(Battle.TargetGroup.PARTY, i)
                if entity ~= nil then
                    local hx, hy, hz = entity:get_position()
                    self.party[i].home = {x = hx, y = hy, z = hz}
                end
                if Characters[Party[i]].back_row == true then self:move_row(i, true, true) end
                self.party[i].mega_all_used = 0
                -- Fill Limit UI bar
                -- 0   -> 0%
                -- 255 -> 18%-6
                local width_percent = math.min(18, 18 * Characters[self.party[i].character].limit.bar / 255)
                local width_fixed = math.max(-6, -6 * width_percent / 18)
                ui_manager:get_widget("BattleUi.Container.Bottom.Character" .. tostring(i) .. ".Right.LimitFill"):set_width(width_percent, width_fixed)
                -- Counter materia paired with a command or a magic materia.
                self.party[i].counters = self:get_counters(Characters[Party[i]])
                -- Counter Attack counters the physical attacks with a normal attack.
                if self.party[i].independent.counter_attack > 0 then
                    table.insert(self.party[i].counters, {chance = self.party[i].independent.counter_attack / 100, command = 1, physical = true})
                end
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
                            local links = self:get_materia_links(equip, m)
                            if materia.type == 5 then -- Summon
                                local summon = {
                                    id = materia.summon,
                                    times = materia.times[level],
                                    all = links.all,
                                    quadra = links.quadra,
                                    mp_turbo = links.mp_turbo,
                                    mp_absorb = links.mp_absorb,
                                    hp_absorb = links.hp_absorb,
                                    sneak_attack = 0,
                                    final_attack = 0,
                                    added_cut = links.added_cut,
                                    steal_as_well = links.steal_as_well
                                }
                                self:add_spell(self.party[i].summon, summon)
                            elseif materia.type == 1 then -- Magic
                                for lv, spell in ipairs(materia.magic) do
                                    if level >= lv then
                                        local magic = {
                                            id = spell,
                                            group = (e - 1) * 10 + m,
                                            all = links.all,
                                            quadra = links.quadra,
                                            mp_turbo = links.mp_turbo,
                                            mp_absorb = links.mp_absorb,
                                            hp_absorb = links.hp_absorb,
                                            sneak_attack = 0,
                                            final_attack = 0,
                                            added_cut = links.added_cut,
                                            steal_as_well = links.steal_as_well
                                        }
                                        self:add_spell(self.party[i].magic, magic)
                                    end
                                end
                            elseif materia.type == 3 then -- Command
                                -- Pick only the highest level command available
                                local command_id = nil
                                for lv, cmd in ipairs(materia.command) do if level >= lv then command_id = cmd end end
                                if command_id ~= nil then
                                    -- The links of the materia that gives the Attack command (Steal as well).
                                    if command_id ~= 20 and (command_id == 1 or command_id == 24 or command_id == 25 or command_id == 26 or command_id == 27) then
                                        self.party[i].attack_links = links
                                    end
                                    if command_id == 1 or command_id == 20 or command_id == 24 or command_id == 25 or command_id == 26 or command_id == 27 then
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
                      -- Attack substitutes always go first. The order has to be strict (an element is
                      -- never before itself, and two substitutes don't go before each other), or the
                      -- sort fails with "invalid order function".
                      local function substitute(id) return id == 1 or id == 20 or (id >= 24 and id <= 27) end
                      if substitute(l) ~= substitute(r) then return substitute(l) end
                      return l < r
                  end
                )

                -- The spells of a materia go together, in the order of the materia (weapon first).
                if #self.party[i].magic > 1 then
                    table.sort(self.party[i].magic, function(l, r)
                        if l.group ~= r.group then return l.group < r.group end
                        return l.id < r.id
                    end)
                end
                if #self.party[i].summon > 1 then table.sort(self.party[i].summon, function(l, r) return l.id < r.id end) end
                -- The first command is the Attack one (or its replacement), and the Limit one takes its place when
                -- the limit bar is full.
                self.party[i].attack_command = self.party[i].command[1]
                -- The Item command, or W-Item, that a frog turns back into Item.
                for slot, command_id in ipairs(self.party[i].command) do
                    if command_id == 4 or command_id == 23 then
                        self.party[i].item_slot, self.party[i].item_command = slot, command_id
                    end
                end
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
        self.delayed_calls = {}
        self.row_moves = {}
        self.attack_cache = {}
        self.viewer = {combatant = 1, animation = 0}
        self:populate()
        ui_manager:get_widget("BattleUi"):set_visible(true)
        self.active = true;

        -- Lopad combatant data
        self:load_party_members()
        self:load_enemies()
        self.last_party_action = nil
        self.limit_learned = nil
        self:ai_setup()
        self:init_atb()
        -- Near Death and the limit bars at the start of the battle.
        for i = 1, #self.party do
            self:update_near_death(Battle.TargetGroup.PARTY, i)
            self:update_party_bars(i)
            self:update_limit(i)
        end
        for i = 1, #self.enemies do self:update_near_death(Battle.TargetGroup.ENEMIES, i) end
        self:queue_sneak_attacks()
        self:apply_animation_speed()
        UiContainer.BattleFx:show()
        local ok, intro = pcall(BattleCameraController.start_intro, BattleCameraController)
        if not ok then print("Battle camera: " .. tostring(intro)) end
        self.intro = ok and intro
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
        self:show_message("V: target  N/B: anim  F: rotate  C: camera  Z/X: speed  K: end")

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
        -- A member that manipulates an enemy chooses among the attacks of the enemy.
        self:update_manip_commands(party_id)
        local command_list = self:get_command_list(party_id)
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
            if command_list[c] == nil then ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cmd" .. tostring(c)):set_visible(false)
            else
                -- The colour of the Limit command changes, so it is set again.
                if self.party[party_id].manip_commands == nil then
                    self:set_command_widget(party_id, c, command_list[c])
                else
                    -- The attacks of a manipulated enemy.
                    ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cmd" .. tostring(c)):set_colour(1, 1, 1)
                    ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cmd" .. tostring(c)):set_text(Game.Commands[command_list[c]].name)
                end
                ui_manager:get_widget("BattleUi.Container.Bottom.Commands.Cmd" .. tostring(c)):set_visible(true)
                total_commands = total_commands + 1
            end
        end
        -- Resize the window
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.Commands"):set_width(60 * (1 + math.floor(total_commands / 4)))
        self.commands_width = 60 * (1 + math.floor(total_commands / 4))
        ui_manager:get_widget("BattleUi.Container.Bottom.Change"):set_visible(false)
        ui_manager:get_widget("BattleUi.Container.Bottom.Defend"):set_visible(false)
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
