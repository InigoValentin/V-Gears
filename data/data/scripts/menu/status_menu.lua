if UiContainer == nil then UiContainer = {} end

local ROOT = "StatusMenu.Container."

--- Inside width of the bars of the header.
local BAR_WIDTH = 54

--- Colour of the names of the elements and statuses that apply, and of those that don't.
local COLOUR_ACTIVE = { 1.0, 1.0, 1.0 }
local COLOUR_INACTIVE = { 0.45, 0.45, 0.35 }

--- Widgets of the first page with a number, and where its value comes from.
local NUMBERS = {
    { "Page1.Attributes.Str", function(c) return c.stats.str.total end },
    { "Page1.Attributes.Dex", function(c) return c.stats.dex.total end },
    { "Page1.Attributes.Vit", function(c) return c.stats.vit.total end },
    { "Page1.Attributes.Mag", function(c) return c.stats.mag.total end },
    { "Page1.Attributes.Spr", function(c) return c.stats.spr.total end },
    { "Page1.Attributes.Lck", function(c) return c.stats.lck.total end },
    { "Page1.Attributes.Atk", function(c) return c.stats.atk end },
    { "Page1.Attributes.AtkPct", function(c) return c.stats.acc end },
    { "Page1.Attributes.Def", function(c) return c.stats.def end },
    { "Page1.Attributes.DefPct", function(c) return c.stats.eva end },
    { "Page1.Attributes.MAtk", function(c) return c.stats.matk end },
    { "Page1.Attributes.MDef", function(c) return c.stats.mdef end },
    { "Page1.Attributes.MDefPct", function(c) return c.stats.meva end },
    { "Character.Exp", function(c) return c.exp end, 8 },
    { "Character.Next", function(c) return c.exp_to_next end, 8 },
    { "Character.LimitLevel", function(c) return c.limit.current end, 2 },
}

--- Elements of the second page, in the order of the widgets.
local ELEMENTS = {
    { Characters.ELEMENT.FIRE, "Fire" }, { Characters.ELEMENT.ICE, "Ice" },
    { Characters.ELEMENT.BOLT, "Lightning" }, { Characters.ELEMENT.EARTH, "Earth" },
    { Characters.ELEMENT.POISON, "Poison" }, { Characters.ELEMENT.GRAVITY, "Gravity" },
    { Characters.ELEMENT.WATER, "Water" }, { Characters.ELEMENT.WIND, "Wind" },
    { Characters.ELEMENT.HOLY, "Holy" },
}

--- Statuses of the third page, in the order of the widgets.
local STATUSES = {
    { Characters.STATUS.DEATH, "Death" }, { Characters.STATUS.NEAR_DEATH, "Near-death" },
    { Characters.STATUS.SLEEP, "Sleep" }, { Characters.STATUS.POISON, "Poison" },
    { Characters.STATUS.SADNESS, "Sadness" }, { Characters.STATUS.FURY, "Fury" },
    { Characters.STATUS.CONFUSION, "Confusion" }, { Characters.STATUS.SILENCE, "Silence" },
    { Characters.STATUS.HASTE, "Haste" }, { Characters.STATUS.SLOW, "Slow" },
    { Characters.STATUS.STOP, "Stop" }, { Characters.STATUS.FROG, "Frog" },
    { Characters.STATUS.SMALL, "Small" }, { Characters.STATUS.SLOW_NUMB, "Slow-numb" },
    { Characters.STATUS.PETRIFY, "Petrify" }, { Characters.STATUS.REGEN, "Regen" },
    { Characters.STATUS.BARRIER, "Barrier" }, { Characters.STATUS.MBARRIER, "MBarrier" },
    { Characters.STATUS.REFLECT, "Reflect" }, { Characters.STATUS.SHIELD, "Shield" },
    { Characters.STATUS.D_SENTENCE, "Death-sentence" }, { Characters.STATUS.MANIPULATE, "Manipulate" },
    { Characters.STATUS.BERSERK, "Berserk" }, { Characters.STATUS.PEERLESS, "Peerless" },
    { Characters.STATUS.PARALYZED, "Paralyzed" }, { Characters.STATUS.DARKNESS, "Darkness" },
}

--- Sets a name, white if it applies, gray otherwise.
local function set_name(path, name, active)
    local widget = ui_manager:get_widget(path)
    local colour = active and COLOUR_ACTIVE or COLOUR_INACTIVE
    -- The text is set after the colour to apply it.
    widget:set_colour(colour[1], colour[2], colour[3])
    widget:set_text(name)
end

--- The status menu. Purely informative.
--
-- It has three pages that share the character data: attributes and equipment, elemental effects,
-- and status effects. OK goes to the next page, looping, and Cancel closes the menu.
UiContainer.StatusMenu = UiContainer.Menu.new{

    id = "status",
    parent = "main",

    --- ID of the character being shown.
    char_id = -1,

    --- Page being shown (1-3).
    page = 1,

    --- Total number of pages.
    page_total = 3,

    states = {
        [""] = {
            confirm = function(self)
                self:show_page(self.page % self.page_total + 1)
            end,
            cancel = function(self)
                script:request_end_sync(Script.UI, "StatusMenu", "hide", 0)
            end,
        },
    },

    --- Shows a page and hides the others.
    show_page = function(self, page)
        self.page = page
        for p = 1, self.page_total do
            ui_manager:get_widget(ROOT .. "Page" .. tostring(p)):set_visible(p == page)
        end
    end,

    --- Shows the data shared by every page.
    populate_header = function(self, character)
        UiContainer.populate_character_data(ROOT .. "Character", character)
        ui_manager:get_widget(ROOT .. "Character.Portrait"):set_image("characters/" .. tostring(self.char_id) .. ".png")
        UiContainer.set_bar(ROOT .. "Character.NextLevelLinePart", UiContainer.exp_progress(character), BAR_WIDTH)
        UiContainer.set_limit_bar(ROOT .. "Character.LimitLinePart", character, BAR_WIDTH)
    end,

    --- Shows the slots and materia of a weapon or armor.
    --
    -- @param prefix Prefix of the widgets: "Wpn" or "Arm".
    -- @param item Definition of the weapon or armor.
    -- @param materia Materia equipped on it. Slots 0-7.
    populate_slots = function(self, prefix, item, materia)
        local path = ROOT .. "Page1.Equipment." .. prefix .. ".Slots."
        for slot = 1, 8 do
            ui_manager:get_widget(path .. "Slot" .. tostring(slot)):set_visible(#(item.slots) >= slot)
            local equipped = materia[slot - 1]
            local widget = ui_manager:get_widget(path .. "Mat" .. tostring(slot))
            if equipped ~= nil and Game.Materia[equipped.id] ~= nil then
                widget:set_image("icons/materia_" .. tostring(Game.Materia[equipped.id].type) .. ".png")
                widget:set_visible(true)
            else
                widget:set_visible(false)
            end
            -- At every odd slot, if there is a next slot and it's linked, show the linker.
            if slot % 2 ~= 0 then
                local linked = #(item.slots) > slot and item.slots[slot] == 1
                ui_manager:get_widget(path .. "Link" .. tostring(math.floor(slot / 2) + 1)):set_visible(linked)
            end
        end
    end,

    --- Shows the attributes and the equipment.
    populate_page_1 = function(self, character)
        for _, entry in ipairs(NUMBERS) do
            local value = entry[2](character)
            ui_manager:get_widget(ROOT .. entry[1]):set_text(UiContainer.pad_value(value or 0, entry[3] or 4))
        end

        local equipment = ROOT .. "Page1.Equipment."
        local weapon = Game.Items[character.weapon.id]
        local armor = Game.Items[character.armor.id]
        ui_manager:get_widget(equipment .. "Wpn.Lbl"):set_text(weapon.name)
        ui_manager:get_widget(equipment .. "Arm.Lbl"):set_text(armor.name)
        if character.accessory ~= nil then
            ui_manager:get_widget(equipment .. "Acc.Lbl"):set_text(Game.Items[character.accessory].name)
        else
            ui_manager:get_widget(equipment .. "Acc.Lbl"):set_text("")
        end
        ui_manager:get_widget(equipment .. "Wpn.Icon"):set_image("icons/item_weapon_" .. tostring(self.char_id) .. ".png")
        self:populate_slots("Wpn", weapon, character.weapon.materia)
        self:populate_slots("Arm", armor, character.armor.materia)
    end,

    --- Shows the elements the attacks have, and the elements that are halved, nullified or
    -- absorbed.
    populate_page_2 = function(self, effects)
        local mode = Characters.ELEMENT_MODE
        local groups = {
            { "Attack", function(element) return effects.elements.attack[element] == true end },
            { "Halve", function(element) return effects.elements.defense[element] == mode.HALF end },
            { "Invalid", function(element) return effects.elements.defense[element] == mode.NULLIFY end },
            { "Absorb", function(element) return effects.elements.defense[element] == mode.ABSORB end },
        }
        for _, group in ipairs(groups) do
            for index, element in ipairs(ELEMENTS) do
                set_name(ROOT .. "Page2.Table." .. group[1] .. ".Name" .. tostring(index), element[2], group[2](element[1]))
            end
        end
    end,

    --- Shows the statuses the attacks inflict, and the ones the character is protected from.
    populate_page_3 = function(self, effects)
        local groups = {
            { "Attack", effects.status.attack },
            { "Defend", effects.status.defense },
        }
        for _, group in ipairs(groups) do
            for index, status in ipairs(STATUSES) do
                set_name(ROOT .. "Page3.Table." .. group[1] .. ".Name" .. tostring(index), status[2], group[2][status[1]] == true)
            end
        end
    end,

    --- Shows the data of the current character in every page.
    populate = function(self)
        local character = Characters[self.char_id]
        local effects = Characters.get_effects(self.char_id)
        self:populate_header(character)
        self:populate_page_1(character)
        self:populate_page_2(effects)
        self:populate_page_3(effects)
    end,

    --- Opens the status menu for UiContainer.current_character_id.
    show = function(self)
        self.char_id = UiContainer.current_character_id
        self:activate()
        self:populate()
        self:show_page(1)
        ui_manager:get_widget("StatusMenu"):set_visible(true)
        return 0
    end,

    --- Hides the status menu and goes back to the main menu.
    hide = function(self)
        ui_manager:get_widget("StatusMenu"):set_visible(false)
        self:deactivate()
        return 0
    end,
}
