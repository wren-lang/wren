// calculate fibonacy numbers

var fibs = [0, 1, 2, 3, 4, 5]

var Fib = Fn.new {|num|
    if (num <= 1) {
        return 1
    } else {
        return Fib.call(num - 2) + Fib.call(num - 1)
    }
}


for (number in fibs) {
    System.print(Fib.call(number))
}
