-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
local api = require("./icons_api")
export type IconIds = api.IconIds

local host = require("@b/winghex")
local Icons = { Id = host.StandardIcon }
local function require_gui()
    assert(host.guiAvailable, "Standard icons require an available GUI")
    require("@b/qt")
end
function Icons.icon(id) require_gui(); return host.standard_icon(id) end
function Icons.pixmap(id) require_gui(); return host.standard_pixmap(id) end
function Icons.data(id) require_gui(); return host.standard_icon_data(id) end
return table.freeze(Icons) :: api.Module
