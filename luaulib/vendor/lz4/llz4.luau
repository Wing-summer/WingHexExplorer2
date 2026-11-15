local band, lshift, rshift = bit32.band, bit32.lshift, bit32.rshift

local MIN_MATCH = 4 -- A sequence is 3 bytes, so it has to encode at least 4 to have any use
local MIN_LENGTH = 13
local MIN_TRAILING_LITERALS = 5
local MISS_COUNTER_BITS = 6 -- Lower values = the step is incremented sooner
local HASH_SHIFT = 32 - 16  -- 32 - # of bits in the hash
local MAX_DISTANCE = 0xFFFF -- Maximum offset that can fit into two bytes

local COMPRESSION_GROWTH = 1 + 1/255 -- Asymptotic growth ratio of incompressible data
local BUFFER_SIZE_PADDING = 16
local DEFAULT_BUFFER_SIZE = 512 * 1024^1 -- 512 KiB
local DEFAULT_MAX_BUFFER_SIZE = 2^31 -- BIG

local LIT_COUNT_BITS = 4
local LIT_COUNT_MASK = lshift(1, LIT_COUNT_BITS) - 1
local MATCH_LEN_BITS = 4
local MATCH_LEN_MASK = lshift(1, MATCH_LEN_BITS) - 1


local function check_type(value, typ, argNum, funcName)
	local actual = typeof(value)
	assert(actual == typ, `bad argument #{argNum} to '{funcName}' ({typ} expected, got {actual})`)
end

local function check_data(data, dataStart, dataLen, funcName)
	check_type(data, "buffer", 1, funcName)
	check_type(dataStart, "number", 2, funcName)
	check_type(dataLen, "number", 3, funcName)
	assert(dataStart >= 0, "dataStart cannot be negative")
	assert(dataLen >= 0, "dataLen cannot be negative")
	assert(dataStart + dataLen <= buffer.len(data), "data range not in buffer")
end

local function check_acceleration(acceleration, argNum, funcName)
	acceleration = acceleration or 1
	check_type(acceleration, "number", argNum, funcName)
	assert(acceleration >= 1 and acceleration % 1 == 0, "acceleration must be an integer >= 1")
	return acceleration
end


-- MARK: compress_buffer
--[[=
	Compresses a portion of a buffer using the LZ4 block format.

	@param buffer data The buffer containing the data to compress.
	@param number dataStart The offset at which the data to compress starts.
	@param number dataLen The length, in bytes, of the data to compress.
	@param number? acceleration A positive integer, defaults to 1. Higher values
	  may increase the compression speed, especially on incompressible data, at
	  the cost of compression efficiency.

	@return buffer A buffer containing the compressed data at offset 0.
	@return number The length of the compressed data.
]]
local function compress_buffer(data, dataStart, dataLen, acceleration)
	check_data(data, dataStart, dataLen, "compress_buffer")
	acceleration = check_acceleration(acceleration, 4, "compress_buffer")

	local hashTable = {}
	local out, outNext = buffer.create(math.ceil(dataLen * COMPRESSION_GROWTH) + BUFFER_SIZE_PADDING), 0

	local pos = dataStart -- 0-indexed
	local nextUnencodedPos = pos -- Sometimes called the "anchor" in other implementations

	if dataLen >= MIN_LENGTH then
		-- The lower MISS_COUNTER_BITS bits are the miss counter, upper bits are the step. The step
		-- starts at `acceleration` and increments every time the miss counter overflows.
		local stepAndMissCounterInit = lshift(acceleration, MISS_COUNTER_BITS)
		local stepAndMissCounter = stepAndMissCounterInit

		while pos + MIN_MATCH < dataLen - MIN_TRAILING_LITERALS do
			local sequence = buffer.readu32(data, pos)
			local hash = rshift(sequence * 2654435761, HASH_SHIFT)
			-- ^ This is awfully simple for a hash function, but it's fast and seems to give pretty good results. The
			-- magic constant was taken from https://github.com/lz4/lz4/blob/836decd8a898475dcd21ed46768157f4420c9dd2/lib/lz4.c#L782

			-- Check and update match
			local matchPos = hashTable[hash]
			hashTable[hash] = pos

			-- Determine if there is a match in range
			if not matchPos or pos - matchPos > MAX_DISTANCE or buffer.readu32(data, matchPos) ~= sequence then
				pos = pos + rshift(stepAndMissCounter, MISS_COUNTER_BITS) -- Extract and add the step part
				stepAndMissCounter = stepAndMissCounter + 1
				continue
			end

			stepAndMissCounter = stepAndMissCounterInit

			-- Calculate literal count and offset
			local literalCount = pos - nextUnencodedPos
			local matchOffset = pos - matchPos

			-- Try to extend backwards
			while literalCount > 0 and matchPos > 0 and buffer.readu8(data, pos - 1) == buffer.readu8(data, matchPos - 1) do
				literalCount = literalCount - 1
				pos = pos - 1
				matchPos = matchPos - 1
			end

			-- Skip the 4 bytes we already matched
			pos = pos + MIN_MATCH
			matchPos = matchPos + MIN_MATCH

			-- Determine match length
			-- NOTE: matchLength does not include minMatch; it is added during decoding
			local matchLength = pos
			while pos < dataLen - MIN_TRAILING_LITERALS and buffer.readu8(data, pos) == buffer.readu8(data, matchPos) do
				pos = pos + 1
				matchPos = matchPos + 1
			end
			matchLength = pos - matchLength

			-- Write token
			local literalCountHalf = (literalCount < LIT_COUNT_MASK) and literalCount or LIT_COUNT_MASK
			local matchLenHalf = (matchLength < MATCH_LEN_MASK) and matchLength or MATCH_LEN_MASK
			local token = lshift(literalCountHalf, MATCH_LEN_BITS) + matchLenHalf
			buffer.writeu8(out, outNext, token)
			outNext = outNext + 1

			-- Write literal count
			local remaining = literalCount - LIT_COUNT_MASK
			while remaining >= 0xFF do
				buffer.writeu8(out, outNext, 0xFF)
				outNext = outNext + 1
				remaining = remaining - 0xFF
			end
			if remaining >= 0 then
				buffer.writeu8(out, outNext, remaining)
				outNext = outNext + 1
			end

			-- Write literals
			buffer.copy(out, outNext, data, nextUnencodedPos, literalCount)
			outNext = outNext + literalCount

			-- Write offset (little-endian)
			buffer.writeu16(out, outNext, matchOffset)
			outNext = outNext + 2

			-- Write match length
			remaining = matchLength - MATCH_LEN_MASK
			while remaining >= 0xFF do
				buffer.writeu8(out, outNext, 0xFF)
				outNext = outNext + 1
				remaining = remaining - 0xFF
			end
			if remaining >= 0 then
				buffer.writeu8(out, outNext, remaining)
				outNext = outNext + 1
			end

			-- Move the anchor
			nextUnencodedPos = pos
		end
	end

	-- Write remaining token (only literals, match length is 0)
	local literalCount = dataLen - nextUnencodedPos
	local token = lshift((literalCount < LIT_COUNT_MASK) and literalCount or LIT_COUNT_MASK, MATCH_LEN_BITS)
	buffer.writeu8(out, outNext, token)
	outNext = outNext + 1

	-- Write remaining literal count
	local remaining = literalCount - LIT_COUNT_MASK
	while remaining >= 0xFF do
		buffer.writeu8(out, outNext, 0xFF)
		outNext = outNext + 1
		remaining = remaining - 0xFF
	end
	if remaining >= 0 then
		buffer.writeu8(out, outNext, remaining)
		outNext = outNext + 1
	end

	-- Write remaining literals
	buffer.copy(out, outNext, data, nextUnencodedPos)
	outNext = outNext + (dataLen - nextUnencodedPos)

	return out, outNext
end

-- MARK: compress_string
--[[=
	Compresses a string using the LZ4 block format.

	@param string data The string to compress.
	@param number? acceleration A positive integer, defaults to 1. Higher values
	  may increase the compression speed, especially on incompressible data, at
	  the cost of compression efficiency.
	@return string The compressed data as a string.
]]
local function compress_string(str, acceleration)
	check_type(str, "string", 1, "compress_string")
	acceleration = check_acceleration(acceleration, 2, "compress_string")

	local buf, len = compress_buffer(buffer.fromstring(str), 0, #str, acceleration)
	return buffer.readstring(buf, 0, len)
end


local function expand_buffer(out, maxLen)
	local curLen = buffer.len(out)
	if curLen >= maxLen then
		return nil, "maximum decompressed length was exceeded"
	end
	local newLen = math.min(maxLen, curLen * 2)
	local newOut = buffer.create(newLen)
	buffer.copy(newOut, 0, out)
	return newOut, newLen
end

-- MARK: decompress_buffer
--[[=
	Decompresses a portion of a buffer that was compressed using the LZ4 block
	format. If any issue is encountered during decompression, this function will
	throw an error; call via `pcall()` when processing untrusted input.

	@param buffer data The buffer which contains the compressed data.
	@param number dataStart The offset at which the compressed data starts.
	@param number dataLen The length, in bytes, of the compressed data.
	@param number? decompressedLen If positive, gives the exact size of the
	  output buffer; an error will be thrown if the decompressed data doesn't
	  fit into the buffer. If negative, the output buffer will be dynamically
	  resized up to the argument's absolute value; an error will be thrown if
	  the decompressed data doesn't fit into the specified size. If omitted, the
	  output buffer will be dynamically resized with no upper bound.

	@return buffer A buffer containing the decompressed data at offset 0.
	@return number The length of the decompressed data.
]]
local function decompress_buffer(data, dataStart, dataLen, decompressedLen)
	check_data(data, dataStart, dataLen)
	decompressedLen = decompressedLen or -DEFAULT_MAX_BUFFER_SIZE
	check_type(decompressedLen, "number", 4, "decompress_buffer")

	local maxLen = math.abs(decompressedLen)
	local outLen = (decompressedLen >= 0) and decompressedLen or math.min(DEFAULT_BUFFER_SIZE, maxLen)
	local out, outNext = buffer.create(outLen), 0

	local pos = dataStart -- 0-indexed
	while pos < dataStart + dataLen do
		local token = buffer.readu8(data, pos)
		pos = pos + 1

		-- Literals --
		local literalCount = rshift(token, MATCH_LEN_BITS)

		-- Read literal count
		if literalCount == LIT_COUNT_MASK then
			repeat
				local lenPart = buffer.readu8(data, pos)
				pos = pos + 1
				literalCount = literalCount + lenPart
			until lenPart < 0xFF
		end

		while outNext + literalCount > outLen do
			out, outLen = assert(expand_buffer(out, maxLen))
		end

		-- Copy literals (if any)
		buffer.copy(out, outNext, data, pos, literalCount)
		outNext = outNext + literalCount
		pos = pos + literalCount

		if pos >= dataStart + dataLen then
			break -- This was the last sequence (which has no match part)
		end

		-- Match --
		local matchLength = band(token, MATCH_LEN_MASK)
		local matchOffset = buffer.readu16(data, pos)
		pos = pos + 2
		assert(matchOffset > 0 and matchOffset <= outNext, "Invalid LZ4 match offset")

		-- Read match length
		if matchLength == MATCH_LEN_MASK then
			repeat
				local lenPart = buffer.readu8(data, pos)
				pos = pos + 1
				matchLength = matchLength + lenPart
			until lenPart < 0xFF
		end

		matchLength = matchLength + MIN_MATCH

		while outNext + matchLength > outLen do
			out, outLen = assert(expand_buffer(out, maxLen))
		end

		-- Copy match
		while matchLength > matchOffset do
			buffer.copy(out, outNext, out, outNext - matchOffset, matchOffset)
			outNext = outNext + matchOffset
			matchLength = matchLength - matchOffset
		end

		buffer.copy(out, outNext, out, outNext - matchOffset, matchLength)
		outNext = outNext + matchLength
	end

	return out, outNext
end

-- MARK: decompress_string
--[[=
	Decompresses a string that was compressed using the LZ4 block format,
	optionally specifying a maximum decompressed length. If any issue is
	encountered during decompression, this function will throw an error; call
	via `pcall()` when processing untrusted input.

	@param string data The string to decompress.
	@param number? decompressedLen If positive, gives the exact size of the
	  output buffer; an error will be thrown if the decompressed data doesn't
	  fit into the buffer. If negative, the output buffer will be dynamically
	  resized up to the argument's absolute value; an error will be thrown if
	  the decompressed data doesn't fit into the specified size. If omitted, the
	  output buffer will be dynamically resized with no upper bound.
	@return string The decompressed string.
]]
local function decompress_string(str, decompressedLen)
	check_type(str, "string", 1, "decompress_string")
	decompressedLen = decompressedLen or -DEFAULT_MAX_BUFFER_SIZE
	check_type(decompressedLen, "number", 4, "decompress_buffer")

	local buf, len = decompress_buffer(buffer.fromstring(str), 0, #str, decompressedLen)
	return buffer.readstring(buf, 0, len)
end


return {
	compress_buffer = compress_buffer,
	compress_string = compress_string,
	compress       = compress_string,

	decompress_buffer = decompress_buffer,
	decompress_string = decompress_string,
	decompress       = decompress_string,
}
