start = Time.now

map = Hash.new

for i in (1..1000000)
  map[i] = i
end

sum = 0
for i in (1..1000000)
  sum = sum + map[i]
end
puts sum

for i in (1..1000000)
  map.delete(i)
end

puts "elapsed: " + (Time.now - start).to_s
