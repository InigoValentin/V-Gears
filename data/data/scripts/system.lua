MAX_INVENTORY_SLOTS = 320
MAX_KEY_SLOTS = 50
MAX_MATERIA_SLOTS = 200

--- System related parameters and utilities
System = {

    --- Indicates if the game is paused.
    paused = false,

    --- Indicates if the menu was available when paused.
    paused_menu_settings_available = false,

    --- Pauses the game.
    pause = function(self)
        if MenuSettings.pause_available == false then
            do return end
        end
        if self.paused == false then
            self.paused = true
            entity_manager:set_paused(true)
            self.paused_menu_settings_available = MenuSettings.available
            MenuSettings.available = false
            audio_manager:play_sound("Cursor")
            UiContainer.PauseMenu:show()
        end
    end,

    --- Unpauses the game.
    unpause = function(self)
        if self.paused == true then
            self.paused = false
            entity_manager:set_paused(false)
            MenuSettings.available = self.paused_menu_settings_available
            audio_manager:play_sound("Cursor")
            UiContainer.PauseMenu:hide()
        end
    end,

    --- Checks if the game is paused.
    --
    -- @return True if the game is paused, false if not.
    is_paused = function(self)
        return self.paused
    end
}

--- Stands for the model of the script entities that have none: the opcodes that move or hide a
-- model are run by them in the original game and do nothing. Any method called on it is ignored.
NullEntity = setmetatable({}, {__index = function() return function() end end})

Timer = {
    -- TODO When the game clock is implemented, dont use FPS, call this every second.
    FPS = 250,
    time = 0,
    frames = 0,
    update = function()
        if Timer.time > 0 or Timer.frames > 0 then
            Timer.frames = Timer.frames - 1
            if Timer.frames == 0 then
                Timer.time = Timer.time - 1
                dialog:update_timer(Timer.time)
                if Timer.time > 0 then
                    Timer.frames = Timer.FPS
                end
                Banks[1][20]--[[timer_hours]] = math.floor(Timer.time / 3600)
                Banks[1][21]--[[timer_minutes]] = math.floor(Timer.time / 60) % 60
                Banks[1][22]--[[timer_seconds]] = Timer.time % 60
            end
            Banks[1][23]--[[timer_frames]] = Timer.frames
        end
    end,
    set = function(time)
        Timer.time = tonumber(time)
        Timer.frames = Timer.FPS
        dialog:update_timer(Timer.time)
        Banks[1][20]--[[timer_hours]] = math.floor(Timer.time / 3600)
        Banks[1][21]--[[timer_minutes]] = math.floor(Timer.time / 60) % 60
        Banks[1][22]--[[timer_seconds]] = Timer.time % 60
        Banks[1][23]--[[timer_frames]] = Timer.frames
    end
}

--- Export character names to the text manager.
--
-- Used to compose dialog with character names. Must be called on start and when a character is
-- renamed.
export_character_names = function()
    for c = 0, 8 do
        if Characters[c] ~= nil then
            text_manager:set_character_name(c, Characters[c].name)
        end
    end
end

--- Export party members to the text manager.
--
-- Used to compose dialog with party character names. Must be called when the party formation is
-- changed in any way.
export_party_members = function()
    if Party[1] == nil then
        c1 = -1
    else
        c1 = Party[1]
    end
    if Party[2] == nil then
        c2 = -1
    else
        c2 = Party[2]
    end
    if Party[3] == nil then
        c3 = -1
    else
        c3 = Party[3]
    end
    text_manager:set_party(c1, c2, c3)
end

--- Restores the HP and MP of every character, also the ones that are not in the party, and clears
-- their status (MHMMX).
restore_all_characters = function()
    for id, character in pairs(Characters) do
        if type(id) == "number" and type(character) == "table" and character.stats ~= nil
          and character.stats.hp ~= nil and character.stats.mp ~= nil then
            Characters.recalculate_stats(id)
            character.stats.hp.current = character.stats.hp.max
            character.stats.mp.current = character.stats.mp.max
            character.status = {}
        end
    end
end

--- Sets the current party.
--
-- @param character1 The first member character ID, the party leader.
-- @param character2 The second member character ID, can be nil.
-- @param character3 The third member character ID, can be nil.
set_party = function(character1, character2, character3)
    Party[1] = character1
    Party[2] = character2
    Party[3] = character3
    export_party_members()
end

--- Sets the party from three values (SPTYE), the ones that GTPYE stored: 254 and 255 are an empty
-- place.
--
-- @param character1 The first member character ID, the party leader.
-- @param character2 The second member character ID.
-- @param character3 The third member character ID.
set_party_from_values = function(character1, character2, character3)
    local function id(value)
        if value == nil or value >= 254 then return nil end
        return value
    end
    set_party(id(character1), id(character2), id(character3))
end

--- Checks if a a character ID actually belongs to a character.
--
-- @param character The ID to check.
-- @return True if the ID is a valid character ID, false otherwise.
is_character = function(character)
    if Characters[character] == nil then
        return false
    end
    return true
end

--- Adds a character to the curent party.
--
-- @param character ID of the character to add.
add_party_character = function(character)
    if Characters[character] == nil then
        print("Tried add to party invalid character \"" .. character .. "\".");
        return
    end
    if Characters[character].locked == true then
        print("Tried add to party locked character \"" .. character .. "\".");
        return
    end
    -- Check if character already in party
    for i = 1, 3 do
        if Party[i] == character then
            print("Character \"" .. character .. "\" already in party.");
            return
        end
    end
    -- Try to add in first free slot.
    for i = 1, 3 do
        if Party[i] == nil then
            Party[i] = character
            export_party_members()
            return
        end
    end
end

--- Removes a character from the curent party.
--
-- @param character ID of the character to remove.
remove_party_character = function(character)
    if Characters[character] == nil then
        print("Tried remove from party invalid character \"" .. character .. "\".");
        return
    end
    if Characters[character].locked == true then
        print("Tried remove from party locked character \"" .. character .. "\".");
        return
    end
    for i = 1, 3 do
        if Party[i] == character then
            Party[i] = nil
            export_party_members()
            return
        end
    end
end

--- Locks a character in the party.
--
-- Locked characters can't be manually  swapped in or out of the party.
--
-- @param character ID of the character to lock.
set_party_lock = function(character, lock)
    if Characters[character] == nil then
        print("Tried set/unset party lock to invalid character \"" .. character .. "\".");
        return
    end
    Characters[character].locked = lock;
end

--- Checks if a character is locked in the party.
--
-- Locked characters can't be manually  swapped in or out of the party.
--
-- @param character ID of the character to check for locks.
get_party_lock = function(character)
    if Characters[character] == nil then
        print("Tried get party lock from invalid character \"" .. character .. "\".");
        return false
    end
    return Characters[character].locked
end

--- Enables or disables a character is the PHS.
--
-- Disabled characters don't appear in the PHS.
--
-- @param character ID of the character to enable or disable.
-- @param enable True to enable, false to disable.
set_character_enable = function(character, enable)
    if Characters[character] == nil then
        print("Tried enable/disable invalid character \"" .. character .. "\".");
        return
    end
    Characters[character].enabled = enable;
    -- As in ff7.exe (0x61c782), a character that is hidden also leaves the party, locked or not.
    if not enable then
        for i = 1, 3 do
            if Party[i] == character then
                Party[i] = nil
                export_party_members()
            end
        end
    end
end

--- Checks if a character is enabled in the PHS.
--
-- Disabled characters don't appear in the PHS.
--
-- @param character ID of the character to check.
-- @return True if enabled, false if disables.
get_character_enable = function(character)
    if Characters[character] == nil then
        print("Tried get enabled from invalid character \"" .. character .. "\".");
        return false
    end
    return Characters[character].enabled;
end

--- Checks if a character is in the current party (IFPRTYQ).
--
-- @param character ID of the character to check.
-- @return True if the character is in one of the party slots.
IFPRTYQ = function(character)
    for i = 1, 3 do
        if Party[i] == character then return true end
    end
    return false
end

--- Checks if a character is available as a party member (IFMEMBQ).
--
-- A character is available when it is enabled in the PHS, whether it is in the party or not.
--
-- @param character ID of the character to check.
-- @return True if the character is enabled.
IFMEMBQ = function(character)
    return Characters[character] ~= nil and Characters[character].enabled == true
end

--- Adds items to the inventory.
--
-- Inventory quantities will be capped at 99.
--
-- @param item Item ID.
-- @param quantity Quantity to add.
Inventory.add_item = function(item, quantity)
    -- TODO: Also check weapons, armors and accessories.
    if Game.Items[item] == nil then
        print("Tried add invalid item \"" .. item .. "\".");
        return
    end
    for i = 1, MAX_INVENTORY_SLOTS do
        if Inventory[i] ~= nil and Inventory[i].item == item then
            -- Item already in inventory, add quantity
            Inventory[i].quantity = Inventory[i].quantity + quantity
            if Inventory[i].quantity > 99 then
                Inventory[i].quantity = 99
            end
            return
        end
    end
    -- Item not in inventory, search for the first available position.
    for i = 1, MAX_INVENTORY_SLOTS do
        if Inventory[i] == nil then
            Inventory[i] = {item = item, quantity = quantity}
            if Inventory[i].quantity > 99 then
                Inventory[i].quantity = 99
            end
            return
        end
    end
    -- If reached this point, it means that the item is not in inventory, and all slots are full.
    -- TODO: The orignal game does nothing, but in here?
    return
end

--- Checks how many items of a type are in the inventory.
--
-- @param item The item ID.
-- @return Number of the items in the inventory.
Inventory.get_item_quantity = function(item)
    local qty = 0
    for i = 1, MAX_INVENTORY_SLOTS do
        if Inventory[i] ~= nil and Inventory[i].item == item then
            qty = qty + Inventory[i].quantity
        end
    end
    return qty
end

--- Removes items from the inventory.
--
-- Negative quantities can't remain in the inventory. If there are less than requested to remove,
-- all of them will be removed.
--
-- @param item Item ID.
-- @param quantity Quantity to remove.
Inventory.remove_item = function(item, quantity)
    for i = 1, MAX_INVENTORY_SLOTS do
        if Inventory[i] ~= nil and Inventory[i].item == item then
            -- Item already in inventory, add quantity
            Inventory[i].quantity = Inventory[i].quantity - quantity
            if Inventory[i].quantity <= 0 then
                Inventory[i] = nil
            end
            return
        end
    end
end

--- Adds a key item.
--
-- @param item ID of the key item (0-50).
Inventory.add_key_item = function(item)
    Inventory.key[item] = true
    return
end

--- Removes a key item.
--
-- @param item ID of the key item (0-50).
Inventory.remove_key_item = function(item)
    Inventory.key[item] = false
    return
end

--- Checks if the party has a key item.
--
-- @param item ID of the key item (0-50).
-- @return True if the party has the key item, false otherwise.
Inventory.has_key_item = function(item)
    return Inventory.key[item]
end

--- Adds money.
--
-- @param amount The money to add.
Inventory.add_money = function(amount)
    Inventory.money = Inventory.money + amount
    if Inventory.money > 99999999 then
        Inventory.money = 99999999
    end
end

--- Removes money.
--
-- Negative quantities can't remain as money. If there are less than requested to remove, all of
-- it will be removed.
--
-- @param amount Money to remove.
Inventory.remove_money = function(amount)
    Inventory.money = Inventory.money - amount
    if Inventory.money < 0 then
        Inventory.money = 0
    end
end

--- Copies the amount of gil the party has into data banks
--
-- As the gil amount is a four-byte value, in the original game the arguments require two
-- destination addresses to place two two-byte values into. Address 1 takes the lower two bytes of
-- the gil amount, while address 2 takes the higher two bytes.
--
-- @param b1 Bank of the first destination (lower two bytes).
-- @param b2 Bank of the second destination (higher two bytes).
-- @param a1 Address in the first bank.
-- @param a2 Address in the second bank.
Inventory.money_to_banks = function(b1, b2, a1, a2)
    Banks[b1][a1] = Inventory.money % 65536
    Banks[b2][a2] = math.floor(Inventory.money / 65536)
end

--- Sorts the items in the inventory.
--
-- @param criteria {@see Inventory.ORDER.CUSTOM} (unimplemented), {@see Inventory.ORDER.FIELD.},
-- {@see Inventory.ORDER.BATTLE}, {@see Inventory.ORDER.THROW}, {@see Inventory.ORDER.TYPE},
-- {@see Inventory.ORDER.NAME}, {@see Inventory.ORDER.MOST} or {@see Inventory.ORDER.LEAST}. Any
-- othe rvalue and the items will not be sorted.
Inventory.sort = function(criteria)
    if criteria == Inventory.ORDER.CUSTOM then
        print("Custom order unimplemented")
    elseif criteria == Inventory.ORDER.FIELD then
        table.sort(Inventory, function (left, right)
            if Game.Items[left.item].menu ~= Game.Items[right.item].menu then
                return Game.Items[left.item].menu > Game.Items[right.item].menu
            end
            return left.item < right.item
        end
        )
    elseif criteria == Inventory.ORDER.BATTLE then
        table.sort(Inventory, function (left, right)
            if Game.Items[left.item].battle ~= Game.Items[right.item].battle then
                return Game.Items[left.item].battle > Game.Items[right.item].battle
            end
            return left.item < right.item
        end
        )
    elseif criteria == Inventory.ORDER.THROW then
        table.sort(Inventory, function (left, right)
            if Game.Items[left.item].throw ~= Game.Items[right.item].throw then
                return Game.Items[left.item].throw > Game.Items[right.item].throw
            end
            return left.item < right.item
        end
        )
    elseif criteria == Inventory.ORDER.TYPE then
        table.sort(Inventory, function (left, right)
            return left.item < right.item
        end
        )
    elseif criteria == Inventory.ORDER.NAME then
        table.sort(Inventory, function (left, right)
            return Game.Items[left.item].name < Game.Items[right.item].name
        end
        )
    elseif criteria == Inventory.ORDER.MOST then
        table.sort(Inventory, function (left, right)
            if left.quantity ~= right.quantity then
                return left.quantity > right.quantity
            end
            return left.item < right.item
        end
        )
    elseif criteria == Inventory.ORDER.LEAST then
        table.sort(Inventory, function (left, right)
            if left.quantity ~= right.quantity then
                return left.quantity < right.quantity
            end
            return left.item < right.item
        end
        )
    end
end

-- Leveling up, as in ff7.exe (1998 build, functions 0x5c76d9 to 0x5c7bb1).

Characters.MAX_LEVEL = 99
Characters.MAX_EXP = 999999999
--- Caps of the base stats that level ups give. Sources go to the stat bonus, not to the base.
Characters.MAX_BASE_STAT = 100
Characters.MAX_BASE_HP = 9999
Characters.MAX_BASE_MP = 999

--- Divides truncating toward zero, like the integer divisions of the original.
local function divide(a, b)
    local quotient = a / b
    if quotient < 0 then return math.ceil(quotient) end
    return math.floor(quotient)
end

--- Gets the segment of the growth curves used for a level: one every 10 levels up to 60, then one
-- every 20.
local function curve_segment(level)
    if level <= 60 then return math.max(0, math.floor((level - 1) / 10)) end
    return math.min(7, math.floor((level - 61) / 20) + 6)
end

--- Picks one of the 12 bonuses: a random number from 1 to 8 plus how far the character is behind.
local function bonus_index(difference)
    return math.max(0, math.min(11, math.random(1, 8) + difference))
end

--- Gets the total experience a character needs to go from a level to the next one.
--
-- It's the sum of gradient * n^2 / 10 for n from 1 to the level, all of them with the gradient of
-- the segment of that level.
--
-- @param id Character ID.
-- @param level The level.
-- @return The total experience, 0 for level 0.
Characters.exp_threshold = function(id, level)
    local gradient = Game.Growth.curves[Game.Characters[id].curves.exp][curve_segment(level)].gradient
    local total = 0
    for n = 1, level do total = total + math.floor(gradient * n * n / 10) end
    return total
end

--- Raises the base stats of a character for one level.
--
-- Each stat has a curve that gives the value expected at the new level; the further the character
-- is below it, the better the bonus is likely to be.
local function level_up(id)
    local character = Characters[id]
    local stats = character.stats
    local level = character.level
    local segment = curve_segment(level)
    local curves = Game.Characters[id].curves
    local function curve(name) return Game.Growth.curves[curves[name]][segment] end

    for _, name in ipairs({"str", "vit", "mag", "spr", "dex", "lck"}) do
        local stat = stats[name]
        local value = math.max(1, stat.base)
        local data = curve(name)
        local expected = math.floor(data.gradient * (level + 1) / 100) + data.base
        local gain = Game.Growth.bonus.primary[bonus_index(expected - value)]
        stat.base = math.min(Characters.MAX_BASE_STAT, value + gain)
    end

    local hp = curve("hp")
    local function expected_hp(n) return hp.gradient * n + hp.base * 40 end
    local value = math.max(1, stats.hp.base)
    local index = bonus_index(divide(expected_hp(level) * 100, value) - 100)
    local gain = divide((expected_hp(level + 1) - expected_hp(level)) * Game.Growth.bonus.hp[index], 100)
    stats.hp.base = math.min(Characters.MAX_BASE_HP, value + gain)

    local mp = curve("mp")
    local function expected_mp(n) return math.floor(mp.gradient * n / 10) + mp.base * 2 end
    value = math.max(1, stats.mp.base)
    index = bonus_index(divide(expected_mp(level) * 100, value) - 100)
    gain = divide((expected_mp(level + 1) - expected_mp(level)) * Game.Growth.bonus.mp[index], 100)
    stats.mp.base = math.min(Characters.MAX_BASE_MP, value + gain)

    character.level = level + 1
end

--- Gives experience to a character, and levels it up as many times as it reaches.
--
-- The current HP and MP don't change, only the base values and the maximums.
--
-- @param id Character ID.
-- @param exp Experience gained.
-- @return The number of levels gained.
Characters.add_exp = function(id, exp)
    local character = Characters[id]
    local definition = Game.Characters[id]
    if character == nil or definition == nil or definition.curves == nil then return 0 end
    character.exp = math.min(Characters.MAX_EXP, (character.exp or 0) + exp)
    local first_level = character.level
    while character.level < Characters.MAX_LEVEL
      and character.exp >= Characters.exp_threshold(id, character.level) do
        level_up(id)
    end
    if character.level < Characters.MAX_LEVEL then
        character.exp_to_next = Characters.exp_threshold(id, character.level) - character.exp
    else
        character.exp_to_next = 0
    end
    if character.level > first_level then Characters.recalculate_stats(id) end
    return character.level - first_level
end

--- Variable of the field scripts with the level of the characters whose level doesn't depend on
-- the one of the party (Yuffie): bank 1, address 2.
Characters.JOIN_LEVEL_BANK = 1
Characters.JOIN_LEVEL_ADDRESS = 2

--- Gets the average level of the available characters (in the party or enabled in the PHS), without
-- counting one of them.
--
-- @param excluded ID of the character that isn't counted.
-- @return The average, or the level of the excluded character if there is nobody else.
Characters.average_level = function(excluded)
    local sum, count = 0, 0
    for id = 0, 8 do
        if id ~= excluded and Characters[id] ~= nil and (IFMEMBQ(id) or IFPRTYQ(id)) then
            sum = sum + Characters[id].level
            count = count + 1
        end
    end
    if count == 0 then return Characters[excluded].level end
    return math.floor(sum / count)
end

--- Copies a table and the tables in it.
local function copy_table(source)
    if type(source) ~= "table" then return source end
    local result = {}
    for key, value in pairs(source) do result[key] = copy_table(value) end
    return result
end

--- Gives a character the data it joins with. The slots of Cait Sith and Vincent hold young Cloud and
-- Sephiroth at the start: ff7.exe replaces the whole slot with data of its own when the window to
-- name them opens (0x70ec9a). Nothing happens for the other characters.
--
-- The availability and the lock in the party are not part of that data, and are kept.
--
-- @param id Character ID.
Characters.load_recruit_record = function(id)
    local record = Game.RecruitRecords ~= nil and Game.RecruitRecords[id] or nil
    if record == nil or Characters[id] == nil then return end
    local character = copy_table(record)
    character.enabled = Characters[id].enabled
    character.locked = Characters[id].locked
    Characters[id] = character
    Characters.recalculate_stats(id)
end

--- Sets the level of a character that joins, as in ff7.exe (0x5c72a6). The original does it when the
-- window to name the character is closed (for everybody but Cloud).
--
-- The level is the average of the other available characters plus half of the level offset of the
-- character. With an odd offset (Yuffie) it comes from a variable of the field scripts instead, and
-- the character is built again from level 1. The experience of the previous level is added to the
-- one the character has, and it levels up as usual, so it never goes down. HP and MP end up filled.
--
-- @param id Character ID.
Characters.join = function(id)
    local character = Characters[id]
    local definition = Game.Characters[id]
    if id == 0 or character == nil or definition == nil or definition.curves == nil then return end
    -- The offset is a signed byte.
    local offset = tonumber(definition.initial_level) or 0
    if offset > 127 then offset = offset - 256 end
    local stats = character.stats
    local level
    if offset % 2 == 0 then
        level = Characters.average_level(id) + divide(offset, 2)
    else
        level = Banks[Characters.JOIN_LEVEL_BANK][Characters.JOIN_LEVEL_ADDRESS] or 0
        -- It is unknown what writes the variable in a normal game: without it, the character would
        -- be left at level 1, so the average of the party is used.
        if level == 0 then level = Characters.average_level(id) end
        character.level = 1
        character.exp = 0
        for _, name in ipairs({"str", "vit", "mag", "spr", "dex", "lck", "hp", "mp"}) do
            stats[name].base = 1
        end
    end
    level = math.max(1, math.min(Characters.MAX_LEVEL, level))
    Characters.add_exp(id, Characters.exp_threshold(id, level - 1))
    stats.hp.current = stats.hp.base
    stats.mp.current = stats.mp.base
    Characters.recalculate_stats(id)
    print(character.name .. " joins at level " .. tostring(character.level))
end

--- Names of the primary stats.
Characters.PRIMARY_STATS = {"str", "vit", "mag", "spr", "dex", "lck"}

--- Cap of the primary stats and of the bonus that the sources give, as in ff7.exe.
Characters.MAX_STAT = 255

--- Primary stat or maximum raised by each of the Plus materia, by the ID of its ability.
Characters.PLUS_MATERIA = {
    [0] = "str", [1] = "vit", [2] = "mag", [3] = "spr", [4] = "dex", [5] = "lck", [8] = "hp", [9] = "mp",
}

--- Item ID of the Curse Ring, and what it adds to the stats on top of the bonuses of its data.
Characters.CURSE_RING = 313
Characters.CURSE_RING_BONUS = {str = 15, vit = 15, mag = 15, spr = 15, dex = 15, lck = 10}

--- Uses a source on a character: one more point of bonus for a primary stat.
--
-- @param id Character ID.
-- @param name Name of the stat, one of {@see Characters.PRIMARY_STATS}.
-- @return False if the bonus is already at its cap, and nothing happens.
Characters.add_stat_bonus = function(id, name)
    local stat = Characters[id].stats[name]
    if (stat.bonus or 0) >= Characters.MAX_STAT then return false end
    stat.bonus = (stat.bonus or 0) + 1
    Characters.recalculate_stats(id)
    return true
end

Characters.recalculate_stats = function(id)
    -- Initialize
    local stats = Characters[id].stats
    -- The primary stats start with the base value, that grows with the levels, plus the bonus that the
    -- sources give.
    for _, name in ipairs(Characters.PRIMARY_STATS) do
        stats[name].total = stats[name].base + (stats[name].bonus or 0)
    end
    stats.hp.max = stats.hp.base
    stats.mp.max = stats.mp.base
    local weapon = Game.Items[Characters[id].weapon.id]
    local armor = Game.Items[Characters[id].armor.id]
    -- Bonuses of the equipment: points added to the primary stats. They are keyed by name, so they are
    -- walked with pairs (ipairs only walks the array part and found nothing).
    for _, item in pairs({weapon, armor, Game.Items[Characters[id].accessory]}) do
        for name, value in pairs(item.stats or {}) do
            if stats[name] ~= nil and stats[name].total ~= nil then
                stats[name].total = stats[name].total + value
            end
        end
    end
    -- The Curse Ring has more bonuses than its data can hold, ff7.exe adds them (0x6bbb04).
    if Characters[id].accessory == Characters.CURSE_RING then
        for name, value in pairs(Characters.CURSE_RING_BONUS) do stats[name].total = stats[name].total + value end
    end
    -- Get a list of materias
    local materias, equipped_materias = {}, {}
    for _, part in ipairs({Characters[id].weapon, Characters[id].armor}) do
        for m = 0, 7 do
            local equipped = part.materia[m]
            if equipped ~= nil and Game.Materia[equipped.id] ~= nil then
                table.insert(materias, Game.Materia[equipped.id])
                table.insert(equipped_materias, equipped)
            end
        end
    end
    -- Materia changes, as in ff7.exe (0x5cd1e8 and 0x5ce487): points for the primary stats and percents
    -- for HP and MP, that every equipped materia adds up, and the percents of the Plus materia (HP,
    -- MP, Speed, Magic and Luck Plus).
    local points, percents = {}, {}
    local function add(list, name, value) list[name] = (list[name] or 0) + value end
    for index, materia in ipairs(materias) do
        for name, value in pairs(materia.stats or {}) do
            if name == "hp" or name == "mp" then add(percents, name, value)
            else add(points, name, value)
            end
        end
        local name = materia.type == Materia.TYPE.INDEPENDENT and materia.ability ~= nil
          and Characters.PLUS_MATERIA[materia.ability.id] or nil
        if name ~= nil then
            local level = math.max(1, Characters.get_materia_level(materia, equipped_materias[index].ap or 0))
            local param = materia.ability.param
            add(percents, name, param[level] or param[#param])
        end
    end
    local function percent_of(value, name)
        local result = value * math.min(100, percents[name] or 0) / 100
        -- Truncated toward zero, like the integer division of the original.
        if result < 0 then return math.ceil(result) end
        return math.floor(result)
    end
    -- The percent of a Plus materia applies to the stat with the equipment, without the materia points.
    for _, name in ipairs(Characters.PRIMARY_STATS) do
        local value = math.min(Characters.MAX_STAT, stats[name].total)
        value = value + percent_of(value, name) + (points[name] or 0)
        stats[name].total = math.max(0, math.min(Characters.MAX_STAT, math.floor(value)))
    end
    stats.hp.max = math.min(Characters.MAX_BASE_HP, stats.hp.max + percent_of(stats.hp.max, "hp"))
    stats.mp.max = math.min(Characters.MAX_BASE_MP, stats.mp.max + percent_of(stats.mp.max, "mp"))

    -- HP<->MP materia swap the maximum HP and MP. TODO: Unverified: its levels (it is done at any level).
    for _, materia in ipairs(materias) do
        if materia ~= nil and materia.type == Materia.TYPE.INDEPENDENT and materia.ability ~= nil
          and materia.ability.id == Materia.ABILITY.HP_MP then
            stats.hp.max, stats.mp.max = stats.mp.max, stats.hp.max
            break
        end
    end
    -- The current values can't be over the maximums.
    if stats.hp.current ~= nil then stats.hp.current = math.min(stats.hp.current, math.max(1, stats.hp.max)) end
    if stats.mp.current ~= nil then stats.mp.current = math.min(stats.mp.current, math.max(0, stats.mp.max)) end

    if stats.hp.max < 1 then
        stats.hp.max = 1
    end
    if stats.mp.max < 1 then
        stats.mp.max = 1
    end

    -- Calculate composed stats.
    stats.atk = stats.str.total + weapon.power
    stats.acc = weapon.accuracy
    stats.def = stats.vit.total + armor.defense.physical
    stats.eva = armor.evasion.physical
    stats.matk = stats.mag.total
    stats.mdef = stats.spr.total + armor.defense.magical
    stats.meva = armor.evasion.magical

    -- Cap composed stats. TODO: Max caps?
    if stats.atk < 1 then
        stats.atk = 1
    end
    if stats.acc < 1 then
        stats.acc = 1
    end
    if stats.def < 1 then
        stats.def = 1
    end
    if stats.eva < 0 then
        stats.eva = 0
    end
    if stats.matk < 1 then
        stats.matk = 1
    end
    if stats.mdef < 1 then
        stats.mdef = 1
    end
    if stats.meva < 0 then
        stats.meva = 0
    end

    -- Cap current HP/MP
    if stats.hp.current > stats.hp.max then
        stats.hp.current = stats.hp.max
    end
    if stats.mp.current > stats.mp.max then
        stats.mp.current = stats.mp.max
    end

    -- Assign
    Characters[id].stats = stats
end

--- Sends the materia that doesn't fit in a piece of equipment back to the materia inventory.
--
-- @param char_id ID of the character.
-- @param part "weapon" or "armor".
-- @param slots Number of materia slots the equipment will have. Slots are 0-7 in the character.
Characters.remove_excess_materia = function(char_id, part, slots)
    local materia = Characters[char_id][part].materia
    for m = slots, 7 do
        if materia[m] ~= nil then
            Materia.add(materia[m].id, materia[m].ap)
            materia[m] = nil
        end
    end
end

--- Equips an item on a character.
--
-- Works for weapons, armors and accessories. If there is a previously equiped item, it
-- will be unequiped and added to the inventory. The equiped item will be removed form the
-- inventory. For armors and weapons, if the newly equiped item has less materia slots than
-- the previuos one, remainig materia will be unequiped and added to the materia inventory.
--
-- @param char_id ID of the character to equip the item to.
-- @param item_id ID of the item to equip.
-- @return True if the item was equipped. False if the character or item don't exist, if
-- the item is not in the inventory, or if the item is not equippable or not equippable by
-- the selected character. In any of those cases, an error message will be printed.
Characters.equip = function(char_id, item_id)

    -- Error checks
    if Characters[char_id] == nil then
        print("Tried to equip item on wrong character: " .. tostring(char_id))
        return false
    elseif Game.Items[item_id] == nil then
        print("Tried to equip invalid item: " .. tostring(item_id))
        return false
    elseif Inventory.get_item_quantity(item_id) < 1 then
        print("Tried to equip item not in inventory: " .. item_id .. " (" .. Game.Items[item_id].name .. ")")
        return false
    end
    local type = Game.Items[item_id].type
    if type ~= Inventory.ITEM_TYPE.WEAPON and type ~= Inventory.ITEM_TYPE.ARMOR and type ~= Inventory.ITEM_TYPE.ACCESSORY then
        print("Tried to equip non-equipable item: " .. item_id .. " (" .. Game.Items[item_id].name .. ")")
        return false
    end
    local equipable = false
    for _, u in ipairs(Game.Items[item_id].users) do
        if u == char_id then
            equipable = true
            break
        end
    end
    if equipable == false then
        print("Tried to equip item " .. tostring(item_id) .. " (" .. Game.Items[item_id].name .. ") on a non valid character: " .. tostring(char_id) .. " (" .. Characters[char_id].name .. ")")
        return false
    end

    -- All is good, equip.
    if type == Inventory.ITEM_TYPE.ACCESSORY then
        -- Equip accessory. If any was equipped, send it to the inventory.
        if Characters[char_id].accessory ~= nil then
            Inventory.add_item(Characters[char_id].accessory, 1)
        end
        Characters[char_id].accessory = item_id
        Inventory.remove_item(item_id, 1)
    elseif type == Inventory.ITEM_TYPE.WEAPON then
        -- Weapon. Remove excess materia, send previuos one to the inventory.
        Characters.remove_excess_materia(char_id, "weapon", #(Game.Items[item_id].slots))
        Inventory.add_item(Characters[char_id].weapon.id, 1)
        Characters[char_id].weapon.id = item_id
        Inventory.remove_item(item_id, 1)
    elseif type == Inventory.ITEM_TYPE.ARMOR then
        -- Armor. Remove excess materia, send previuos one to the inventory.
        Characters.remove_excess_materia(char_id, "armor", #(Game.Items[item_id].slots))
        Inventory.add_item(Characters[char_id].armor.id, 1)
        Characters[char_id].armor.id = item_id
        Inventory.remove_item(item_id, 1)
    else
        return false
    end
    -- The stats change with the equipment (and the maximum HP and MP, so the current ones may have to be capped).
    Characters.recalculate_stats(char_id)
    return true
end

--- Takes the equipped materia off a character and puts it in the materia inventory (menu 17 of the
-- field scripts: the "remove materia from other members" of the scenes with a fixed party).
--
-- @param id ID of the character.
Characters.unequip_materia = function(id)
    local character = Characters[id]
    if character == nil then return end
    local free = 1
    for _, equipment in ipairs({character.weapon, character.armor}) do
        for slot = 0, 7 do
            local materia = equipment.materia[slot]
            if materia ~= nil then
                while free <= MAX_MATERIA_SLOTS and Materia[free] ~= nil do free = free + 1 end
                if free > MAX_MATERIA_SLOTS then return end
                Materia[free] = materia
                equipment.materia[slot] = nil
            end
        end
    end
    Characters.recalculate_stats(id)
end

--- Removes all materia from a character.
--
-- @param id ID of the character to remove materia from.
Characters.remove_materia = function(id)
    -- TODO: Implement
end

--- Restores all materia to a character.
--
-- @param id ID of the character to restore materia to.
Characters.restore_materia = function(id)
    -- TODO: Implement
end


Materia.add = function(id, ap)
    local ap = ap or 0
    if Game.Materia[id] == nil then
        print("Tried to add invalid materia \"" .. tostring(id) .. "\".");
        return
    end
    -- TODO Cap AP?
    for i = 1, MAX_MATERIA_SLOTS do
        if Materia[i] == nil then
            Materia[i] = {id = id, ap = ap}
            if Materia[i].id == Materia.ENEMY_SKILL_ID then
                Materia[i].ap = 0
                Materia[i].skills = {}
                for s = 1, 24 do
                    Materia[i].skills[s] = false
                end
            end
            return
        end
    end
    -- If reached this point, it means that the materia wasn't added because all slots are full.
    -- TODO: Do something.
    return
end

--- Splits the party in teams.
--
-- @param number Number of teams (only 2 or 3).
Party.split_teams = function(number)
    if number < 2 then
        number = 2
    elseif number > 3 then
        number = 3
    end
    -- TODO: Implement.
end

--- Displays a menu to choose a party of three.
--
-- The script that asks for it waits until the menu is closed.
Party.choose_party = function()
    -- The button that asked for the menu (Cancel) is still down: it would close the menu at once.
    while entity_manager:is_key_on(64) do script:wait(0) end
    UiContainer.PhsMenu.standalone = true
    script:request_end_sync(Script.UI, "PhsMenu", "show", 0)
    while UiContainer.current_menu == "phs" do script:wait(0) end
    -- The button that closed the menu is still down: the script that asked for the menu would
    -- read it as a new request (Cancel opens the menu).
    while entity_manager:is_key_on(32 + 64) do script:wait(0) end
end

--- Special event. Removes all materia from the party, equipped and unequipped.
Party.steal_materia = function()
    -- TODO: Implement.
    -- TODO: How is this reversed?
end

--- Integer division of the field scripts (DIV, DIV2).
--
-- @param a Dividend.
-- @param b Divisor. The result is the dividend if it is zero.
-- @return The integer part of a / b.
function math_div(a, b)
    if b == 0 then return a end
    return math.floor(a / b)
end

--- Implementation of OR bit operation.
--
-- @param a First operand.
-- @param b Second operand.
-- @return a | b.
function bit_or(a, b)
    local r, m, s = 0, 2 ^ 31
    repeat
        s, a, b = a + b + m, a % m, b % m
        r, m = r + m * 1 % (s - a - b), m / 2
    until m < 1
    return r
end

--- Implementation of XOR bit operation.
--
-- @param a First operand.
-- @param b Second operand.
-- @return a ^^ b.
function bit_xor(a, b)
    local r, m, s = 0, 2 ^ 31
    repeat
        s, a, b = a + b + m, a % m, b % m
        r, m = r + m * 3 % (s - a - b), m / 2
    until m < 1
    return r
end

--- Implementation of AND bit operation.
--
-- @param a First operand.
-- @param b Second operand.
-- @return a & b.
function bit_and(a, b)
    local r, m, s = 0, 2 ^ 31
    repeat
        s, a, b = a + b + m, a % m, b % m
        r, m = r + m * 4 % (s - a - b), m / 2
    until m < 1
    return r
end

--- Obtains a bit from an address in a data bank
--
-- @param bank The bank (0-14)
-- @param address The bank address (0-254)
-- @param bit The bit to get (0-7)
-- @return The bit value (1-0)
function get_bank_bit(bank, address, bit)
    if bank < 0 or bank > Banks.COUNT or address < 0 or address > Banks.SIZE or bit < 0 or bit > 7 then
        print("ERROR: Tried to check invalid bit " .. tostring(bit) .. " from bank " .. tostring(bank) .. " at address " .. tostring(address) .. ".")
        return 0
    end
    local val = Banks[bank][address]
    return get_byte_nth_bit(val, bit)
end

--- Sets a bit in an address in a data bank
--
-- @param bank The bank (0-14)
-- @param address The bank address (0-254)
-- @param bit The bit to set (0-7)
-- @param value The bit value (1-0)
function set_bank_bit(bank, address, bit, value)
    if bank < 0 or bank > Banks.COUNT or address < 0 or address > Banks.SIZE or bit < 0 or bit > 7 then
        print("ERROR: Tried to set invalid bit " .. tostring(bit) .. " from bank " .. tostring(bank) .. " at address " .. tostring(address) .. ".")
        return
    end
    if value ~= 1 and value ~= 0  then
        print("ERROR: Tried to set bit to invalid value " .. tostring(value) .. " from bank " .. tostring(bank) .. " at address " .. tostring(address) .. ".")
        return
    end
    local val = Banks[bank][address]
    Banks[bank][address] =  set_byte_nth_bit(val, bit, value)
end

--- Obtains a bit from an unsigned byte
--
-- @param value The value (0-254)
-- @param bit The bit to get (0-7)
-- @return The bit value (1-0)
function get_byte_nth_bit(value, n)
    if n < 0 or n > 7 then
        print("ERROR: Tried to get invalid bit " .. tostring(bit) .. " from value " .. tostring(value) .. ".")
        return 0
    end
    if value < 0 then
        print("ERROR: Tried to get bit " .. tostring(bit) .. " from negative value " .. tostring(value) .. ".")
        return 0
    end
    if value > 254 then
        print("ERROR: Tried to get bit " .. tostring(bit) .. " from value " .. tostring(value) .. " that exceeds byte size.")
        return 0
    end
    -- Convert to binary string
    local s = ""
    local temp = value
    for i = 1, 8 do
        s = s .. tostring(temp % 2)
        temp = math.floor(temp / 2)
    end
    return tonumber(string.sub(s, n + 1, n + 1))
end

--- Obtains a bit from an unsigned byte
--
-- @param value The value (0-254)
-- @param bit The bit to get (0-7)
-- @return The bit value (1-0)
function set_byte_nth_bit(value, n, bit)
    if bit ~= 0 and bit ~= 1 then
        print("ERROR: Tried to set bit " .. tostring(bit) .. " to an invalid value " .. tostring(bit) .. " from value " .. tostring(value) .. ".")
        return value
    end
    if n < 0 or n > 7 then
        print("ERROR: Tried to set invalid bit " .. tostring(bit) .. " to " .. tostring(bit) .. " from value " .. tostring(value) .. ".")
        return value
    end
    if value < 0 then
        print("ERROR: Tried to set bit " .. tostring(bit) .. " to " .. tostring(bit) .. "from negative value " .. tostring(value) .. ".")
        return value
    end
    if value > 254 then
        print("ERROR: Tried to set bit " .. tostring(bit) .. " to " .. tostring(bit) .. "from value " .. tostring(value) .. " that exceeds byte size.")
        return value
    end
    -- Convert to binary string.
    local s = ""
    local temp = value
    for i = 1, 8 do
        s = s .. tostring(temp % 2)
        temp = math.floor(temp / 2)
    end
    -- Replace the requested bit.
    s = string.sub(s, 1, n) .. tostring(bit) .. string.sub(s, n + 2)
    -- Convert back to a number.
    val = 0
    for i = 1, 8 do
        val = val + (tonumber(string.sub(s, i, i )) * 2 ^ (i - 1))
    end
    return math.floor(val)
end
set_byte_nth_bit(25, 2, 1)
