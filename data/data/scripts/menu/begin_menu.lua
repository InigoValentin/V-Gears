if UiContainer == nil then UiContainer = {} end

--- The begin menu
UiContainer.BeginMenu = UiContainer.Menu.new{

    id = "begin",

    --- Cursor over the entries.
    cursor = UiContainer.Cursor.new{
        widget = "BeginMenu.Container.Cursor",
        total = 7,
    },

    --- Run when the menu is created.
    --
    -- Sets this menu as the one being displayed.
    on_start = function(self)
        self:activate()
        self.cursor:refresh()
        return 0
    end,

    --- What each entry does. Indexed by cursor position.
    actions = {
        -- 1: New game
        function(self)
            audio_manager:play_sound("Window")
            generate_control_key()
            load_field_map_request("md1stin", "")
            console("camera_free false")
            --console("debug_walkmesh true")
            script:request_end_sync(Script.UI, "BeginMenu", "hide", 0)
            MenuSettings.pause_available = true
        end,

        -- 2: Load, show save menu.
        function(self)
            audio_manager:play_sound("Cursor")
            UiContainer.SaveMenu.operation = "load"
            script:request_end_sync(Script.UI, "SaveMenu", "show", 0)
        end,

        -- 3: NMKIN_3 test.
        function(self)
            Data.progress_game = 27
            set_party(0, 1, -1)
            load_field_map_request("nmkin_3", "")
            console("camera_free false")
            console("debug_walkmesh true")
            script:wait(1)
            script:request_end_sync(Script.UI, "BeginMenu", "hide", 0)
            entity_manager:get_entity("Cloud"):set_position(-1.090750, 15.310307, 17.281244)
            entity_manager:set_player_entity("Cloud")
            script:wait(1)
            entity_manager:get_player_entity():set_position(-1.093750, 15.320307, 17.281244)
            entity_manager:get_entity("Cloud"):jump_to_position(0.46875, 14.9922, -1, 1, 42) -- Triangle 42, 2 steps.
            entity_manager:get_entity("Cloud"):jump_sync()
            entity_manager:player_lock(false)
            MenuSettings.pause_available = true
            -- NMKIN_4 test:
            --[[Data.progress_game = 14
            --load_field_map_request("nmkin_4", "")
            map("nmkin_4")
            console("camera_free false")
            console("debug_walkmesh true")
            script:wait(1.5)
            script:request_end_sync(Script.UI, "BeginMenu", "hide", 0)
            entity_manager:get_entity("Cloud"):set_position(-0.820312, 0.500000, 17.281244)
            entity_manager:set_player_entity("Cloud")
            set_party(0, 1, 4)
            script:wait(0.5)
            entity_manager:get_player_entity():set_position(-0.820312, 0.500000, 17.281244)
            entity_manager:get_entity("Cloud"):jump_to_position(-0.800312, 0.500000, -1, 1) -- Triangle 42, 2 steps.
            entity_manager:get_entity("Cloud"):jump_sync()
            entity_manager:player_lock(false)
            print("TRIANGLE " .. entity_manager:get_entity("Cloud"):get_move_triangle_id())
            MenuSettings.pause_available = true]]
        end,

        -- 4: Walkmesh test map.
        function(self)
            script:request_end_sync(Script.UI, "BeginMenu", "hide", 0)
            console("camera_free true")
            console("debug_walkmesh true")
            map("test_2")
            MenuSettings.pause_available = true
        end,

        -- 5: World map.
        function(self)
            set_party(0, 2, 3)
            script:request_end_sync(Script.UI, "BeginMenu", "hide", 0)
            console("camera_free true")
            console("debug_entity 2")
            MenuSettings.pause_available = true
            world_map_module:init()
        end,

        -- 6: Start map.
        function(self)
            load_field_map_request("startmap", "")
            console("camera_free false")
            console("debug_walkmesh false")
            script:request_end_sync(Script.UI, "BeginMenu", "hide", 0)
            MenuSettings.pause_available = true
        end,

        -- 7: Test battle.
        function(self)
            --load_field_map_request("startmap", "")
            console("camera_free false")
            console("debug_walkmesh false")
            script:request_end_sync(Script.UI, "BeginMenu", "hide", 0)
            set_party(0, 2, 3)
            Battle.start(372) -- Single level 2 MP, easy test battle
            MenuSettings.pause_available = true
        end,
    },

    --- Handles button events.
    --
    -- Handles directional Up, Down and Enter keys.
    -- @param button Pressed direction or action name.
    -- @param event Trigger event. Normally, "Press".
    on_button = function(self, button, event)
        if self:is_active() then
            if button == Config.ACTIONS.CONFIRM and event == "Press" then
                local action = self.actions[self.cursor:index()]
                if action ~= nil then action(self) end
            else
                self.cursor:handle(button, "vertical")
            end
        end
        return 0
    end,

    --- Opens the begin menu.
    show = function(self)
        self:activate()
        ui_manager:get_widget("BeginMenu"):set_visible(true)
        return 0
    end,

    --- Hides the begin menu.
    --
    -- It doesn't do anything else.
    hide = function(self)
        self:deactivate("")
        ui_manager:get_widget("BeginMenu"):set_visible(false)
        return 0
    end,
}
