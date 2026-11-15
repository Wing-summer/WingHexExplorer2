-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
local api = require("./env_api")
export type Version = api.Version
export type Environment = api.Environment

return require("@b/winghex").env :: api.Module
