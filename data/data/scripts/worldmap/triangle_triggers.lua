System.WorldMap = {
    triangle_triggers = {},
    triangle_functions = {},
    field_table = {},

    --[[
    Registers a callback for a WM0 mesh function.
    @param mesh_x World-map mesh X coordinate (0-35).
    @param mesh_y World-map mesh Y coordinate (0-27).
    @param function_id Triangle mesh function ID (0-4).
    @param callback Function called when the player enters the trigger.
    @return True when registered, false for invalid input.
    ]]
    register_triangle_trigger = function(self, mesh_x, mesh_y, function_id, callback)
        if type(mesh_x) ~= "number" or mesh_x < 0 or mesh_x > 35
          or type(mesh_y) ~= "number" or mesh_y < 0 or mesh_y > 27
          or type(function_id) ~= "number" or function_id < 0 or function_id > 4
          or type(callback) ~= "function" then
            return false
        end

        local mesh_triggers = self.triangle_triggers[mesh_y]
        if not mesh_triggers then
            mesh_triggers = {}
            self.triangle_triggers[mesh_y] = mesh_triggers
        end
        local function_triggers = mesh_triggers[mesh_x]
        if not function_triggers then
            function_triggers = {}
            mesh_triggers[mesh_x] = function_triggers
        end
        function_triggers[function_id] = callback
        return true
    end,

    --[[
    Dispatches an entered WM0 mesh function to its registered callback.
    @param self World-map system script table.
    @param mesh_index Linear mesh index (mesh_y * 36 + mesh_x).
    @param function_id Triangle mesh function ID.
    ]]
    on_triangle_enter = function(self, mesh_index, function_id)
        mesh_index = tonumber(mesh_index)
        function_id = tonumber(function_id)
        if not mesh_index or not function_id then
            return
        end

        local mesh_x = mesh_index % 36
        local mesh_y = math.floor(mesh_index / 36)
        local mesh_triggers = self.triangle_triggers[mesh_y]
        local function_triggers = mesh_triggers and mesh_triggers[mesh_x]
        local callback = function_triggers and function_triggers[function_id]
        if callback then
            callback(mesh_x, mesh_y, function_id)
            return
        end

        local mesh_functions = self.triangle_functions[mesh_index]
        local function_index = mesh_functions and mesh_functions[function_id]
        local world_function = function_index
          and self["world_function_" .. tostring(function_index)]
        if world_function then
            world_function(self)
        else
            print(
                "No world-map function for mesh " .. mesh_index
                .. ", function " .. function_id
            )
        end
    end,
}

EnterFieldScene = function(field_table_id, scenario)
    local field_table = System.WorldMap.field_table[tonumber(field_table_id)]
    local entry = field_table and field_table[tonumber(scenario)]
    if not entry then
        print("No installed field entry for FIELD.TBL record " .. tostring(field_table_id))
        return 0
    end

    System.MapChanger.world_map_entry = entry
    load_field_map_request(entry.field, "")
    return 0
end