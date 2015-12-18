import "cthulu" for Cthulu

class Lovecraft {
  construct new() {}
  def say() { Cthulu.new().message }
}

System.print(Lovecraft.new().say())
