if UiContainer == nil then UiContainer = {} end

UiContainer.pad_value = function(value, length)
    local str = tostring(value)
    while #(str) < length do
        str = " " .. str
    end
    return str
end

UiContainer.zero_pad_value = function(value, length)
    local str = tostring(value)
    while #(str) < length do
        str = "0" .. str
    end
    return str
end

--- Populates a character data.
--
-- @param widget Name of the widget with the character data.
-- @param character The character to get the data from. If nill, the character details will be
-- hidden.
UiContainer.populate_character_data = function(widget, character)

    if character == nil then
        ui_manager:get_widget(widget):set_visible(false)
        return 0
    end

    -- Maximum HP and MP include the bonuses of equipment and materia. Before the stats are first
    -- calculated they are not set, so fall back to the base values.
    local hp_max = character.stats.hp.max or character.stats.hp.base
    local mp_max = character.stats.mp.max or character.stats.mp.base

    local char_id = character.char_id
    ui_manager:get_widget(widget):set_visible(true)

    -- Critical?
    if character.stats.hp.current == 0 then
        ui_manager:get_widget(widget .. ".Data.HpCurrent"):set_colour(1.0, 0, 0)
    elseif character.stats.hp.current <= hp_max / 4 then
        ui_manager:get_widget(widget .. ".Data.HpCurrent"):set_colour(1.0, 1.0, 0)
    else
        ui_manager:get_widget(widget .. ".Data.HpCurrent"):set_colour(0.9, 0.9, 0.9)
    end

    -- Text data
    ui_manager:get_widget(widget .. ".Data.Name"):set_text(character.name)
    ui_manager:get_widget(widget .. ".Data.LvNumber"):set_text(UiContainer.pad_value(character.level, 3))
    ui_manager:get_widget(widget .. ".Data.HpCurrent"):set_text(UiContainer.pad_value(character.stats.hp.current, 5))
    ui_manager:get_widget(widget .. ".Data.HpMax"):set_text(UiContainer.pad_value(hp_max, 5))
    ui_manager:get_widget(widget .. ".Data.MpCurrent"):set_text(UiContainer.pad_value(character.stats.mp.current, 5))
    ui_manager:get_widget(widget .. ".Data.MpMax"):set_text(UiContainer.pad_value(mp_max, 5))

    -- Calculate the HP and MP bar.
    local max_hp_width= ui_manager:get_widget(widget .. ".Data.HpLine"):get_width()
    local hp_width = math.floor(character.stats.hp.current * max_hp_width / hp_max)
    ui_manager:get_widget(widget .. ".Data.HpLineCurrent"):set_width(hp_width)
    local max_mp_width= ui_manager:get_widget(widget .. ".Data.MpLine"):get_width()
    local mp_width = character.stats.mp.current * max_mp_width / mp_max
    ui_manager:get_widget(widget .. ".Data.MpLineCurrent"):set_width(mp_width)

    -- Status (Fury/Sadness)
    local status_text = ""
    for _, st in ipairs(character.status) do
        if st == UiContainer.Status.SADNESS then
            status_text = "Sadness"
            break
        elseif st == UiContainer.Status.FURY then
            status_text = "Fury"
            break
        end
    end
    local status_widget = ui_manager:get_widget(widget .. ".Data.Status")
    status_widget:set_colour(0.8, 0, 1)
    status_widget:set_text(status_text)

    return 0
end
