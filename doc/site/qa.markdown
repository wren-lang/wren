^title Q & A

## Why did you create Wren?

Most creative endeavors aren't immediately met with existential crises, but for some reason many programmers don't seem to like the idea of new languages. My reason for creating Wren is pretty simple: I really enjoy building things.

I find it deeply soothing to write a bit of code, see a new test pass and check it in. I seem to be wired to seek concrete accomplishment. I'm not very good at relaxing and doing nothing. I need to feel like I earned a gold star, or a few XP, or levelled up, or something. Too many years as a teacher's pet has left it's mark on me.

## Why did you create *Wren?*

I guess that's not the question you were asking. You want to know why I created *Wren* as opposed to some other language?

* I already know C and it's fun to code in. When I code in my free time, I have limited patience for learning new things. I want to *make*, not *learn-how-to-make*.

* Dynamically-typed languages are a lot simpler to design and build. I like static types, but good type systems are *hard*. Implementing a type-checker isn't too hard, but doing things like stack maps for GC didn't seem like fun to me.

* Creating a scripting language for embedding means I don't have to make a huge standard library before the langauge is useful. Creating a new language is a ton of work. You have to design it, document it, implement it, optimize it. On top of that, you need a standard library, packages, tooling, a while ecosystem. Doing a scripting language cuts that problem down to a much more manageable size while still yielding a language that real people can use for real things.

* I really like classes and objects. I could go into this at length, but I'll spare you.

## Why should *I* care that you created Wren?

Ah, here's the real question. You want to know why Wren might be relevant to *you*. Good question! Here's the niche I'm trying to fill:

There are a handful of scripting languages that are in use for embedding in applications. Lua is the big one. There's also Guile, increasingly JavaScript, and some applications embed Python. I'm an ex-game developer, so when I think "scripting", I tend to think "game scripting".

Lua is a good answer there: it's small, simple, and fast. But, and I don't mean this as a criticism, it's also weird if you're used to languages like C++ and Java. The syntax is clean but different. The semantics, especially the object model are unusual. Anyone can get used to 1-based indexing, but things like metatables really show that objects were bolted onto Lua after the fact.

I feel like there's an opportunity for a language that's as small, simple, and fast as Lua, but also one that feels familiar and natural to someone with a conventional OOP background. Wren is my attempt at that.

Here's an example of object-oriented programming in Lua:

    :::lua
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
    acc = Account.create(1000)
    acc:withdraw(100)

Here's the same example in Wren:

    :::wren
    class Account {
      new(balance) { _balance = balance }
      withdraw(amount) { _balance = _balance - amount }
    }

    // create and use an Account
    var acc = new Account(100)
    acc.withdraw(100)

I also feel there's room for a language with a minimal, beautiful *implementation*. Lua's code is clean, but not well documented or easy to read. With Wren, I'm trying to make the code as approachable as possible so that others feel confident using it and extending it.

## Why compile to bytecode?

Interpreters come in three flavors, in order of increasing performance: "tree-walk" or line-based interpreters run the code directly from the source or a parse tree. Bytecode interpreters compile the parsed source code to a set of instructions for a virtual machine which they then implement. JIT compilers do something similar but actually compile to the host machine's native instruction set.

JIT compilers are nice, but:

* They are *much* more complex to implement and debug.
* Since they use the machine's native stack, cooperative multitasking (fibers) have to do lower-level shenanigans.
* Many devices like iPhones and game consoles do not allow runtime generated code to be executed.

Bytecode, meanwhile is quite simple while also fast enough for real-world usage. It also makes supporting fibers straightforward since the virtual machine defines its own stack(s) and can switch between them easily.

## What about your other languages?

This is a strange question if you don't happen to know who I am. In the past, I've hacked on and blogged about a couple of other hobby languages. The two most frequent are [Finch](http://finch.stuffwithstuff.com/) and [Magpie](http://magpie-lang.org/). Why a third?

Well, actually, Wren isn't my *third*, it's probably closer to tenth at this point. I try out lots of different ideas. Wren grew directly out of Finch. I started Finch to learn more about implementing an interpreter and also about the prototype paradigm. I learned a ton about both.

What I learned in particular is that C++ is nice but not that necessary if your codebase is relatively small. And I learned that I really prefer classes over prototypes. I started retrofitting classes into Finch but eventually realized it was too big of a change to do through evolution, and thus Wren was born.

Wren is effectively a replacement for Finch to me. I gave it a new name mainly so that I can keep Finch around in case other people want to take it and do something with it. I don't have any intention to work on Finch anymore. Wren scratches that same itch 10x.

Magpie is a trickier one. I really really like the ideas behind Magpie. It's the general-purpose language I wish I had most of the time. I love patterns and multiple-dispatch. I like how it integrates the event-based IO of libuv with the simplicity of fibers.

But it's also a much more challenging project. As a general-purpose language, there's a ton of library work to do before Magpie is useful for anything. It has some unresolved GC issues. And I'm frankly not skilled enough right now to implement multiple dispatch efficiently. Meanwhile, since I started working on Magpie, [Julia](http://julialang.org/) has appeared and [Dylan](http://opendylan.org/) has reappeared. I was really astonished at how much Magpie had in common with Julia when it came out. I created Magpie partially to carry the torch of multiple dispatch, but others are starting to spread that light now.

I think I have a greater chance of success with Wren, but that doesn't mean I don't still love Magpie and want to work on it. I tend to rotate through projects. I rarely abandon them forever, I just take long breaks. Ask me about my roguelike sometime.
