-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
local api = require("./i18n_api")
export type Catalog = api.Catalog
export type Translator = api.Translator

local env = require("@s/env")
local I18n = {}

local function normalize(locale)
    assert(type(locale) == "string", "Expected a locale string")
    return string.lower(string.gsub(locale, "_", "-"))
end

function I18n.create(catalog, locale)
    assert(type(catalog) == "table", "Expected translation dictionaries")
    local dictionaries = {}
    for language, entries in catalog do
        local name = normalize(language)
        assert(name ~= "" and dictionaries[name] == nil, "Duplicate or empty catalog locale: " .. name)
        assert(type(entries) == "table", "Expected a translation dictionary: " .. name)
        for source, translation in entries do
            assert(type(source) == "string" and type(translation) == "string", "Translations must map source strings to strings")
        end
        dictionaries[name] = entries
    end

    -- Select only caller-supplied dictionaries, from most to least specific.
    -- Language/script/territory aliases belong to the catalog, not the library.
    local fallbacks = {}
    local name = normalize(locale or env.locale)
    while name ~= "" do
        if dictionaries[name] then table.insert(fallbacks, dictionaries[name]) end
        name = string.match(name, "^(.*)%-[^-]+$") or ""
    end

    return function(source, ...)
        local text = source
        for _, dictionary in fallbacks do
            local translation = dictionary[source]
            if translation ~= nil then text = translation; break end
        end
        if select("#", ...) == 0 then return text end
        return string.format(text, ...)
    end
end

return table.freeze(I18n) :: api.Module
