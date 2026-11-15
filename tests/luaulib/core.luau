local bytes = require("@s/bytes")
local base64 = require("@s/base64")
local lz4 = require("@s/lz4")
local binary = require("@s/binary")
local i18n = require("@s/i18n")
local function rejects(callback) assert(not pcall(callback), "Expected rejection") end
for _, text in { "", "a", "ab", "abc", "hello\0world", string.rep("abc", 10000) } do
    assert(buffer.tostring(bytes.from_hex(bytes.to_hex(text))) == text)
    assert(buffer.tostring(base64.decode(base64.encode(text))) == text)
    assert(lz4.decompress(lz4.compress(text), math.max(1, #text)) == text)
end
for _, text in { "a", "AB=C", "====", "Zh==", "!AAA", "Zg=", "Zg==\n" } do rejects(function() base64.decode(text) end) end
rejects(function() bytes.from_hex("a") end)
rejects(function() bytes.from_hex("zz") end)
rejects(function() lz4.decompress("\0\0\0") end)
rejects(function() lz4.decompress(lz4.compress(string.rep("x", 4096)), 8) end)
-- Locale matching is generic: each message falls back through parent tags.
local catalog = {
    ["sr-Latn-RS"] = { Exact = "regional" },
    sr_Latn = { Exact = "script", Script = "script %s", Empty = "" },
    sr = { Exact = "base", Script = "base", Base = "base", Percent = "100%" },
    fr = { Hello = "Bonjour" },
    zh_CN = { Hello = "explicit mainland" },
}
local tr = i18n.create(catalog, "SR_latn_RS")
assert(tr("Exact") == "regional")
assert(tr("Script", "Wing") == "script Wing")
assert(tr("Base") == "base" and tr("Empty") == "")
assert(tr("Missing %s", "Wing") == "Missing Wing")
assert(tr("Percent") == "100%")
assert(i18n.create(catalog, "fr_CA")("Hello") == "Bonjour")
assert(i18n.create(catalog, "de_DE")("Missing 50%") == "Missing 50%")
assert(i18n.create(catalog, "zh-SG")("Hello") == "Hello")
assert(i18n.create(catalog, "zh-Hant-HK")("Hello") == "Hello")
local host_locale = require("@s/env").locale
assert(i18n.create({[host_locale] = {Hello = "host locale"}})("Hello") == "host locale")
assert(i18n.create({sr = {Base = "independent"}}, "sr")("Base") == "independent")
assert(tr("Base") == "base")
assert(i18n.create({}, "")("source") == "source")
rejects(function() i18n.create({en_US = {}, ["EN-us"] = {}}) end)
rejects(function() i18n.create({en = {Hello = false}}) end)
rejects(function() i18n.create({en = "not a dictionary"}) end)
local nested = binary.register("nested", { fields = { {name = "x", type = "i16"} } })
local schema = binary.register("fixture", { size = 24, endian = "big", fields = {
    {name = "a", type = "u16", offset = 2}, {name = "b", type = nested, count = 2},
    {name = "c", type = "f32"}, {name = "s", type = "bytes", size = 3},
} })
local original = string.rep("\255", 24)
local encoded = binary.encode(schema, {a = 65535, b = {{x = -1}, {x = -32768}}, c = 1.5, s = "a\0b"}, original)
local result = binary.decode(schema, "prefix" .. encoded, 6)
assert(result.offset == 6 and result.values.a == 65535 and result.values.b[2].x == -32768)
assert(result.values.c == 1.5 and result.values.s == "a\0b" and #result.fields == 7)
assert(string.sub(encoded, 1, 2) == "\255\255" and string.sub(encoded, 16) == string.rep("\255", 9))
assert(binary.encode(schema, result.values, result.original) == encoded)
rejects(function() binary.decode(schema, "short") end)
rejects(function() binary.encode(schema, {a = 65536}) end)
rejects(function() binary.encode(schema, {s = "toolong"}) end)
rejects(function() binary.decode(schema, encoded, 0, {maxWork = 1}) end)
local union = binary.register("union", {union = true, member = "word", fields = {{name = "word", type = "u32", offset = 0}, {name = "bytes", type = "bytes", size = 4, offset = 0}}})
assert(binary.decode(union, "\1\0\0\0").values.word == 1)
print("PASS standard library core")
