^title Q & A

## Why did you create Wren?

Other creative endeavors aren't immediately met with existential crises, but
for some reason programmers don't seem to like new languages. Here's the niche
I'm trying to fill:

There are a few scripting languages used for embedding in applications. Lua is
the main one. TCL used to be. There's also Guile, increasingly JavaScript, and
some applications embed Python. I'm an ex-game developer, so when I think
"scripting", I tend to think "game scripting".

Lua is nice: it's small, simple, and fast. But&mdash;and I don't mean this as a
criticism&mdash;it's also weird if you're used to languages like C++ and Java.
The syntax is different. The semantics, especially the object model are
unusual. Anyone can get used to 1-based indexing, but things like metatables
really show that objects were bolted onto Lua after the fact.

I think there's room for a language as simple as Lua, but that feels natural to
someone with an OOP background. Wren is my attempt at that.

## Why classes?

Thanks to JavaScript's popularity, lots of people are discovering prototypes
right now, and the paradigm is experiencing a popularity boom. I think
prototypes are interesting, but after [several years playing with them][finch],
I concluded (like many people on the original Self project that invented
prototypes) that classes are more usable.

[finch]: http://finch.stuffwithstuff.com/

Here's an example of that kind of object-oriented programming in Lua:

<pre class="snippet">
-- account.lua

Account = {}
Account.__index = Account

function Account.create(balance)
   local acnt = {}             -- our new object
   setmetatable(acnt,Account)  -- make Account handle lookup
   acnt.balance = balance      -- initialize our object
   return acnt
end

function Account:withdraw(amount)
   self.balance = self.balance - amount
end

-- create and use an Account
account = Account.create(1000)
account:withdraw(100)
</pre>

Here's the same example in Wren:

<pre class="snippet">
//account.wren

class Account {
  construct new(balance) { _balance = balance }
  withdraw(amount) { _balance = _balance - amount }
}

// create and use an Account
var account = Account.new(1000)
account.withdraw(100)
</pre>

Classes have a reputation for complexity because most of the widely used
languages with them are quite complex: C++, Java, C#, Ruby, and Python. I hope
to show with Wren that it is those languages that are complex, and not classes
themselves.

Smalltalk, the language that inspired most of those languages, is famously
simple. Its syntax [fits on an index card][card]. My aim is to keep Wren that
minimal while still having the expressive power of [classes](classes.html).

[card]: http://www.jarober.com/blog/blogView?showComments=true&title=Readability+is+Key&entry=3506312690

## Why compile to bytecode?

The [performance page](performance.html) has more details, but the short answer
is that bytecode is a nice trade-off between performance and simplicity. Also:

 *  Many devices like iPhones and game consoles don't allow executing code
    generated at runtime, which rules out just-in-time compilation.

 *  I think [fibers][] are a really powerful tool, and implementing them is
    straightforward in a bytecode VM that doesn't use the native stack.

[fibers]: concurrency.html

## Why is the VM stack-based instead of register-based?

Bytecode VMs come in two flavors. Stack-based VMs have short (usually one byte)
instructions whose operands are implicitly understood to be at the top of the
stack. That means you often have a couple of instructions to push some stuff on
the stack and then an instruction to do something.

Register-based VMs have big instructions (usually 32 bits) that contain both an
opcode and a couple of numbers indicating where in the stack the operands can
be found. This is cool because it means, that, for example, this Lua statement:

<pre class="snippet">
a = b + c
</pre>

Can be a single bytecode instruction. In a stack-based language, it would be
four&mdash;push `b`, push `c`, add, store `a`. (Though note that in both cases
you've got 32 total bits of code.)

Lua used to be stack-based and switched to register-based and got a speed
boost. Why not use registers for Wren?

I've implemented a [register-based VM
before](http://finch.stuffwithstuff.com/). I think it's a cool model, but I
don't think it would bring much benefit for Wren. It's more effort to compile,
and I'm trying to keep Wren's implementation as simple as possible.

In return for that complexity, you can generate fewer instructions. However, I
don't think Wren would be able to take advantage of that. Wren doesn't
currently have any dedicated instructions for arithmetic. Operators are just
regular method calls and can call user-defined procedures.

The calling convention for methods requires all of their parameters to be at
the top of the caller's stack so that they can become bottom of the callee's
stack frame window. To call `+` in Wren, we still have to push the arguments on
top of the stack. Likewise, the method calling convention places the return
value where the first argument was, so we'd have to move it back down to the
destination slot after the call.

It may be worth having dedicated instructions for arithmetic that special case
the built-in types before falling back to user-defined operator methods (which
I assume is what Lua does since they added operator overloading late in the
language's development). If that happens, it may be possible to switch to
register-based.

But I'm not convinced it would be an actual performance win. A lot of details
of the language affect whether a register-based VM is better. For example,
assignments are statements in Lua but expressions in Wren, which would make
them harder to compile to efficient register-based code.

## What about your other languages?

This is a strange question if you don't happen to know [who I am][me]. In the
past, I've hacked on and blogged about a couple of other hobby languages like
[Finch][] and [Magpie][].

[me]: http://journal.stuffwithstuff.com
[magpie]: http://magpie-lang.org/

I started Finch to learn more about implementing an interpreter and also about
the prototype paradigm. I learned a ton about both. Critically, I learned that
I really prefer classes over prototypes. I started retrofitting classes into
Finch but realized it was too big of a change, and thus Wren was born.

Wren is a replacement for Finch to me. I gave it a new name mainly so that I
can keep Finch around in case other people want to take it and do something
with it. I don't have any intention to work on it anymore.

Magpie is a trickier one. I really like the ideas behind Magpie. It's the
general-purpose language I wish I had much of the time. I love pattern matching
and multiple dispatch. I like how it integrates the event-based IO of [libuv][]
with the simplicity of fibers.

[libuv]: https://github.com/joyent/libuv

But it's also a much more challenging project. As a general-purpose language,
there's a ton of library work to do before Magpie is useful for anything. It
has some unresolved GC issues. And I'm frankly not skilled enough right now to
implement multiple dispatch efficiently.

Meanwhile, since I started working on Magpie, [Julia](http://julialang.org/)
appeared and [Dylan](http://opendylan.org/) *re*appeared. I created Magpie
partially to carry the torch of multiple dispatch, but others are starting to
spread that light now.
