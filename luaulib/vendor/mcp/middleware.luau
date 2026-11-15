local Middleware = {}

export type Next = () -> any

export type Context = {
	method: string,
	id: any?,
	params: any?,
}

export type Handler = (ctx: Context, next: Next) -> any

function Middleware.compose(handlers: { Handler })
	return function(ctx: Context, terminal: Next)
		local index = 0
		local function dispatch(i: number): any
			if i <= index then
				error("next() called multiple times")
			end
			index = i
			local h = handlers[i]
			if h == nil then
				return terminal()
			end
			return h(ctx, function()
				return dispatch(i + 1)
			end)
		end
		return dispatch(1)
	end
end

return Middleware
