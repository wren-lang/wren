map = Hash.new

start = Time.now

for i in (1..100000)
  map[i] = i
end

sum = 0
for i in (1..100000)
  sum = sum + map[i]
end
puts sum

for i in (1..100000)
  map.delete(i)
end

puts "elapsed: " + (Time.now - start).to_s
