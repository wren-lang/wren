^title Random Class

A simple, fast pseudo-random number generator. Internally, it uses the [well
equidistributed long-period linear PRNG][well] (WELL512a).

[well]: https://en.wikipedia.org/wiki/Well_equidistributed_long-period_linear

Each instance of the class generates a sequence of randomly distributed numbers
based on the internal state of the object. The state is initialized from a
*seed*. Two instances with the same seed generate the exact same sequence of
numbers.


It must be imported from the [random][] module:

<pre class="snippet">
    import "random" for Random
</pre>

[random]: ../

## Constructors

### Random.**new**()

Creates a new generator whose state is seeded based on the current time.

<pre class="snippet">
var random = Random.new()
</pre>

### Random.**new**(seed)

Creates a new generator initialized with [seed]. The seed can either be a
number, or a non-empty sequence of numbers. If the sequnce has more than 16
elements, only the first 16 are used. If it has fewer, the elements are cycled
to generate 16 seed values.

<pre class="snippet">
Random.new(12345)
Random.new("appleseed".codePoints)
</pre>

## Methods

### **float**()

Returns a floating point value between 0.0 and 1.0, including 0.0, but excluding
1.0.

<pre class="snippet">
var random = Random.new(12345)
System.print(random.float()) //> 0.53178795980617
System.print(random.float()) //> 0.20180515043262
System.print(random.float()) //> 0.43371948658705
</pre>

### **float**(end)

Returns a floating point value between 0.0 and `end`, including 0.0 but
excluding `end`.

<pre class="snippet">
var random = Random.new(12345)
System.print(random.float(0))     //> 0
System.print(random.float(100))   //> 20.180515043262
System.print(random.float(-100))  //> -43.371948658705
</pre>

### **float**(start, end)

Returns a floating point value between `start` and `end`, including `start` but
excluding `end`.

<pre class="snippet">
var random = Random.new(12345)
System.print(random.float(3, 4))    //> 3.5317879598062
System.print(random.float(-10, 10)) //> -5.9638969913476
System.print(random.float(-4, 2))   //> -1.3976830804777
</pre>

### **int**(end)

Returns an integer between 0 and `end`, including 0 but excluding `end`.

<pre class="snippet">
var random = Random.new(12345)
System.print(random.int(1))    //> 0
System.print(random.int(10))   //> 2
System.print(random.int(-50))  //> -22
</pre>

### **int**(start, end)

Returns an integer between `start` and `end`, including `start` but excluding
`end`.

<pre class="snippet">
var random = Random.new(12345)
System.print(random.int(3, 4))    //> 3
System.print(random.int(-10, 10)) //> -6
System.print(random.int(-4, 2))   //> -2
</pre>

### **sample**(list)

Selects a random element from `list`.

### **sample**(list, count)

Samples `count` randomly chosen unique elements from `list`.

This uses "random without replacement" sampling&mdash;no index in the list will
be selected more than once.

Returns a new list of the selected elements.

It is an error if `count` is greater than the number of elements in the list.

### **shuffle**(list)

Randomly shuffles the elements in `list`. The items are randomly re-ordered in
place.

<pre class="snippet">
var random = Random.new(12345)
var list = (1..5).toList
random.shuffle(list)
System.print(list) //> [3, 2, 4, 1, 5]
</pre>

Uses the Fisher-Yates algorithm to ensure that all permutations are chosen
with equal probability.

Keep in mind that a list with even a modestly large number of elements has an
astronomically large number of permutations. For example, there are about 10^74
ways a deck of 56 cards can be shuffled. The random number generator's internal
state is not that large, which means there are many permutations it will never
generate.
