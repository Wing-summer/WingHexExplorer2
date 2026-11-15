--!native
--!optimize 2

local base64 = require("../base64/Base64")

local coolbuffer = {}

function coolbuffer.fromhex(hex: buffer): buffer -- this will do by number calculations
	-- local len = #hex
	-- if len % 2 ~= 0 then
	-- 	hex = "0" .. hex
	-- 	len += 1
	-- end
	local size = buffer.len(hex)
	if size % 2 ~= 0 then
		local b = buffer.create(size + 1)
		buffer.writeu8(b, 0, 48)
		buffer.copy(b, 1, hex, 0, size)
		hex = b
		size += 1
	end
	local b = buffer.create(size / 2)
	local offset = 0

	for i = 0, size - 1, 2 do
		local first = buffer.readu8(hex, i) -- string.byte(hex, i)
		local second = buffer.readu8(hex, i + 1) -- string.byte(hex, i + 1)
		if first >= 48 and first <= 57 then
			first -= 48
		elseif first >= 65 and first <= 70 then
			first -= 55
		elseif first >= 97 and first <= 102 then
			first -= 87
		else
			error("unexpected character in hex string")
		end
		if second >= 48 and second <= 57 then
			second -= 48
		elseif second >= 65 and second <= 70 then
			second -= 55
		elseif second >= 97 and second <= 102 then
			second -= 87
		else
			error("unexpected character in hex string")
		end
		buffer.writeu8(b, offset, first * 16 + second)
		offset += 1
	end

	return b
end

function coolbuffer.tohex(b: buffer): buffer
	local size = buffer.len(b)
	local hex = buffer.create(size * 2) -- table.create(size * 2)

	for i = 0, size - 1 do
		local byte = buffer.readu8(b, i)
		local first = math.floor(byte / 16)
		local second = byte % 16
		buffer.writeu8(hex, i * 2, if first < 10 then first + 48 else first + 87)
		buffer.writeu8(hex, i * 2 + 1, if second < 10 then second + 48 else second + 87)
		--hex[i * 2 + 1] = if first < 10 then string.char(first + 48) else string.char(first + 87)
		--hex[i * 2 + 2] = if second < 10 then string.char(second + 48) else string.char(second + 87)
	end

	return hex
end

coolbuffer.frombase64 = base64.decode
coolbuffer.tobase64 = base64.encode

return coolbuffer
