local start = os.clock()
local list = {}
for i = 0, 999999 do
  list[i] = i
end

local sum = 0
for k, i in pairs(list) do
  sum = sum + i
end
io.write(sum .. "\n")
io.write(string.format("elapsed: %.8f\n", os.clock() - start))
