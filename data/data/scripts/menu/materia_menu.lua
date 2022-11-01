if UiContainer == nil then UiContainer = {} end

--- The item menu.
UiContainer.MateriaMenu = {

    --- Current header cursor row (1: Check/Weapon, 2: Order/Armor).
    header_position_row = 1,

    --- Current header cursor position (1: Check/Order, 2-9: Weapon/Armor slots).
    header_position = 1,

    --- Current header cursor max row.
    header_position_row_total = 2,

    --- Current header cursor max position.
    header_position_total = 9,

    --- ID of the character the menu is open for.
    char_id = -1,

    list_position = 1,

    list_position_total = 10,

    list_first_visible = 1,

    --- Run when the menu is creaded.
    --
    -- It does nothing.
    on_start = function(self)
        return 0
    end,

    --- Handles button events.
    --
    -- For the current submenu, handles directional keys, enter and escape events.
    -- @param button Pressed button string key. "Up", "Left", "Enter" and "Escape" are handled.
    -- @param event Trigger event. Normally, "Press".
    on_button = function(self, button, event)
        if UiContainer.current_menu == "materia" then
            if UiContainer.current_submenu == "" then
                -- TODO: Handle L1/R1 (change character)
                -- TODO: Handle square: Materia menu
                if button == "Escape" and event == "Press" then
                    UiContainer.current_menu = "main"
                    script:request_end_sync(Script.UI, "MateriaMenu", "hide", 0)
                elseif button == "Down" then
                    self.header_position_row = self.header_position_row + 1
                    if self.header_position_row > self.header_position_row_total then
                        self.header_position_row = 1
                    end
                    ui_manager:get_widget("MateriaMenu.Container.Character.Cursor"):set_default_animation("Position" .. self.header_position_row .. "-" .. self.header_position)
                    self.populate_details(self, false)
                elseif button == "Up" then
                    self.header_position_row = self.header_position_row - 1
                    if self.header_position_row < 1 then
                        self.header_position_row = self.header_position_row_total
                    end
                    ui_manager:get_widget("MateriaMenu.Container.Character.Cursor"):set_default_animation("Position" .. self.header_position_row .. "-" .. self.header_position)
                    self.populate_details(self, false)
                elseif button == "Right" then
                    self.header_position = self.header_position + 1
                    if self.header_position > self.header_position_total then
                        self.header_position = 1
                    end
                    ui_manager:get_widget("MateriaMenu.Container.Character.Cursor"):set_default_animation("Position" .. self.header_position_row .. "-" .. self.header_position)
                    self.populate_details(self, false)
                elseif button == "Left" then
                    self.header_position = self.header_position - 1
                    if self.header_position < 1 then
                        self.header_position = self.header_position_total
                    end
                    ui_manager:get_widget("MateriaMenu.Container.Character.Cursor"):set_default_animation("Position" .. self.header_position_row .. "-" .. self.header_position)
                    self.populate_details(self, false)
                elseif button == "Enter" then
                    --[[if self.equip_position == 1 then
                        self.selecting_slot = Inventory.ITEM_TYPE.WEAPON
                    elseif self.equip_position == 2 then
                        self.selecting_slot = Inventory.ITEM_TYPE.ARMOR
                    elseif self.equip_position == 3 then
                        self.selecting_slot = Inventory.ITEM_TYPE.ACCESSORY
                    end
                    self.submenu_select(self)]]
                -- TODO: Triangle->Remove
                else
                    return 0
                end
            end
        end
        return 0
    end,

    --- Opens the materia menu.
    --
    -- Populates and updates displayed data before opening.
    show = function(self)
        self.char_id = UiContainer.current_character_id
        ui_manager:get_widget("MateriaMenu"):set_visible(true)
        UiContainer.current_menu = "materia"
        UiContainer.current_submenu = ""
        self.header_position_row = 1
        self.header_position = 1
        self.submenu_none(self)
        return 0;
    end,

    --- Enters or returns to the main equip menu.
    submenu_none = function(self)
        UiContainer.current_submenu = ""
        UiContainer.populate_character_data("MateriaMenu.Container.Character", Characters[self.char_id])
        ui_manager:get_widget("MateriaMenu.Container.Character.Portrait"):set_image("images/characters/" .. tostring(self.char_id) .. ".png")
        self.populate_equip(self)
        -- TODO: Do something for chars 9 and 10


        --[[self.load_availables(self)
        self.populate_details(self, false)
        self.populate_current_stats(self)
        self.calculate_stat_diffs(self)
        -- Hide stat arrows and difs
        ui_manager:get_widget("EquipMenu.Container.Stats.AtkDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.AccDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.DefDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.EvaDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MAtkDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MDefDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MEvaDif"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.AtkArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.AccArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.DefArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.EvaArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MAtkArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MDefArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.Stats.MEvaArw"):set_visible(false)
        ui_manager:get_widget("EquipMenu.Container.List.Cursor"):set_visible(false)
        for i = 1, self.list_position_total do
            ui_manager:get_widget("EquipMenu.Container.List.Item" .. tostring(i)):set_text("")
        end]]
    end,

    populate_equip = function(self)
        local weapon = Game.Items[Characters[self.char_id].weapon.id]
        local armor = Game.Items[Characters[self.char_id].armor.id]
        ui_manager:get_widget("MateriaMenu.Container.Character.WpnLbl"):set_text(weapon.name)
        ui_manager:get_widget("MateriaMenu.Container.Character.ArmLbl"):set_text(armor.name)
        for i = 1, 8 do
            -- Show/hide slots
            -- TODO: Change image for no-growth slots?
            if #(weapon.slots) >= i then
                ui_manager:get_widget("MateriaMenu.Container.Character.WeaponSlots.Slot" .. tostring(i)):set_visible(true)
            else
                ui_manager:get_widget("MateriaMenu.Container.Character.WeaponSlots.Slot" .. tostring(i)):set_visible(false)
            end
            if #(armor.slots) >= i then
                ui_manager:get_widget("MateriaMenu.Container.Character.ArmorSlots.Slot" .. tostring(i)):set_visible(true)
            else
                ui_manager:get_widget("MateriaMenu.Container.Character.ArmorSlots.Slot" .. tostring(i)):set_visible(false)
            end
            -- Show/hide linkers
            if i % 2 ~= 0 then
                if #(weapon.slots) > i  and weapon.slots[i] == 1 then
                    ui_manager:get_widget("MateriaMenu.Container.Character.WeaponSlots.Link" .. tostring(math.floor(i / 2) + 1)):set_visible(true)
                else
                    ui_manager:get_widget("MateriaMenu.Container.Character.WeaponSlots.Link" .. tostring(math.floor(i / 2) + 1)):set_visible(false)
                end
                if #(armor.slots) > i  and armor.slots[i] == 1 then
                    ui_manager:get_widget("MateriaMenu.Container.Character.ArmorSlots.Link" .. tostring(math.floor(i / 2) + 1)):set_visible(true)
                else
                    ui_manager:get_widget("MateriaMenu.Container.Character.ArmorSlots.Link" .. tostring(math.floor(i / 2) + 1)):set_visible(false)
                end
            end
            -- Show/hide/set materia
            if Characters[self.char_id].weapon.materia[i - 1] ~= nil then
                local mat_type =  Game.Materia[Characters[self.char_id].weapon.materia[i - 1].id].type
                ui_manager:get_widget("MateriaMenu.Container.Character.WeaponSlots.Mat" .. tostring(i)):set_image("images/icons/materia_" .. tostring(mat_type) .. ".png")
                ui_manager:get_widget("MateriaMenu.Container.Character.WeaponSlots.Mat" .. tostring(i)):set_visible(true)
            else
                ui_manager:get_widget("MateriaMenu.Container.Character.WeaponSlots.Mat" .. tostring(i)):set_visible(false)
            end
            if Characters[self.char_id].armor.materia[i - 1] ~= nil then
                local mat_type =  Game.Materia[Characters[self.char_id].armor.materia[i - 1].id].type
                ui_manager:get_widget("MateriaMenu.Container.Character.ArmorSlots.Mat" .. tostring(i)):set_image("images/icons/materia_" .. tostring(mat_type) .. ".png")
                ui_manager:get_widget("MateriaMenu.Container.Character.ArmorSlots.Mat" .. tostring(i)):set_visible(true)
            else
                ui_manager:get_widget("MateriaMenu.Container.Character.ArmorSlots.Mat" .. tostring(i)):set_visible(false)
            end
        end
    end,

    --- Hiddes the materia details.
    --
    -- Must be called when the cursor goes into an empty slot
    hide_details = function(self)
        -- TODO
    end,

    --- Show details of an enemy skill materia.
    --
    -- Must be called when the cursor goes into an empty slot
    populate_e_skill_details = function(self)
        -- TODO
    end,

    --- Populated the details of the selected header item (check, materia, or none).
    --
    -- @param from_list If true, data will be read from the item higlighted in the list on the
    -- right. If false, data wil be red from the currently equipped items.
    populate_details = function(self, from_list)
        local id
        local ap
        local ap_next
        local level
        local max_level
        local master = false
        local materia
        if from_list == false then
            if self.header_position_row == 1 then
                -- Weapon materia
                local weapon_id = Characters[self.char_id].weapon.id
                local weapon = Game.Items[weapon_id]
                if #(weapon.slots) < self.header_position -1 or Characters[self.char_id].weapon.materia[self.header_position - 2] == nil then
                    -- No slot or no materia in slot. Clear and exit
                    self.hide_details(self)
                    return 0
                end
                id = Characters[self.char_id].weapon.materia[self.header_position - 2].id
                ap = Characters[self.char_id].weapon.materia[self.header_position - 2].ap
                materia = Game.Materia[id]
            elseif self.header_position_row == 2 then
                -- Armor materia
                local armor_id = Characters[self.char_id].armor.id
                local armor = Game.Items[armor_id]
                if #(armor.slots) < self.header_position - 1 or Characters[self.char_id].armor.materia[self.header_position - 2] == nil then
                    -- No slot or no materia in slot. Clear and exit
                    self.hide_details(self)
                    return 0
                end
                id = Characters[self.char_id].armor.materia[self.header_position - 2].id
                ap = Characters[self.char_id].armor.materia[self.header_position - 2].ap
                materia = Game.Materia[id]
            else
                -- This should not happen, but just in case.
                self.hide_details(self)
                return 0
            end
        else
            -- TODO: Get materia data from list
        end

        -- Get level and max level, and check for master.
        level = 0
        for lv, lv_ap in ipairs(materia.levels_ap) do
            if lv_ap > ap then
                break
            end
            level = level + 1
        end
        max_level = #(materia.levels_ap)
        if level == max_level then
            master = true
            ap_next = 0
        else
            ap_next = materia.levels_ap[level + 1] - ap
        end
        -- Start displaying data
        ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Icon"):set_image("images/icons/materia_" .. tostring(materia.type) .. ".png")
        ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Name"):set_text(materia.name)
        for l = 1, 5 do
            if l > max_level then
                -- No such level, hide star.
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Stars.Star" .. tostring(l)):set_visible(false)
            else
                if l <= level then
                    ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Stars.Star" .. tostring(l)):set_image("images/icons/materia_star_" .. tostring(materia.type) .. ".png")
                else
                    ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Stars.Star" .. tostring(l)):set_image("images/icons/materia_star_empty_" .. tostring(materia.type) .. ".png")
                end
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Stars.Star" .. tostring(l)):set_visible(true)
            end
        end
        ui_manager:get_widget("MateriaMenu.Container.Details.Materia.AP"):set_text(UiContainer.pad_value(ap, 6))
        ui_manager:get_widget("MateriaMenu.Container.Details.Materia.APNext"):set_text(UiContainer.pad_value(ap_next, 6))
        --ui_manager:get_widget("MateriaMenu.Container.Details.Materia.AP"):set_text(UiContainer.pad_value(200000, 6))
        --ui_manager:get_widget("MateriaMenu.Container.Details.Materia.APNext"):set_text(UiContainer.pad_value(30000, 6))

        -- Hide all attacks
        for a = 1, 5 do
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(a)):set_visible(false)
        end

        -- Show attacks, vary by type
        if materia.type == Materia.TYPE.MAGIC then
            local attack_index = 1
            for attack_level, attack_id in pairs(materia.magic) do
                if level >= attack_level then
                    ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_colour(1, 1, 1)
                else
                    ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_colour(0.6, 0.6, 0.6)
                end
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_text(Game.Attacks[attack_id].name)
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_visible(true)
                attack_index = attack_index + 1
            end
        elseif materia.type == Materia.TYPE.SUMMON then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_colour(1, 1, 1)
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_text(Game.Attacks[materia.summon].name)
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_visible(true)
        elseif materia.type == Materia.TYPE.COMMAND then
            local attack_index = 1
            for attack_level, attack_id in ipairs(materia.command) do
                if level >= attack_level then
                    ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_colour(1, 1, 1)
                else
                    ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_colour(0.6, 0.6, 0.6)
                end
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_text(Game.Commands[attack_id].name)
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability" .. tostring(attack_index)):set_visible(true)
                attack_index = attack_index + 1
            end
        elseif materia.type == Materia.TYPE.INDEPENDENT then
            local bonus = 0
            for l = 1, max_level do
                if l <= level and materia.ability.param[l] ~= nil then
                    bonus = materia.ability.param[l]
                end
            end
            if materia.ability ~= nil then
                local text = "TEXT"
                if materia.ability.id == Materia.ABILITY.MP_PLUS then
                    text = "<include name=\"StatMP\"/> +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.HP_PLUS then
                    text = "<include name=\"StatHP\"/> +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.STR_PLUS then
                    text = "<include name=\"StatStr\"/> +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.VIT_PLUS then
                    text = "<include name=\"StatVit\"/> +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.MAG_PLUS then
                    text = "<include name=\"StatMag\"/> +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.DEX_PLUS then
                    text = "<include name=\"StatDex\"/> +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.LCK_PLUS then
                    text = "<include name=\"StatLck\"/> +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.EXP_PLUS then
                    text = "<include name=\"StatEXP\"/> +"
                elseif materia.ability.id == Materia.ABILITY.MONEY_PLUS then
                    text = "<include name=\"Currency\"/> +"
                elseif materia.ability.id == Materia.ABILITY.ENCOUNTER_DOWN then
                    text = "<include name=\"MateriaEncounters\"/> -"
                elseif materia.ability.id == Materia.ABILITY.ENCOUNTER_UP then
                    text = "<include name=\"MateriaEncountersChocobo\"/> -"
                elseif materia.ability.id == Materia.ABILITY.PREEMPTIVE then
                    text = materia.name
                elseif materia.ability.id == Materia.ABILITY.LONG_RANGE then
                    text = materia.name
                elseif materia.ability.id == Materia.ABILITY.MEGA_ALL then
                    text = materia.name
                elseif materia.ability.id == Materia.ABILITY.COUNTER_ATTACK then
                    text = materia.name
                elseif materia.ability.id == Materia.ABILITY.COVER then
                    text = materia.name .. " +<colour value=\"1 1 0\">" .. tostring(bonus) .. "</colour>%"
                elseif materia.ability.id == Materia.ABILITY.UNDERWATER then
                    text = materia.name
                elseif materia.ability.id == Materia.ABILITY.HP_MP then
                    text = materia.name
                end
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_colour(1, 1, 1)
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_text(text)
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_visible(true)
            end
        elseif materia.type == Materia.TYPE.SUPPORT then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_colour(1, 1, 1)
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_text(materia.name)
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Abilities.Ability1"):set_visible(true)
        end

        -- Hide all effects
        for a = 1, 7 do
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(a)):set_visible(false)
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(a)):set_visible(false)
        end

        -- Show effects, if any.
        local effect_index = 1
        if materia.stats.str ~= nil and materia.stats.str ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatStr\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.str < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.str, 2) .. "</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.str, 2) .. "</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
        if materia.stats.vit ~= nil and materia.stats.vit ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatVit\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.vit < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.vit, 2) .. "</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.vit, 2) .. "</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
        if materia.stats.mag ~= nil and materia.stats.mag ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatMag\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.mag < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.mag, 2) .. "</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.mag, 2) .. "</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
        if materia.stats.spr ~= nil and materia.stats.spr ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatSpr\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.spr < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.spr, 2) .. "</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.spr, 2) .. "</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
        if materia.stats.dex ~= nil and materia.stats.dex ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatDex\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.dex < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.dex, 2) .. "</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.dex, 2) .. "</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
        if materia.stats.lck ~= nil and materia.stats.lck ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatLck\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.lck < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.lck, 2) .. "</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.lck, 2) .. "</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
        if materia.stats.hp ~= nil and materia.stats.hp ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatHP\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.hp < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.hp, 2) .. "%</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.hp, 2) .. "%</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
        if materia.stats.mp ~= nil and materia.stats.mp ~= 0 then
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_text("<include name=\"StatMP\" />")
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Stat" .. tostring(effect_index)):set_visible(true)
            if materia.stats.mp < 0 then
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 0 0\">-" .. UiContainer.pad_value(-1 * materia.stats.mp, 2) .. "%</colour>")
            else
                ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_text("<colour value=\"1 1 0\">+" .. UiContainer.pad_value(materia.stats.mp, 2) .. "%</colour>")
            end
            ui_manager:get_widget("MateriaMenu.Container.Details.Materia.Effects.Val" .. tostring(effect_index)):set_visible(true)
            effect_index = effect_index + 1
        end
    end,

    --- Hides the item menu and goes back to the main menu.
    hide = function(self)
        ui_manager:get_widget("MateriaMenu"):set_visible(false)
        UiContainer.current_menu = "main"
        UiContainer.current_submenu = ""
        return 0;
    end,
}
