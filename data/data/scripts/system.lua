MAX_INVENTORY_SLOTS = 320

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
Inventory.add_item = function(item, quantity)
    -- TODO: Also check weapons, armors and accessories.
    if Game.Items[item] == nil then
        print("Tried add invalid item \"" .. item .. "\".");
        return
    end
    for i = 0, MAX_INVENTORY_SLOTS do
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
    for i = 0, MAX_INVENTORY_SLOTS do
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
    for i = 0, MAX_INVENTORY_SLOTS do
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
    for i = 0, MAX_INVENTORY_SLOTS do
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
