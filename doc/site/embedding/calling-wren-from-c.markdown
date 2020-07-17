^title Calling Wren from C

From C, we can tell Wren to do stuff by calling `wrenInterpret()`, but that's
not always the ideal way to drive the VM. First of all, it's slow. It has to
parse and compile the string of source code you give it. Wren has a pretty fast
compiler, but that's still a good bit of work.

It's also not an effective way to communicate. You can't pass arguments to
Wren&mdash;at least, not without doing something nasty like converting them to
literals in a string of source code&mdash;and you can't get a result value back.

`wrenInterpret()` is great for loading code into the VM, but it's not the best
way to execute code that's already been loaded. What we want to do is invoke
some already compiled chunk of code. Since Wren is an object-oriented language,
"chunk of code" means a [method][], not a [function][].

[method]: ../method-calls.html
[function]: ../functions.html

The C API for doing this is `wrenCall()`. In order to invoke a Wren method from
C, we need a few things:

* **The method to call.** Wren is dynamically typed, so this means we'll look it
  up by name. Further, since Wren supports overloading by arity, we actually
  need its entire [signature][].

[signature]: ../method-calls.html#signature

* **The receiver object to invoke the method on.** The receiver's class
  determines which method is actually called.

* **The arguments to pass to the method.**

We'll tackle these one at a time.

### Getting a Method Handle

When you run a chunk of Wren code like this:

<pre class="snippet">
object.someMethod(1, 2, 3)
</pre>

At runtime, the VM has to look up the class of `object` and find a method there
whose signature is `someMethod(_,_,_)`. This sounds like it's doing some string
manipulation&mdash;at the very least hashing the signature&mdash;every time a
method is called. That's how many dynamic languages work.

But, as you can imagine, that's pretty slow. So, instead, Wren does as much of
that work at compile time as it can. When it's compiling the above code to
bytecode, it takes that method signature a converts it to a *method symbol*, a
number that uniquely identifes that method. That's the only part of the process
that requires treating a signature as a string.

At runtime, the VM just looks for the method *symbol* in the receiver's class's
method table. In fact, the way it's implemented today, the symbol is simply the
array index into the table. That's [why method calls are so fast][perf] in Wren.

[perf]: ../performance.html

It would be a shame if calling a method from C didn't have that same speed
benefit. To achieve that, we split the process of calling a method into two
steps. First, we create a handle that represents a "compiled" method signature:

<pre class="snippet" data-lang="c">
WrenHandle* wrenMakeCallHandle(WrenVM* vm, const char* signature);
</pre>

That takes a method signature as a string and gives you back an opaque handle
that represents the compiled method symbol. Now you have a *reusable* handle
that can be used to very quickly call a certain method given a receiver and some
arguments.

This is just a regular WrenHandle, which means you can hold onto it as long as
you like. Typically, you'd call this once outside of your application's
performance critical loops and reuse it as long as you need. It is us up to you
to release it when you no longer need it by calling `wrenReleaseHandle()`.

## Setting Up a Receiver

OK, we have a method, but who are we calling it on? We need a receiver, and as
you can probably guess after reading the [last section][], we give that to Wren
by storing it in a slot. In particular, **the receiver for a method call goes in
slot zero.**

Any object you store in that slot can be used as a receiver. You could even call
`+` on a number by storing a number in there if you felt like it.

[last section]: slots-and-handles.html

Needing a receiver to call some Wren code from C might feel strange. C is
procedural, so it's natural to want to just invoke a bare *function* from Wren,
but Wren isn't procedural. Instead, if you want to define some executable
operation that isn't logically tied to a specific object, the natural way is to
define a static method on an appropriate class.

For example, say we're making a game engine. From C, we want to tell the game
engine to update all of the entities each frame. We'll keep track of the list of
entities within Wren, so from C, there's no obvious object to call `update(_)`
on. Instead, we'll just make it a static method:

<pre class="snippet">
class GameEngine {
  static update(elapsedTime) {
    // ...
  }
}
</pre>

Often, when you call a Wren method from C, you'll be calling a static method.
But, even then, you need a receiver. Now, though, the receiver is the *class
itself*. Classes are first class objects in Wren, and when you define a named
class, you're really declaring a variable with the class's name and storing a
reference to the class object in it.

Assuming you declared that class at the top level, the C API [gives you a way to
look it up][variable]. We can get a handle to the above class like so:

[variable]: slots-and-handles.html#looking-up-variables

<pre class="snippet" data-lang="c">
// Load the class into slot 0.
wrenEnsureSlots(vm, 1);
wrenGetVariable(vm, "main", "GameEngine", 0);
</pre>

We could do this every time we call `update()`, but, again, that's kind of slow
because we're looking up "GameEngine" by name each time. A faster solution is to
create a handle to the class once and use it each time:

<pre class="snippet" data-lang="c">
// Load the class into slot 0.
wrenEnsureSlots(vm, 1);
wrenGetVariable(vm, "main", "GameEngine", 0);
WrenHandle* gameEngineClass = wrenGetSlotHandle(vm, 0);
</pre>

Now, each time we want to call a method on GameEngine, we store that value back
in slot zero:

<pre class="snippet" data-lang="c">
wrenSetSlotHandle(vm, 0, gameEngineClass);
</pre>

Just like we hoisted `wrenMakeCallHandle()` out of our performance critical
loop, we can hoist the call to `wrenGetVariable()` out. Of course, if your code
isn't performance critical, you don't have to do this.

## Passing Arguments

We've got a receiver in slot zero now, next we need to pass in any other
arguments. In our GameEngine example, that's just the elapsed time. Method
arguments go in consecutive slots after the receiver. So the elapsed time goes
into slot one. You can use any of the slot functions to set this up. For the
example, it's just:

<pre class="snippet" data-lang="c">
wrenSetSlotDouble(vm, 1, elapsedTime);
</pre>

## Calling the Method

We have all of the data in place, so all that's left is to pull the trigger and
tell the VM to start running some code:

<pre class="snippet" data-lang="c">
WrenInterpretResult wrenCall(WrenVM* vm, WrenHandle* method);
</pre>

It takes the method handle we created using `wrenMakeCallHandle()`. Now Wren
starts running code. It looks up the method on the receiver, executes it and
keeps running until either the method returns or a fiber [suspends][].

[suspends]: ../modules/core/fiber.html#fiber.suspend()

`wrenCall()` returns the same WrenInterpretResult enum as `wrenInterpret()` to
tell you if the method completed successfully or a runtime error occurred.
(`wrenCall()` never returns `WREN_ERROR_COMPILE` since it doesn't compile
anything.)

## Getting the Return Value

When `wrenCall()` returns, it leaves the slot array in place. In slot zero, you
can find the method's return value, which you can access using any of the slot
reading functions. If you don't need the return value, you can ignore it.

This is how you drive Wren from C, but how do you put control in Wren's hands?
For that, you'll need the next section...

<a class="right" href="calling-c-from-wren.html">Calling C From Wren &rarr;</a>
<a href="slots-and-handles.html">&larr; Slots and Handles</a>
