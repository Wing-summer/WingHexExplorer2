-- Full-width values are constructed from strings, without a double conversion.
local minimum = int64("-9223372036854775808")
local maximum = int64("9223372036854775807")
local unsignedMaximum = uint64("18446744073709551615")
assert(existing_signed == minimum and existing_unsigned == unsignedMaximum)
assert(tostring(minimum) == "-9223372036854775808")
assert(tostring(maximum) == "9223372036854775807")
assert(tostring(unsignedMaximum) == "18446744073709551615")
assert(uint64("0xffffffffffffffff") == unsignedMaximum)
assert(int64("-0x8000000000000000") == minimum)
assert(uint64("0b101010"):to_number() == 42)
assert(int64(1) == uint64(1))
assert(minimum < unsignedMaximum and maximum < unsignedMaximum)
assert(minimum < int64(-1) and int64(-1) < int64(0))
assert(uint64(int64(1)) == uint64(1))
assert(int64(uint64(1)) == int64(1))
assert(tostring(int64("9007199254740993") + 2) == "9007199254740995")
assert(tostring(2 + uint64("9007199254740993")) == "9007199254740995")
assert(maximum - 1 == int64("9223372036854775806"))
assert(minimum + maximum == int64(-1))
assert(int64("4294967296") * int64("2147483647") == int64("9223372032559808512"))
assert(minimum * 1 == minimum)
assert(minimum * 0 == int64(0))
assert(int64(-7) / 3 == int64(-3))
assert(int64(-7) // 3 == int64(-3))
assert(int64(-7) % 3 == int64(2))
assert(int64(7) % -3 == int64(-2))
assert(minimum % -1 == int64(0))
assert(unsignedMaximum / 2 == uint64("9223372036854775807"))
assert(unsignedMaximum % 2 == uint64(1))
assert(-int64(42) == int64(-42))
assert(unsignedMaximum:band(255) == uint64(255))
assert(uint64(0):bnot() == unsignedMaximum)
assert(uint64(1):lshift(63) == uint64("9223372036854775808"))
assert(unsignedMaximum:rshift(63) == uint64(1))
assert(int64(-1):arshift(63) == int64(-1))
assert(int64(-1):rshift(63) == int64(1))
assert(uint64(1):lshift(64) == uint64(0))
assert(int64(-1):arshift(64) == int64(-1))
assert(uint64(1):bor(2):bxor(1) == uint64(2))
assert(unsignedMaximum:high() == 4294967295)
assert(unsignedMaximum:low() == 4294967295)
assert(unsignedMaximum:to_hex() == "ffffffffffffffff")
assert(minimum:to_hex() == "8000000000000000")
assert(int64("9007199254740991"):to_number() == 9007199254740991)
assert(int64("-9007199254740991"):to_number() == -9007199254740991)
for _, invalid in ipairs({"", "-", "0x", "1.0", "1e3", " 1", "12z", "9223372036854775808", "-9223372036854775809"}) do
    assert(not pcall(int64, invalid), invalid)
end
for _, invalid in ipairs({"-1", "18446744073709551616", "0x10000000000000000"}) do
    assert(not pcall(uint64, invalid), invalid)
end
local failures = {
    function() return maximum + 1 end,
    function() return minimum - 1 end,
    function() return unsignedMaximum + 1 end,
    function() return uint64(0) - 1 end,
    function() return maximum * 2 end,
    function() return unsignedMaximum * 2 end,
    function() return -minimum end,
    function() return minimum / -1 end,
    function() return maximum / 0 end,
    function() return maximum % 0 end,
    function() return uint64(-1) end,
    function() return int64(unsignedMaximum) end,
    function() return uint64(minimum) end,
    function() return int64(1.5) end,
    function() return int64(math.huge) end,
    function() return int64(0/0) end,
    function() return int64(9007199254740992) end,
    function() return unsignedMaximum:to_number() end,
    function() return minimum:to_number() end,
    function() return uint64(1):lshift(-1) end,
    function() return uint64(1):rshift(65) end,
}
for index, failure in ipairs(failures) do
    assert(not pcall(failure), `overflow check {index}`)
end
-- Failed conversions leave the VM usable.
assert(int64(21) * 2 == int64(42))
