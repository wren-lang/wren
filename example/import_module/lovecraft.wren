import "./cthulu" for Cthulu

class Lovecraft {
  construct new() {}
  say() { Cthulu.new().message }
}

System.print(Lovecraft.new().say())
