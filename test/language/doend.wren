
if (true) do
  System.print("hello") // expect: hello
end

Fn.new do | hello | System.print(hello) end.call("world") // expect: world

class Test do
  static hello { "Wren" }
  static print(){ System.print(hello) }
end

Test.print() // expect: Wren

do
  System.print("closure") // expect: closure
end
