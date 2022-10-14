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
    if FFVII.Party[1] == nil then
        c1 = -1
    else
        c1 = FFVII.Party[1]
    end
    if FFVII.Party[2] == nil then
        c2 = -1
    else
        c2 = FFVII.Party[2]
    end
    if FFVII.Party[3] == nil then
        c3 = -1
    else
        c3 = FFVII.Party[3]
    end
    text_manager:set_party(c1, c2, c3)
end

--- Sets the current party.
--
-- @param character1 The first member character ID, the party leader.
-- @param character2 The second member character ID, can be nil.
-- @param character3 The third member character ID, can be nil.
FFVII.set_party = function(character1, character2, character3)
    FFVII.Party[1] = character1
    FFVII.Party[2] = character2
    FFVII.Party[3] = character3
    export_party_members()
end

--- Checks if a a character ID actually belongs to a character.
--
-- @param character The ID to check.
-- @return True if the ID is a valid character ID, false otherwise.
FFVII.is_character = function(character)
    if FFVII.Characters[character] == nil then
        return false
    end
    return true
end

--- Adds a character to the curent party.
--
-- @param character ID of the character to add.
FFVII.add_party_character = function(character)
    if FFVII.Characters[character] == nil then
        print("Tried add to party invalid character \"" .. character .. "\".");
        return
    end
    if FFVII.Characters[character].party_lock == true then
        print("Tried add to party locked character \"" .. character .. "\".");
        return
    end
    -- Check if character already in party
    for x = 1, 3 do
        if FFVII.Party[i] == character then
            print("Character \"" .. character .. "\" already in party.");
            return
        end
    end
    -- Try to add in first free slot.
    for x = 1, 3 do
        if FFVII.Party[i] == nil then
            FFVII.Party[i] = character
            export_party_members()
            return
        end
    end
end

--- Removes a character from the curent party.
--
-- @param character ID of the character to remove.
FFVII.remove_party_character = function(character)
    if FFVII.Characters[character] == nil then
        print("Tried remove from party invalid character \"" .. character .. "\".");
        return
    end
    if FFVII.Characters[character].party_lock == true then
        print("Tried remove from party locked character \"" .. character .. "\".");
        return
    end
    for x = 1, 3 do
        if FFVII.Party[i] == character then
            FFVII.Party[i] = nil
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
FFVII.set_party_lock = function(character, lock)
    if FFVII.Characters[character] == nil then
        print("Tried set/unset party lock to invalid character \"" .. character .. "\".");
        return
    end
    FFVII.Characters[character].party_lock = lock;
end

--- Checks if a character is locked in the party.
--
-- Locked characters can't be manually  swapped in or out of the party.
--
-- @param character ID of the character to check for locks.
FFVII.get_party_lock = function(character)
    if FFVII.Characters[character] == nil then
        print("Tried get party lock from invalid character \"" .. character .. "\".");
        return false
    end
    return FFVII.Characters[character].party_lock
end

--- Enables or disables a character is the PHS.
--
-- Disabled characters don't appear in the PHS.
--
-- @param character ID of the character to enable or disable.
-- @param enable True to enable, false to disable.
FFVII.set_character_enable = function(character, enable)
    if FFVII.Characters[character] == nil then
        print("Tried enable/disable invalid character \"" .. character .. "\".");
        return
    end
    FFVII.Characters[character].enable = enable;
end

--- Checks if a character is enabled in the PHS.
--
-- Disabled characters don't appear in the PHS.
--
-- @param character ID of the character to check.
-- @return True if enabled, false if disables.
FFVII.get_character_enable = function(character)
    if FFVII.Characters[character] == nil then
        print("Tried get enabled from invalid character \"" .. character .. "\".");
        return false
    end
    return FFVII.Characters[character].enable;
end

--- Adds items to the inventory.
--
-- Inventory quantities will be capped at 99.
--
-- @param item Item ID.
-- @param quantity Quantity to add.
FFVII.add_item = function(item, quantity)
    if FFVII.Items[item] == nil then
        print("Tried add invalid item \"" .. item .. "\".");
        return
    end
    local old_quantity = 0
    if FFVII.ItemStorage[item] ~= nil then
        old_quantity = FFVII.ItemStorage[item]
    end
    FFVII.ItemStorage[item] = old_quantity + quantity
    if FFVII.ItemStorage[item] > 99 then
        FFVII.ItemStorage[item] = 99
    end
end

--- Checks how many items of a type are in the inventory.
--
-- @param item The item ID.
-- @return Number of the items in the inventory.
FFVII.get_item_quantity = function(item)
    if FFVII.Items[item] == nil then
        print("Tried get quontity for invalid item \"" .. item .. "\".");
        return
    end
    local quantity = 0
    if FFVII.ItemStorage[item] ~= nil then
        quantity = FFVII.ItemStorage[item]
    end
    return quantity
end

--- Removes items from the inventory.
--
-- Negative quantities can't remain in the inventory. If there are less than requested to remove,
-- all of them will be removed.
--
-- @param item Item ID.
-- @param quantity Quantity to remove.
FFVII.remove_item = function(item, quantity)
    if FFVII.Items[item] == nil then
        print("Tried remove invalid item \"" .. item .. "\".");
        return
    end
    local old_quantity = 0
    if FFVII.ItemStorage[item] ~= nil then
        old_quantity = FFVII.ItemStorage[item]
    end
    FFVII.ItemStorage[item] = old_quantity - quantity
    if FFVII.ItemStorage[item] < 0 then
        FFVII.ItemStorage[item] = 0
    end
end

--- Adds money.
--
-- @param amount The money to add.
-- @todo Fix cap.
FFVII.add_money = function(amount)
    FFVII.Savemap.money = FFVII.Savemap.money + amount

    if FFVII.Savemap.money > 999999 then
        FFVII.ItemStorage[item] = 999999
    end
end

--- Checks the curent money.
--
-- @return Current money.
FFVII.get_money = function()
    return FFVII.Savemap.money
end

--- Removes money.
--
-- Negative quantities can't remain as money. If there are less than requested to remove, all of
-- it will be removed.
--
-- @param amount Money to remove.
FFVII.remove_money = function(amount)
    FFVII.Savemap.money = FFVII.Savemap.money - amount
    if FFVII.Savemap.money < 0 then
        FFVII.Savemap.money = 0
    end
end
