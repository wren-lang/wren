import "timer" for Timer

IO.print("1") // expect: 1
Timer.sleep(3)
IO.print("2") // expect: 2
Timer.sleep(3)
IO.print("3") // expect: 3
Timer.sleep(3)
IO.print("4") // expect: 4
