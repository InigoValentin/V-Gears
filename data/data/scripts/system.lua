MAX_INVENTORY_SLOTS = 320
MAX_KEY_SLOTS = 50
MAX_MATERIA_SLOTS = 200

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

--- Sets the current party.
--
-- @param character1 The first member character ID, the party leader.
-- @param character2 The second member character ID, can be nil.
-- @param character3 The third member character ID, can be nil.
set_party = function(character1, character2, character3)
    Party[1] = character1
    Party[2] = character2
    Party[3] = character3
    Party[1] = character1
    Party[2] = character2
    Party[3] = character3
    export_party_members()
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
    if Characters[character].party_lock == true then
        print("Tried add to party locked character \"" .. character .. "\".");
        return
    end
    -- Check if character already in party
    for x = 1, 3 do
        if Party[i] == character then
            print("Character \"" .. character .. "\" already in party.");
            return
        end
    end
    -- Try to add in first free slot.
    for x = 1, 3 do
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
    if Characters[character].party_lock == true then
        print("Tried remove from party locked character \"" .. character .. "\".");
        return
    end
    for x = 1, 3 do
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
    Characters[character].party_lock = lock;
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
    return Characters[character].party_lock
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
    Characters[character].enable = enable;
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
    return Characters[character].enable;
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
-- @param b1 First destination bank.
-- @param b2 Second destination address.
-- @param a1 First destination bank.
-- @param a2 Second destination address.
Inventory.money_to_2_banks = function(b1, b2, a1, a2)
    Banks[b1][a1] = math.floor(Inventory.money / 65535)
    Banks[b2][a2] = math.floor(Inventory.money % 65535) - math.floor(Inventory.money / 65535)
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

Characters.recalculate_stats = function(id)
    -- Initialize
    local stats = Characters[id].stats
    stats.str.total = stats.str.base
    stats.vit.total = stats.vit.base
    stats.mag.total = stats.mag.base
    stats.spr.total = stats.spr.base
    stats.dex.total = stats.dex.base
    stats.lck.total = stats.lck.base
    stats.hp.max = stats.hp.base
    stats.mp.max = stats.mp.base
    -- Weapon bonus. HP and MP are percents of base, the rest absolutes.
    local weapon = Game.Items[Characters[id].weapon.id]
    for stat, value in ipairs(weapon.stats) do
        if stat == "str" then
            stats.str.total = stats.str.total + value
        elseif stat == "vit" then
            stats.vit.total = stats.vit.total + value
        elseif stat == "mag" then
            stats.mag.total = stats.mag.total + value
        elseif stat == "spr" then
            stats.spr.total = stats.spr.total + value
        elseif stat == "dex" then
            stats.dex.total = stats.dex.total + value
        elseif stat == "lck" then
            stats.lck.total = stats.lck.total + value
        elseif stat == "hp" then
            stats.hp.max = stats.hp.max + (value * stats.hp.max / 100)
        elseif stat == "mp" then
            stats.mp.max = stats.mp.max + (value * stats.mp.max / 100)
        end
    end
    -- Armor bonus. HP and MP are percents of bsae, the rest absolutes.
    local armor = Game.Items[Characters[id].armor.id]
    for stat, value in ipairs(armor.stats) do
        if stat == "str" then
            stats.str.total = stats.str.total + value
        elseif stat == "vit" then
            stats.vit.total = stats.vit.total + value
        elseif stat == "mag" then
            stats.mag.total = stats.mag.total + value
        elseif stat == "spr" then
            stats.spr.total = stats.spr.total + value
        elseif stat == "dex" then
            stats.dex.total = stats.dex.total + value
        elseif stat == "lck" then
            stats.lck.total = stats.lck.total + value
        elseif stat == "hp" then
            stats.hp.max = stats.hp.max + (value * stats.hp.max / 100)
        elseif stat == "mp" then
            stats.mp.max = stats.mp.max + (value * stats.mp.max / 100)
        end
    end
    -- Accesory bonus
    local accessory = Game.Items[Characters[id].accessory]
    if (accessory ~= nil) then
        for stat, value in ipairs(armor.stats) do
            if stat == "str" then
                stats.str.total = stats.str.total + value
            elseif stat == "vit" then
                stats.vit.total = stats.vit.total + value
            elseif stat == "mag" then
                stats.mag.total = stats.mag.total + value
            elseif stat == "spr" then
                stats.spr.total = stats.spr.total + value
            elseif stat == "dex" then
                stats.dex.total = stats.dex.total + value
            elseif stat == "lck" then
                stats.lck.total = stats.lck.total + value
            elseif stat == "hp" then
                stats.hp.max = stats.hp.max + (value * stats.hp.max / 100)
            elseif stat == "mp" then
                stats.mp.max = stats.mp.max + (value * stats.mp.max / 100)
            end
        end
    end
    -- Get a list of materias
    local materias = {}
    for m = 0, 7 do
        if Characters[id].weapon.materia[m] ~= nil then
            table.insert(materias, Game.Materia[Characters[id].weapon.materia[m].id])
        end
        if Characters[id].armor.materia[m] ~= nil then
            table.insert(materias, Game.Materia[Characters[id].weapon.materia[m].id])
        end
    end
    -- Get materia bonuses. All are percents of base.
    for _, materia in ipairs(materias) do
        for stat, value in ipairs(materia.stats) do
            if stat == "str" then
                stats.str.total = stats.str.total + (value * stats.str.base / 100)
            elseif stat == "vit" then
                stats.vit.total = stats.vit.total + (value * stats.vit.base / 100)
            elseif stat == "mag" then
                stats.mag.total = stats.mag.total + (value * stats.mag.base / 100)
            elseif stat == "spr" then
                stats.spr.total = stats.spr.total + (value * stats.spr.base / 100)
            elseif stat == "dex" then
                stats.dex.total = stats.dex.total + (value * stats.dex.base / 100)
            elseif stat == "lck" then
                stats.lck.total = stats.lck.total + (value * stats.lck.base / 100)
            elseif stat == "hp" then
                stats.hp.max = stats.hp.max + (value * stats.hp.max / 100)
            elseif stat == "mp" then
                stats.mp.max = stats.mp.max + (value * stats.mp.max / 100)
            end
        end
    end
    -- Round down all basic stats
    stats.str.total = math.floor(stats.str.total)
    stats.vit.total = math.floor(stats.vit.total)
    stats.mag.total = math.floor(stats.mag.total)
    stats.spr.total = math.floor(stats.spr.total)
    stats.dex.total = math.floor(stats.dex.total)
    stats.lck.total = math.floor(stats.lck.total)
    stats.hp.max = math.floor(stats.hp.max)
    stats.mp.max = math.floor(stats.mp.max)

    if stats.str.total < 1 then
        stats.str.total = 1
    end
    if stats.vit.total < 1 then
        stats.vit.total = 1
    end
    if stats.mag.total < 1 then
        stats.mag.total = 1
    end
    if stats.spr.total < 1 then
        stats.spr.total = 1
    end
    if stats.dex.total < 1 then
        stats.dex.total = 1
    end
    if stats.lck.total < 1 then
        stats.lck.total = 1
    end
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
    if stats.eva < 1 then
        stats.eva = 1
    end
    if stats.matk < 1 then
        stats.matk = 1
    end
    if stats.mdef < 1 then
        stats.mdef = 1
    end
    if stats.meva < 1 then
        stats.meva = 1
    end

    -- TODO: If HP<->MP materia, invert values.

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
        print("Tried to equip invalid item: " .. tostring(character))
        return false
    elseif Inventory.get_item_quantity(item_id) < 1 then
        print("Tried to equip item not in inventory: " .. item_id .. " (" .. Game.Items[item.id].name .. ")")
        return false
    end
    local type = Game.Items[item_id].type
    if type ~= Inventory.ITEM_TYPE.WEAPON and type ~= Inventory.ITEM_TYPE.ARMOR and type ~= Inventory.ITEM_TYPE.ACCESSORY then
        print("Tried to equip non-equipable item: " .. item_id .. " (" .. Game.Items[item.id].name .. ")")
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
        print("Tried to equip item " .. tostring(item_id) .. " (" .. Game.Items[item.id].name .. ") on a non valid character: " .. tostring(char_id) .. " (" .. Characters[char_id].name .. ")")
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
        for m = 0, 7 do
            if Characters[char_id].weapon.materia[i] ~= nil and #(Game.Items[item_id].slots) < m then
                -- Materia to inventory
                Materia.add(Characters[char_id].weapon.materia[i].id, Characters[char_id].weapon.materia[i].ap)
                Characters[char_id].weapon.materia[i] = nil
            end
        end
        Inventory.add_item(Characters[char_id].weapon.id, 1)
        Characters[char_id].weapon.id = item_id
        Inventory.remove_item(item_id, 1)
    elseif type == Inventory.ITEM_TYPE.ARMOR then
        -- Armor. Remove excess materia, send previuos one to the inventory.
        for m = 0, 7 do
            if Characters[char_id].armor.materia[i] ~= nil and #(Game.Items[item_id].slots) < m then
                -- Materia to inventory
                Materia.add(Characters[char_id].armor.materia[i].id, Characters[char_id].armor.materia[i].ap)
                Characters[char_id].armor.materia[i] = nil
            end
        end
        Inventory.add_item(Characters[char_id].armor.id, 1)
        Characters[char_id].armor.id = item_id
        Inventory.remove_item(item_id, 1)
    else
        return false
    end
    return true
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
        print("Tried to add invalid materia \"" .. item .. "\".");
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
Party.choose_party = function()
    -- TODO: Implement
end

--- Special event. Removes all materia from the party, equipped and unequipped.
Party.steal_materia = function()
    -- TODO: Implement.
    -- TODO: How is this reversed?
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
    -- TODO: Validation
    local val = Banks[bank][address]
    return get_byte_nth_bit(val, bit)
end

--- Obtains a bit from an unsigned byte
--
-- @param value The value (0-254)
-- @param bit The bit to get (0-7)
-- @return The bit value (1-0)
function get_byte_nth_bit(value, n)
    if n < 0 or n > 7 then
        return 0
    end
    -- Convert to binary string
    local s = ""
    local temp = value
    for i = 1, 8 do
        s = s .. tostring(temp % 2)
        temp = math.floor(temp / 2)
    end
    print("S:" .. s)
    return tonumber(string.sub(s, n + 1, n + 1))
end
