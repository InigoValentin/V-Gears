--- Abilities of the characters that come from their equipped materia.

--- Command of the Enemy Skill materia (see Game.Commands).
Characters.E_SKILL_COMMAND = 13

--- ID of the attack of the first enemy skill. There are 24, in the order of the Enemy Skill materia.
Characters.FIRST_E_SKILL_ATTACK = 72

--- Gets the level of a materia given its AP.
--
-- @param materia Definition of the materia (an entry of Game.Materia).
-- @param ap AP of the materia.
-- @return The level (1 or more), or 0 if the AP are less than needed for the first level.
local function get_materia_level(materia, ap)
    local level = 0
    for l = 1, #materia.levels_ap do
        if materia.levels_ap[l] <= ap then level = l end
    end
    return level
end

--- Gets the magic, summons and enemy skills a character can use with the materia that is equipped.
--
-- Every ability is listed once, even if more than one materia gives it.
--
-- @param char_id ID of the character.
-- @return A table with these lists, sorted by attack ID:
--  * magic: Attack IDs of the spells.
--  * magic_groups: The same spells, in lists with the ones that come from the same materia. The
--    groups are sorted by their first spell.
--  * summon: Tables {id = attack ID, times = times it can be used per battle (255 is no limit)}.
--  * e_skill: Attack IDs of the enemy skills that were learned.
Characters.get_abilities = function(char_id)
    local result = { magic = {}, magic_groups = {}, summon = {}, e_skill = {} }
    local character = Characters[char_id]
    if character == nil then return result end

    local magic_seen = {}
    local e_skill_seen = {}
    local summon_by_id = {}

    for _, part in ipairs({ "weapon", "armor" }) do
        local equipment = character[part]
        -- Slots are 0-7.
        for slot = 0, 7 do
            local equipped = equipment.materia[slot]
            local materia = (equipped ~= nil) and Game.Materia[equipped.id] or nil
            if materia ~= nil then
                local level = get_materia_level(materia, equipped.ap or 0)
                if materia.type == Materia.TYPE.MAGIC and materia.magic ~= nil then
                    local group = {}
                    for spell_level, spell in ipairs(materia.magic) do
                        if level >= spell_level and not magic_seen[spell] then
                            magic_seen[spell] = true
                            result.magic[#result.magic + 1] = spell
                            group[#group + 1] = spell
                        end
                    end
                    if #group > 0 then result.magic_groups[#result.magic_groups + 1] = group end
                elseif materia.type == Materia.TYPE.SUMMON and materia.summon ~= nil and level > 0 then
                    local times = materia.times[level] or 1
                    local existing = summon_by_id[materia.summon]
                    if existing == nil then
                        existing = { id = materia.summon, times = times }
                        summon_by_id[materia.summon] = existing
                        result.summon[#result.summon + 1] = existing
                    elseif times > existing.times then
                        existing.times = times
                    end
                elseif materia.type == Materia.TYPE.COMMAND and materia.command ~= nil then
                    for _, command in ipairs(materia.command) do
                        if command == Characters.E_SKILL_COMMAND and equipped.skills ~= nil then
                            for skill = 1, 24 do
                                local attack = Characters.FIRST_E_SKILL_ATTACK + skill - 1
                                if equipped.skills[skill] == true and not e_skill_seen[attack] then
                                    e_skill_seen[attack] = true
                                    result.e_skill[#result.e_skill + 1] = attack
                                end
                            end
                        end
                    end
                end
            end
        end
    end

    table.sort(result.magic)
    table.sort(result.magic_groups, function(a, b) return a[1] < b[1] end)
    table.sort(result.e_skill)
    table.sort(result.summon, function(a, b) return a.id < b.id end)
    return result
end

--- Status of death (see BattleStatus). The characters with no HP are dead.
local STATUS_DEATH = 0

--- Checks if an attack revives (cures the death status).
local function revives(attack)
    for _, effect in ipairs(attack.status or {}) do
        -- Mode 1 is cure.
        if effect.status == STATUS_DEATH and effect.mode == 1 then return true end
    end
    return false
end

--- Gets the maximum of a stat (HP or MP) of a character.
-- @param stat The stat, with the fields max (if it was calculated) and base.
local function get_stat_max(stat)
    return stat.max or stat.base
end

--- Casts a spell from the menu, out of battle: the cure and life families.
--
-- The spell heals (or revives) the target and the caster pays the MP. If the spell can't be cast
-- (not enough MP, the target is dead for a cure, alive for a life, or already has all its HP), it
-- does nothing and costs nothing.
--
-- The amounts are the ones of the battle: cures heal the magic base times the power / 16, and life
-- spells a fraction of the maximum HP (power / 32).
--
-- @param caster_id ID of the character that casts.
-- @param attack_id ID of the attack (spell).
-- @param target_id ID of the character that receives it.
-- @return True if it was cast, and the HP that were restored.
Characters.cast_menu_spell = function(caster_id, attack_id, target_id)
    local attack = Game.Attacks[attack_id]
    local caster = Characters[caster_id]
    local target = Characters[target_id]
    if attack == nil or caster == nil or target == nil then return false end

    local cost = attack.mp or 0
    if caster.stats.mp.current < cost then return false end

    local hp = target.stats.hp
    local hp_max = get_stat_max(hp)
    local dead = hp.current <= 0
    local power = attack.power or 0
    local amount = 0

    if revives(attack) then
        -- Life: only works on dead characters. A quarter of the HP if the power is unknown.
        if not dead then return false end
        amount = hp_max * ((power > 0) and power or 8) / 32
    elseif attack.dmg_formula == 2 and attack.dmg_modifier == 5 then
        -- Cure: doesn't work on dead characters, or on the ones with all their HP.
        if dead or hp.current >= hp_max then return false end
        local mag = caster.stats.mag.total or caster.stats.mag.base or 1
        local level = caster.level or 1
        local base = 6 * (mag + ((mag + level) / 32) * ((mag * level) / 32))
        amount = base * power / 16 * (15 + math.random()) / 16
    else
        return false
    end

    amount = math.max(1, math.min(math.floor(amount), hp_max - hp.current))
    hp.current = hp.current + amount
    caster.stats.mp.current = caster.stats.mp.current - cost
    return true, amount
end
