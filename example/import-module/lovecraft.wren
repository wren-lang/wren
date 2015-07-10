import "cthulu" for Cthulu

class Lovecraft {
  say() { Cthulu.new().message }
}

IO.print(Lovecraft.new().say())
