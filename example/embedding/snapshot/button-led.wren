import "ZZZ" for Toto

class HW {
  foreign static led(color)
}
class Main {
  static BTN() {
    System.print("btn() {")
    HW.led(5)
    System.print("btn() }")
  }
  static Init() {
    System.print("init() 1")
    HW.led(7)
    System.print("init() 2")
    System.print(Toto)
    System.print("init() 3")
  }
}
