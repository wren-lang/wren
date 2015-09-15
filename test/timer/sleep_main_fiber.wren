import "timer" for Timer

System.print("1") // expect: 1
Timer.sleep(3)
System.print("2") // expect: 2
Timer.sleep(3)
System.print("3") // expect: 3
Timer.sleep(3)
System.print("4") // expect: 4
