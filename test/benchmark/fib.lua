function fib(n)
  if n < 2 then return n end
  return fib(n - 2) + fib(n - 1)
end

local start = os.clock()
for i = 1, 5 do
  io.write(fib(28) .. "\n")
end
io.write(string.format("elapsed: %.8f\n", os.clock() - start))
