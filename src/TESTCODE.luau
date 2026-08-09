-- This is a script

type CMyClass = {
    a: number,
    method: (self: CMyClass) -> number,
}

@native
local function createMyClass(initialA: number): CMyClass
    return {
        a = initialA,
        method = function(self: CMyClass): number
            self.a += 1
            return self.a
        end,
    }
end

local obj = createMyClass(0)
print(obj:method())

for i = 0, 4 do
    print(`hello {i}`)
end

print("Luau for WingHexExplorer2 by wingsummer.")
