import "cthulu" for Cthulu

class Lovecraft {
  def construct new() {}
  def say() { Cthulu.new().message }
}

System.print(Lovecraft.new().say())
