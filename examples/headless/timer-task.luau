-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
local core = require("@b/core")
local timer = core.timer()
local count = 0
timer:connect("timeout", function()
    count += 1
    println("tick", count)
end)
timer:start(1000)
-- The service retains this task until task.stop or service.shutdown.
