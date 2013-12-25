list = Array.new(2000000) {|i| i}

start = Time.now
sum = 0
list.each {|i| sum += i}
puts sum
puts "elapsed: " + (Time.now - start).to_s
