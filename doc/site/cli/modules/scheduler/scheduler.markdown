^title Scheduler Class

The Scheduler class maintains a list of fibers, to be started one after the other, when a signal to do so is received. The signal (a private method call) is typically transmitted by _long running_ methods in the File or Timer classes which suspend the current fiber so that Wren can carry out other tasks in the meantime.

## Static Method

### Scheduler.**add**(callable)

Adds a new fiber to the scheduler's fibers list. This fiber calls `callable` and then transfers to the next fiber in the list, if there is one.

`callable` is a function or other object which has a call() method.

<pre class="snippet">
var a = 3

Scheduler.add {
  a = a * a
}

Scheduler.add {
  a = a + 1
}

System.print(a)        // still 3
Timer.sleep(3000)      // wait 3 seconds
System.print(a)        // now 3 * 3 + 1 = 10
</pre>
