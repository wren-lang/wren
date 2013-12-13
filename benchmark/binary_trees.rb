# The Computer Language Shootout Benchmarks
# http://shootout.alioth.debian.org
#
# contributed by Jesse Millikan
# Modified by Wesley Moxam


def item_check(left, item, right)
  return item if left.nil?
  item + item_check(*left) - item_check(*right)
end

def bottom_up_tree(item, depth)
  return [nil, item, nil] unless depth > 0
  item_item = 2 * item
  depth -= 1
  [bottom_up_tree(item_item - 1, depth), item, bottom_up_tree(item_item, depth)]
end

max_depth = 12
min_depth = 4

max_depth = min_depth + 2 if min_depth + 2 > max_depth

stretch_depth = max_depth + 1
stretch_tree = bottom_up_tree(0, stretch_depth)

start = Time.now
puts "stretch tree of depth #{stretch_depth} check: #{item_check(*stretch_tree)}"
stretch_tree = nil

long_lived_tree = bottom_up_tree(0, max_depth)

min_depth.step(max_depth + 1, 2) do |depth|
  iterations = 2**(max_depth - depth + min_depth)

  check = 0

  for i in 1..iterations
    temp_tree = bottom_up_tree(i, depth)
    check += item_check(*temp_tree)

    temp_tree = bottom_up_tree(-i, depth)
    check += item_check(*temp_tree)
  end

  puts "#{iterations * 2} trees of depth #{depth} check: #{check}"
end

puts "long lived tree of depth #{max_depth} check: #{item_check(*long_lived_tree)}"
puts "elapsed: " + (Time.now - start).to_s
