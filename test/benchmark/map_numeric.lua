local start = os.clock()

local map = {}

for i = 1, 2000000 do
  map[i] = i
end

local sum = 0
for i = 1, 2000000 do
  sum = sum + map[i]
end
io.write(string.format("%d\n", sum))

for i = 1, 2000000 do
  map[i] = nil
end

io.write(string.format("elapsed: %.8f\n", os.clock() - start))
