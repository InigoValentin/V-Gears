if UiContainer == nil then UiContainer = {} end

--- The quit menu.
--
-- Only shows a window warning that the unsaved progress will be lost. Confirming quits the game,
-- declining (or cancelling) goes back to the main menu.
UiContainer.QuitMenu = UiContainer.Menu.new{

    id = "quit",
    parent = "main",

    --- Cursor over Yes and No.
    cursor = UiContainer.Cursor.new{
        widget = "QuitMenu.Container.Confirm.Cursor",
        total = 2,
    },

    states = {
        [""] = {
            cursor = function(self) return self.cursor end,
            axis = "vertical",
            confirm = function(self, cursor)
                if cursor:index() == 1 then
                    console("quit")
                else
                    script:request_end_sync(Script.UI, "QuitMenu", "hide", 0)
                end
            end,
            cancel = function(self)
                script:request_end_sync(Script.UI, "QuitMenu", "hide", 0)
            end,
        },
    },

    --- Opens the window. The cursor starts on No.
    show = function(self)
        self:activate()
        self.cursor:set(2)
        ui_manager:get_widget("QuitMenu"):set_visible(true)
        return 0
    end,

    --- Closes the window and goes back to the main menu.
    hide = function(self)
        ui_manager:get_widget("QuitMenu"):set_visible(false)
        self:deactivate()
        return 0
    end,
}
