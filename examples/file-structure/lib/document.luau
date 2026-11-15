-- Copyright (C) 2026 WingSummer
-- SPDX-License-Identifier: BSD-3-Clause
local Binary = require("@s/binaryreader")
local I18n = require("./i18n")
local Document = {}
local Analysis = require("@s/analysis")
local Icons = require("@s/icons")
function Document.current(callback, i18n)
    assert(reader.is_current_doc_editing(), i18n:text("No document is being edited"))
    return Analysis.current(function(document) return callback(i18n, document) end)
end

-- Notify only after the document scope has released its handle/undo group.
function Document.run(title, icon, callback, language)
    local i18n = I18n.new(language)
    local ok, message, statusIcon = pcall(Document.current, callback, i18n)
    local iconName = if ok then statusIcon or icon else Icons.Id.Error
    local pixmap = Icons.pixmap(iconName)
    if not ok then message = i18n:text("Execution failed: %s", i18n:diagnostic(message)) end
    toast.show(pixmap, i18n:text("%s: %s", i18n:text(title), tostring(message)))
    if not ok then error(message, 0) end
    return message
end

function Document.colorize(parser, title, language)
    return Document.run(title, Icons.Id.ColorPalette, function(i18n, document)
        local plan = Binary.new(document:document_bytes(), function(offset, size) return document:read_bytes(offset, size) end)
        parser(plan)
        local count = Analysis.annotate(document, plan.marks, { title = i18n:text(title), tr = function(source) return i18n:text(source) end })
        return i18n:text("%d annotations applied", count)
    end, language)
end

return Document
