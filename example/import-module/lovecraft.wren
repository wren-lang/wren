import "cthulu" for Cthulu

class Lovecraft {
  construct new() {}
  say() { Cthulu.new().message }
}

IO.print(Lovecraft.new().say())
