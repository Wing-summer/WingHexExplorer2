local Json = require("@s/mcp/json")
local Schema = {}

export type ValidationError = {
	path: string,
	message: string,
}

local function push_error(errors: { ValidationError }, path: string, message: string)
	table.insert(errors, { path = path, message = message })
end

local function is_array(t: any): boolean
	if type(t) ~= "table" then
		return false
	end
	local n = 0
	for k, _ in pairs(t) do
		if type(k) ~= "number" then
			return false
		end
		n += 1
	end
	return n == #t
end

local function join_path(base: string, key: string): string
	if base == "" then
		return "/" .. key
	end
	return base .. "/" .. key
end

local function validate_schema(schema: any, value: any, path: string, errors: { ValidationError })
	if type(schema) ~= "table" then
		return
	end

	local schemaType = schema.type
    if schema.enum then
        local found = false
        for _, candidate in schema.enum do if candidate == value then found = true end end
        if not found then push_error(errors, path, "value is outside enum"); return end
    end
    if schemaType == "integer" then
        if type(value) ~= "number" or value % 1 ~= 0 then push_error(errors, path, "expected integer") end
        return
    end
    if schemaType == "null" then
        if value ~= Json.null then push_error(errors, path, "expected null") end
        return
    end
	if schemaType ~= nil and type(schemaType) == "string" then
		if schemaType == "object" then
			if type(value) ~= "table" or (getmetatable(value) and getmetatable(value).__jsonArray) or (next(value) ~= nil and is_array(value)) then
				push_error(errors, path, "expected object")
				return
			end

			local required = schema.required
			if type(required) == "table" then
				for _, key in ipairs(required) do
					if type(key) == "string" then
						if value[key] == nil then
							push_error(errors, join_path(path, key), "is required")
						end
					end
				end
			end

            if schema.additionalProperties == false then
                for key in value do
                    if not schema.properties or schema.properties[key] == nil then push_error(errors, join_path(path, tostring(key)), "unexpected property") end
                end
            end
			local properties = schema.properties
			if type(properties) == "table" then
				for key, subschema in pairs(properties) do
					if type(key) == "string" then
						if value[key] ~= nil then
							validate_schema(subschema, value[key], join_path(path, key), errors)
						end
					end
				end
			end
			return
		end

		if schemaType == "array" then
			if not is_array(value) or (getmetatable(value) and getmetatable(value).__qtluau_object) then
				push_error(errors, path, "expected array")
				return
			end
			local items = schema.items
			if items ~= nil then
				for i, v in ipairs(value) do
					validate_schema(items, v, join_path(path, tostring(i)), errors)
				end
			end
			return
		end

		if schemaType == "string" then
			if type(value) ~= "string" then
				push_error(errors, path, "expected string")
			end
			return
		end

		if schemaType == "number" then
			if type(value) ~= "number" then
				push_error(errors, path, "expected number")
			end
			return
		end

		if schemaType == "boolean" then
			if type(value) ~= "boolean" then
				push_error(errors, path, "expected boolean")
			end
			return
		end
	end
end

function Schema.validate(schema: any, value: any): (boolean, { ValidationError })
	local errors: { ValidationError } = {}
	validate_schema(schema, value, "", errors)
	return #errors == 0, errors
end

return Schema
