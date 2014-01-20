start = Time.now
list = []
1000000.times {|i| list << i}

sum = 0
list.each {|i| sum += i}
puts sum
puts "elapsed: " + (Time.now - start).to_s
