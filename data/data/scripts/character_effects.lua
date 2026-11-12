--- Elemental and status effects a character gets from the equipment and the equipped materia.
--
-- Used by the status menu, and usable by anything that needs to know what a character resists or
-- inflicts.

--- Element IDs, as stored in the item and materia data.
Characters.ELEMENT = {
    FIRE = 0, ICE = 1, BOLT = 2, EARTH = 3, POISON = 4, GRAVITY = 5, WATER = 6, WIND = 7,
    HOLY = 8, RESTORATIVE = 9, CUT = 10, HIT = 11, PUNCH = 12, SHOOT = 13, SHOUT = 14, HIDDEN = 15,
}

--- How an element affects a character. Lower is stronger.
Characters.ELEMENT_MODE = {
    ABSORB = 0,
    NULLIFY = 1,
    HALF = 2,
}

--- Status IDs, as stored in the item and materia data (bit position in the kernel).
Characters.STATUS = {
    DEATH = 0, NEAR_DEATH = 1, SLEEP = 2, POISON = 3, SADNESS = 4, FURY = 5, CONFUSION = 6,
    SILENCE = 7, HASTE = 8, SLOW = 9, STOP = 10, FROG = 11, SMALL = 12, SLOW_NUMB = 13,
    PETRIFY = 14, REGEN = 15, BARRIER = 16, MBARRIER = 17, REFLECT = 18, DUAL = 19, SHIELD = 20,
    D_SENTENCE = 21, MANIPULATE = 22, BERSERK = 23, PEERLESS = 24, PARALYZED = 25, DARKNESS = 26,
}

--- Element used in the materia data for "no element".
local NO_ELEMENT = Characters.ELEMENT.HIDDEN

--- Materia level (1-based) for an amount of AP.
local function materia_level(definition, ap)
    local level = 0
    for _, level_ap in ipairs(definition.levels_ap) do
        if level_ap > ap then break end
        level = level + 1
    end
    return math.max(level, 1)
end

--- Mode an Elemental materia gives to armor, by materia level.
local function elemental_armor_mode(level)
    if level >= 3 then return Characters.ELEMENT_MODE.ABSORB end
    if level == 2 then return Characters.ELEMENT_MODE.NULLIFY end
    return Characters.ELEMENT_MODE.HALF
end

--- Adds an element resistance, keeping the strongest one.
local function add_resistance(defense, element, mode)
    if mode == nil or mode > Characters.ELEMENT_MODE.HALF then return end
    if defense[element] == nil or mode < defense[element] then
        defense[element] = mode
    end
end

--- Gets the materia linked to a slot.
--
-- @param item The weapon or armor definition.
-- @param slot Slot of the item, 1-8.
-- @return List of slots (1-8) linked to it.
local function linked_slots(item, slot)
    local linked = {}
    -- slots[n] == 1 means that slot n is linked to slot n + 1.
    if item.slots[slot] == 1 and #(item.slots) > slot then table.insert(linked, slot + 1) end
    if slot > 1 and item.slots[slot - 1] == 1 then table.insert(linked, slot - 1) end
    return linked
end

--- Applies the link materia (Elemental, Added Effect) of a weapon or armor.
local function apply_link_materia(char_id, part, result)
    local equipment = Characters[char_id][part]
    local item = Game.Items[equipment.id]
    if item == nil or item.slots == nil then return end
    for slot = 1, #(item.slots) do
        local equipped = equipment.materia[slot - 1]
        local definition = equipped ~= nil and Game.Materia[equipped.id] or nil
        if definition ~= nil and definition.link ~= nil then
            local link = definition.link.id
            for _, partner_slot in ipairs(linked_slots(item, slot)) do
                local partner = equipment.materia[partner_slot - 1]
                local partner_def = partner ~= nil and Game.Materia[partner.id] or nil
                if partner_def ~= nil then
                    if link == Materia.LINK.ELEMENTAL then
                        for _, element in ipairs(partner_def.elements or {}) do
                            if element ~= NO_ELEMENT then
                                if part == "weapon" then
                                    result.elements.attack[element] = true
                                else
                                    add_resistance(
                                        result.elements.defense, element,
                                        elemental_armor_mode(materia_level(definition, equipped.ap))
                                    )
                                end
                            end
                        end
                    elseif link == Materia.LINK.ADDED_EFFECT then
                        local target = (part == "weapon") and result.status.attack or result.status.defense
                        for _, status in ipairs(partner_def.status or {}) do
                            target[status] = true
                        end
                    end
                end
            end
        end
    end
end

--- Calculates the elemental and status effects of a character.
--
-- Takes into account the weapon, the armor, the accessory and the linked materia:
--  * Weapon elements, and Elemental materia linked in the weapon: elements the attacks have.
--  * Armor and accessory elements: resistances. Elemental materia linked in the armor gives half
--    damage (level 1), nullifies (level 2) or absorbs (level 3 or higher).
--  * Added Effect materia: weapon statuses are inflicted, armor statuses are protected against.
--  * Accessory statuses: protected against.
--
-- @param char_id ID of the character.
-- @return A table:
--     elements = {attack = {[element] = true}, defense = {[element] = mode}}
--     status = {attack = {[status] = true}, defense = {[status] = true}}
Characters.get_effects = function(char_id)
    local result = {
        elements = {attack = {}, defense = {}},
        status = {attack = {}, defense = {}},
    }
    local character = Characters[char_id]
    if character == nil then return result end

    -- Weapon
    local weapon = Game.Items[character.weapon.id]
    if weapon ~= nil then
        for _, element in ipairs(weapon.elements or {}) do
            result.elements.attack[element] = true
        end
    end

    -- Armor
    local armor = Game.Items[character.armor.id]
    if armor ~= nil then
        for _, element in ipairs(armor.elements or {}) do
            add_resistance(result.elements.defense, element.id, element.mode)
        end
    end

    -- Accessory
    local accessory = character.accessory ~= nil and Game.Items[character.accessory] or nil
    if accessory ~= nil then
        for _, element in ipairs(accessory.elements or {}) do
            if type(element) == "table" then
                add_resistance(result.elements.defense, element.id, element.mode)
            else
                -- Older data without a mode.
                add_resistance(result.elements.defense, element, Characters.ELEMENT_MODE.NULLIFY)
            end
        end
        for _, status in ipairs(accessory.status or {}) do
            result.status.defense[status] = true
        end
    end

    -- Materia
    apply_link_materia(char_id, "weapon", result)
    apply_link_materia(char_id, "armor", result)

    return result
end
